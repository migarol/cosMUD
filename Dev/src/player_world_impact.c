/*****************************************************************************
 * Player World Impact System
 *
 * Player actions matter! Everything they do affects the world:
 * - Kill mobs → Safety score changes
 * - Complete quests → Economic/cultural boost
 * - Donate → Construction progress
 * - Trade → Economy shifts
 *
 * Changes are GRADUAL (Village → Town = 6 months game time)
 * Rate limited (max 3 rooms/day, 5 NPCs/day)
 *
 * Integration: world_history_tracker.h, beeler_god_mode.h
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "player_world_impact.h"
#include "world_history_tracker.h"
#include "beeler_god_mode.h"
#include "periodicos.h"

/* Global state */
PLAYER_ACTION_IMPACT *first_action = NULL;
int total_actions_recorded = 0;

GRADUAL_CHANGE *first_gradual_change = NULL;
int total_gradual_changes = 0;

AREA_PLAYER_HISTORY *area_histories[100];
int num_area_histories = 0;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_player_world_impact(void)
{
    log_string("Initializing Player World Impact System...");
    first_action = NULL;
    total_actions_recorded = 0;
    first_gradual_change = NULL;
    total_gradual_changes = 0;
    num_area_histories = 0;
    memset(area_histories, 0, sizeof(area_histories));
    log_string("Player World Impact System initialized.");
}

void load_player_impact_history(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "player_impact.dat");

    if ((fp = fopen(filename, "r")) == NULL)
    {
        log_string("No saved player impact history to load.");
        return;
    }

    log_string("Loading player impact history...");
    fclose(fp);
}

void save_player_impact_history(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "player_impact.dat");

    if ((fp = fopen(filename, "w")) == NULL)
    {
        log_string("ERROR: Cannot save player impact history!");
        return;
    }

    fprintf(fp, "#PLAYER_IMPACT\n");
    /* Save impact data */
    fprintf(fp, "#END\n");
    fclose(fp);
}

/*****************************************************************************
 * Temporal Scaling - Game time vs Real time
 *****************************************************************************/

int convert_real_to_game_time(int real_hours)
{
    return real_hours * GAME_TIME_MULTIPLIER;
}

int convert_game_to_real_time(int game_hours)
{
    return game_hours / GAME_TIME_MULTIPLIER;
}

char *format_time_remaining(int game_hours)
{
    static char buf[256];
    int days, weeks, months;

    months = game_hours / (30 * 24);
    game_hours %= (30 * 24);
    weeks = game_hours / (7 * 24);
    game_hours %= (7 * 24);
    days = game_hours / 24;

    if (months > 0)
        sprintf(buf, "%d month%s, %d week%s", months, months > 1 ? "s" : "", weeks, weeks != 1 ? "s" : "");
    else if (weeks > 0)
        sprintf(buf, "%d week%s, %d day%s", weeks, weeks > 1 ? "s" : "", days, days != 1 ? "s" : "");
    else
        sprintf(buf, "%d day%s", days, days != 1 ? "s" : "");

    return buf;
}

/*****************************************************************************
 * Action Tracking - Record player actions
 *****************************************************************************/

PLAYER_ACTION_IMPACT *create_action_impact(CHAR_DATA *player, int action_type)
{
    PLAYER_ACTION_IMPACT *action;

    if (!player)
        return NULL;

    CREATE(action, PLAYER_ACTION_IMPACT, 1);
    action->player = player;
    action->action_type = action_type;
    action->when = time(NULL);
    action->area_affected = player->in_room ? player->in_room->area : NULL;
    action->room = player->in_room;
    action->action_description = NULL;
    action->target_mob = NULL;
    action->target_obj = NULL;
    action->economic_impact = 0;
    action->safety_impact = 0;
    action->population_impact = 0;
    action->cultural_impact = 0;
    action->environmental_impact = 0;
    action->significant = FALSE;
    action->announced = FALSE;
    action->next = first_action;
    first_action = action;
    total_actions_recorded++;

    return action;
}

void record_player_action(CHAR_DATA *player, int action_type, void *target, AREA_DATA *area)
{
    PLAYER_ACTION_IMPACT *action;
    char buf[MAX_STRING_LENGTH];

    if (!player || IS_NPC(player))
        return;

    action = create_action_impact(player, action_type);
    if (!action)
        return;

    action->area_affected = area ? area : (player->in_room ? player->in_room->area : NULL);

    switch (action_type)
    {
        case PLAYER_ACTION_KILL_MOB:
            action->target_mob = (CHAR_DATA *)target;
            action->safety_impact = 5;
            sprintf(buf, "Killed %s", (char *)action->target_mob->short_descr);
            action->action_description = str_dup(buf);
            action->significant = TRUE;
            break;

        case PLAYER_ACTION_COMPLETE_QUEST:
            action->economic_impact = 10;
            action->cultural_impact = 5;
            action->action_description = str_dup("Completed quest");
            action->significant = TRUE;
            break;

        case PLAYER_ACTION_DONATE_GOLD:
            action->economic_impact = 15;
            action->action_description = str_dup("Donated gold");
            action->significant = TRUE;
            break;

        case PLAYER_ACTION_TRADE:
            action->economic_impact = 5;
            action->action_description = str_dup("Traded goods");
            break;

        default:
            action->action_description = str_dup("Performed action");
            break;
    }

    /* Apply impact to area vital signs */
    if (action->area_affected)
    {
        AREA_VITAL_SIGNS *vitals = beeler_get_area_health(action->area_affected);
        if (vitals)
            apply_action_to_vital_signs(action, vitals);
    }

    /* Record in world history */
    if (action->significant)
    {
        record_history_player_action((char *)player->name, action->action_description, 6);
    }

    {
        char log_buf[256];
    sprintf(log_buf, "PLAYER IMPACT: %s - %s in %s", (char *)player->name,
               action->action_description,
               action->area_affected ? action->area_affected->name : "unknown");
    log_string(log_buf);
    }
}

void apply_action_to_vital_signs(PLAYER_ACTION_IMPACT *action, AREA_VITAL_SIGNS *vitals)
{
    if (!action || !vitals)
        return;

    /* Apply impacts (rate limited) */
    if (can_change_vital_sign(vitals, action->safety_impact))
        vitals->safety_score += action->safety_impact;

    if (can_change_vital_sign(vitals, action->economic_impact))
        vitals->economic_score += action->economic_impact;

    if (can_change_vital_sign(vitals, action->cultural_impact))
        vitals->cultural_activity += action->cultural_impact;

    /* Clamp values */
    vitals->safety_score = URANGE(0, vitals->safety_score, 100);
    vitals->economic_score = URANGE(-100, vitals->economic_score, 100);
    vitals->cultural_activity = URANGE(0, vitals->cultural_activity, 100);
}

/*****************************************************************************
 * Specific Action Handlers
 *****************************************************************************/

void player_killed_mob(CHAR_DATA *player, CHAR_DATA *mob)
{
    if (!player || !mob || IS_NPC(player))
        return;

    record_player_action(player, PLAYER_ACTION_KILL_MOB, mob, mob->in_room->area);

    /* Record kill in history */
    record_kill_event((char *)player->name, (char *)mob->short_descr, (char *)"combat",
                     (char *)(mob->in_room ? mob->in_room->area->name : "unknown"));
}

void player_completed_quest(CHAR_DATA *player, void *quest /* QUEST_DATA not yet implemented */)
{
    if (!player || IS_NPC(player))
        return;

    record_player_action(player, PLAYER_ACTION_COMPLETE_QUEST, quest, player->in_room->area);
}

void player_donated_gold(CHAR_DATA *player, AREA_DATA *area, int amount)
{
    PLAYER_ACTION_IMPACT *action;
    char buf[MAX_STRING_LENGTH];

    if (!player || IS_NPC(player))
        return;

    action = create_action_impact(player, PLAYER_ACTION_DONATE_GOLD);
    action->area_affected = area;
    action->economic_impact = UMIN(amount / 100, 20); /* Scale impact */

    sprintf(buf, "Donated %d gold to %s", amount, area->name);
    action->action_description = str_dup(buf);
    action->significant = (amount >= 1000);

    {
        char log_buf[256];
    sprintf(log_buf, "PLAYER IMPACT: %s donated %d gold to %s", (char *)player->name, amount, area->name);
    log_string(log_buf);
    }
}

void player_donated_item(CHAR_DATA *player, AREA_DATA *area, OBJ_DATA *item)
{
    PLAYER_ACTION_IMPACT *action;
    char buf[MAX_STRING_LENGTH];

    if (!player || !item || IS_NPC(player))
        return;

    action = create_action_impact(player, PLAYER_ACTION_DONATE_ITEM);
    action->area_affected = area;
    action->target_obj = item;
    action->economic_impact = UMIN(item->cost / 1000, 15);

    sprintf(buf, "Donated %s to %s", item->short_descr, area->name);
    action->action_description = str_dup(buf);
    action->significant = (item->cost >= 5000);
}

void player_helped_npc(CHAR_DATA *player, CHAR_DATA *npc, char *how)
{
    PLAYER_ACTION_IMPACT *action;
    char buf[MAX_STRING_LENGTH];

    if (!player || !npc || IS_NPC(player))
        return;

    action = create_action_impact(player, PLAYER_ACTION_HELP_NPC);
    action->target_mob = npc;
    action->cultural_impact = 5;

    sprintf(buf, "Helped %s: %s", npc->short_descr, how);
    action->action_description = str_dup(buf);
}

void player_harmed_npc(CHAR_DATA *player, CHAR_DATA *npc)
{
    PLAYER_ACTION_IMPACT *action;
    char buf[MAX_STRING_LENGTH];

    if (!player || !npc || IS_NPC(player))
        return;

    action = create_action_impact(player, PLAYER_ACTION_HARM_NPC);
    action->target_mob = npc;
    action->safety_impact = -10;
    action->population_impact = -5;

    sprintf(buf, "Harmed %s", npc->short_descr);
    action->action_description = str_dup(buf);
    action->significant = TRUE;

    /* Update reputation negatively */
    if (action->area_affected)
        update_player_reputation(player, action->area_affected, -20);
}

/*****************************************************************************
 * Reputation System
 *****************************************************************************/

void update_player_reputation(CHAR_DATA *player, AREA_DATA *area, int change)
{
    /* Track player reputation in this area */
    /* This would integrate with a reputation tracking system */
    {
        char log_buf[256];
    sprintf(log_buf, "REPUTATION: %s's reputation in %s changed by %d", (char *)player->name, area->name, change);
    log_string(log_buf);
    }
}

int get_player_reputation(CHAR_DATA *player, AREA_DATA *area)
{
    /* Get player's reputation score in area */
    /* Would load from saved data */
    return 0; /* Neutral */
}

char *get_reputation_title(CHAR_DATA *player, AREA_DATA *area)
{
    int rep = get_player_reputation(player, area);

    if (rep >= 80)
        return "Hero";
    else if (rep >= 50)
        return "Protector";
    else if (rep >= 20)
        return "Friend";
    else if (rep <= -80)
        return "Destroyer";
    else if (rep <= -50)
        return "Enemy";
    else if (rep <= -20)
        return "Troublemaker";

    return "Stranger";
}

bool is_player_hero(CHAR_DATA *player, AREA_DATA *area)
{
    return (get_player_reputation(player, area) >= 80);
}

bool is_player_villain(CHAR_DATA *player, AREA_DATA *area)
{
    return (get_player_reputation(player, area) <= -80);
}

/*****************************************************************************
 * Gradual Change System - Changes happen over time
 *****************************************************************************/

GRADUAL_CHANGE *create_gradual_change(AREA_DATA *area, int change_type, int duration_hours)
{
    GRADUAL_CHANGE *change;
    static int next_change_id = 1;

    CREATE(change, GRADUAL_CHANGE, 1);
    change->change_id = next_change_id++;
    change->change_description = str_dup("Gradual change in progress");
    change->area = area;
    change->change_type = change_type;
    change->start_time = time(NULL);
    change->total_duration_hours = duration_hours;
    change->hours_elapsed = 0;
    change->progress_percentage = 0;
    change->estimated_completion = change->start_time + (duration_hours * 3600 / GAME_TIME_MULTIPLIER);
    change->completion_effect = NULL;
    change->rooms_to_add = 0;
    change->npcs_to_spawn = 0;
    change->player_can_help = TRUE;
    change->gold_needed = 0;
    change->num_helpers = 0;
    change->visible_to_players = TRUE;
    change->can_be_disrupted = TRUE;
    change->disruption_events = 0;
    change->next = first_gradual_change;
    first_gradual_change = change;
    total_gradual_changes++;

    {
        char log_buf[256];
    sprintf(log_buf, "GRADUAL CHANGE: Created change #%d in %s (%d hours duration)", change->change_id, area->name, duration_hours);
    log_string(log_buf);
    }

    return change;
}

void update_gradual_changes(void)
{
    GRADUAL_CHANGE *change, *change_next;
    time_t now = time(NULL);
    int elapsed;

    for (change = first_gradual_change; change; change = change_next)
    {
        change_next = change->next;

        /* Calculate elapsed time */
        elapsed = (int)(difftime(now, change->start_time) / 3600) * GAME_TIME_MULTIPLIER;

        if (elapsed > change->hours_elapsed)
        {
            advance_gradual_change(change, elapsed - change->hours_elapsed);
        }

        /* Check if complete */
        if (change->hours_elapsed >= change->total_duration_hours)
        {
            complete_gradual_change(change);
        }
    }
}

void advance_gradual_change(GRADUAL_CHANGE *change, int hours)
{
    if (!change)
        return;

    change->hours_elapsed += hours;
    change->progress_percentage = (change->hours_elapsed * 100) / change->total_duration_hours;

    if (change->progress_percentage > 100)
        change->progress_percentage = 100;

    {
        char log_buf[256];
    sprintf(log_buf, "GRADUAL CHANGE: #%d advanced to %d%%", change->change_id, change->progress_percentage);
    log_string(log_buf);
    }
}

void complete_gradual_change(GRADUAL_CHANGE *change)
{
    if (!change)
        return;

    {
        char log_buf[256];
    sprintf(log_buf, "GRADUAL CHANGE: #%d COMPLETED in %s", change->change_id, (char *)change->area->name);
    log_string(log_buf);
    }

    /* Apply the change */
    if (change->rooms_to_add > 0)
    {
        {
            char log_buf[256];
        sprintf(log_buf, "  Adding %d rooms to %s", change->rooms_to_add, (char *)change->area->name);
    log_string(log_buf);
        }
        /* Would call beeler_execute_growth() */
    }

    if (change->npcs_to_spawn > 0)
    {
        {
            char log_buf[256];
        sprintf(log_buf, "  Spawning %d NPCs in %s", change->npcs_to_spawn, (char *)change->area->name);
    log_string(log_buf);
        }
    }

    /* Announce completion */
    smart_announce(
        (char *)(change->completion_effect ? change->completion_effect : "Area evolution complete"),
        change->change_description,
        EVENT_CATEGORY_CONSTRUCTION,
        ANNOUNCE_PRIORITY_MEDIUM,
        (char *)change->area->name
    );

    /* Record in history */
    record_area_creation("Beeler", (char *)change->area->name, change->change_description);
}

/*****************************************************************************
 * Player Contribution to Gradual Changes
 *****************************************************************************/

void player_contribute_gold(CHAR_DATA *player, GRADUAL_CHANGE *change, int amount)
{
    int acceleration;

    if (!player || !change || amount <= 0)
        return;

    if (change->gold_needed > 0)
    {
        change->gold_needed -= amount;
        if (change->gold_needed < 0)
            change->gold_needed = 0;
    }

    /* Add player to helpers */
    if (change->num_helpers < 20)
    {
        change->helpers[change->num_helpers] = player;
        change->contributions[change->num_helpers] = amount;
        change->num_helpers++;
    }

    /* Calculate acceleration */
    acceleration = calculate_acceleration(change);
    if (acceleration > 0)
    {
        advance_gradual_change(change, acceleration);
        {
            char log_buf[256];
        sprintf(log_buf, "PLAYER CONTRIBUTION: %s accelerated change by %d hours with %d gold", (char *)player->name, acceleration, amount);
    log_string(log_buf);
        }
    }
}

int calculate_acceleration(GRADUAL_CHANGE *change)
{
    int total_contribution = 0;
    int i;

    for (i = 0; i < change->num_helpers; i++)
        total_contribution += change->contributions[i];

    /* Each 1000 gold = 1 hour acceleration */
    return total_contribution / 1000;
}

void disrupt_gradual_change(GRADUAL_CHANGE *change, char *reason, int setback_hours)
{
    if (!change || !change->can_be_disrupted)
        return;

    change->hours_elapsed -= setback_hours;
    if (change->hours_elapsed < 0)
        change->hours_elapsed = 0;

    change->disruption_events++;

    {
        char log_buf[256];
    sprintf(log_buf, "GRADUAL CHANGE: #%d disrupted (%s) - setback %d hours", change->change_id, reason, setback_hours);
    log_string(log_buf);
    }
}

/*****************************************************************************
 * Rate Limiting for Vital Sign Changes
 *****************************************************************************/

bool can_change_vital_sign(AREA_VITAL_SIGNS *vitals, int change_amount)
{
    /* Limit how fast vital signs can change */
    if (abs(change_amount) > MAX_VITAL_SIGN_CHANGE_PER_DAY)
        return FALSE;

    return TRUE;
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_areainfo(CHAR_DATA *ch, char *argument)
{
    AREA_DATA *area;
    AREA_VITAL_SIGNS *vitals;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    area = ch->in_room ? ch->in_room->area : NULL;
    if (!area)
    {
        send_to_char("You are nowhere.\n\r", ch);
        return;
    }

    vitals = beeler_get_area_health(area);
    if (!vitals)
    {
        send_to_char("No area information available.\n\r", ch);
        return;
    }

    sprintf(buf, "&c=== %s ===&w\n\r\n\r", area->name);
    send_to_char(buf, ch);

    sprintf(buf, "Economic Score: %d/100\n\r", vitals->economic_score);
    send_to_char(buf, ch);
    sprintf(buf, "Safety Score: %d/100\n\r", vitals->safety_score);
    send_to_char(buf, ch);
    sprintf(buf, "Population: %d\n\r", vitals->population);
    send_to_char(buf, ch);
    sprintf(buf, "Status: %s\n\r", vitals->status == AREA_STATUS_THRIVING ? "Thriving" :
                                   vitals->status == AREA_STATUS_PROSPEROUS ? "Prosperous" :
                                   vitals->status == AREA_STATUS_STABLE ? "Stable" :
                                   vitals->status == AREA_STATUS_DECLINING ? "Declining" : "Struggling");
    send_to_char(buf, ch);
}

void do_progress(CHAR_DATA *ch, char *argument)
{
    GRADUAL_CHANGE *change;
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    if (IS_NPC(ch))
        return;

    send_to_char("&c=== Ongoing Changes ===&w\n\r\n\r", ch);

    for (change = first_gradual_change; change; change = change->next)
    {
        if (change->visible_to_players && change->area == (ch->in_room ? ch->in_room->area : NULL))
        {
            sprintf(buf, "&Y[%d]&w %s\n\r", change->change_id, change->change_description);
            send_to_char(buf, ch);
            sprintf(buf, "Progress: %d%% - Time remaining: %s\n\r\n\r",
                    change->progress_percentage,
                    format_time_remaining(change->total_duration_hours - change->hours_elapsed));
            send_to_char(buf, ch);
            count++;
        }
    }

    if (count == 0)
        send_to_char("No ongoing changes in this area.\n\r", ch);
}

void do_reputation(CHAR_DATA *ch, char *argument)
{
    AREA_DATA *area;
    char buf[MAX_STRING_LENGTH];
    int rep;

    if (IS_NPC(ch))
        return;

    area = ch->in_room ? ch->in_room->area : NULL;
    if (!area)
    {
        send_to_char("You are nowhere.\n\r", ch);
        return;
    }

    rep = get_player_reputation(ch, area);

    sprintf(buf, "Your reputation in %s: %s (%d)\n\r",
            area->name, get_reputation_title(ch, area), rep);
    send_to_char(buf, ch);
}
