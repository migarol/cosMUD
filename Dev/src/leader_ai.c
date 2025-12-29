/*****************************************************************************
 * Leader AI System - Intelligent Area Leadership
 *
 * Leaders make strategic decisions for their domains: BUILD, TRADE, RECRUIT,
 * DECLARE WAR, DIPLOMACY. Integrates with Beeler for execution, world history
 * for tracking, and periodicos for announcements.
 *
 * "A king without a kingdom is just a man with a crown."
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "mud.h"
#include "leader_ai.h"
#include "beeler_god_mode.h"
#include "world_history_tracker.h"
#include "periodicos.h"
#include "world_context.h"
#include "ollama_integration.h"

/* Global leader list */
LEADER_AI_DATA *first_leader = NULL;
LEADER_AI_DATA *last_leader = NULL;
int num_active_leaders = 0;

/* Area classifications cache */
AREA_CLASSIFICATION *area_classifications[200];
int num_classifications = 0;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_leader_ai(void)
{
    log_string("LEADER AI: Initializing leader system...");

    first_leader = NULL;
    last_leader = NULL;
    num_active_leaders = 0;
    num_classifications = 0;

    /* Scan world for existing leaders */
    scan_world_for_leaders();

    sprintf(log_buf, "LEADER AI: System initialized with %d leaders", num_active_leaders);
    log_string(log_buf);
}

/*****************************************************************************
 * World Scan - Find all leaders at boot
 *****************************************************************************/

void scan_world_for_leaders(void)
{
    AREA_DATA *area;
    ROOM_INDEX_DATA *room;
    CHAR_DATA *mob;
    AREA_CLASSIFICATION *classification;
    int vnum;

    log_string("LEADER AI: Scanning world for leaders...");

    /* Scan all areas */
    for (area = first_area; area; area = area->next)
    {
        if (!area->name)
            continue;

        /* Classify this area */
        classification = classify_area(area);

        if (!classification || !classification->needs_leader)
            continue;

        /* Search for existing leader mob in this area */
        for (vnum = area->low_m_vnum; vnum <= area->hi_m_vnum; vnum++)
        {
            MOB_INDEX_DATA *pMobIndex = get_mob_index(vnum);
            if (!pMobIndex)
                continue;

            /* Check if this mob is a leader type */
            if (strstr(pMobIndex->player_name, "king") ||
                strstr(pMobIndex->player_name, "queen") ||
                strstr(pMobIndex->player_name, "mayor") ||
                strstr(pMobIndex->player_name, "chieftain") ||
                strstr(pMobIndex->player_name, "archmage") ||
                strstr(pMobIndex->player_name, "guildmaster") ||
                strstr(pMobIndex->player_name, "elder") ||
                strstr(pMobIndex->player_name, "warlord"))
            {
                /* Find instance of this mob */
                for (mob = first_char; mob; mob = mob->next)
                {
                    if (IS_NPC(mob) && mob->pIndexData == pMobIndex)
                    {
                        /* Assign leader AI to this mob */
                        assign_leader_ai(mob, classification->suggested_leader_type,
                                       number_range(0, 5)); /* Random personality */
                        sprintf(log_buf, "LEADER AI: Found %s in %s",
                               mob->short_descr, area->name);
                        log_string(log_buf);
                        break;
                    }
                }
            }
        }
    }

    detect_missing_leaders();
}

void detect_missing_leaders(void)
{
    AREA_DATA *area;
    AREA_CLASSIFICATION *classification;
    LEADER_AI_DATA *existing;

    /* Check all areas that should have leaders */
    for (area = first_area; area; area = area->next)
    {
        classification = classify_area(area);

        if (!classification || !classification->needs_leader)
            continue;

        /* Check if area already has a leader */
        existing = find_leader_in_area(area);
        if (!existing)
        {
            sprintf(log_buf, "LEADER AI: WARNING - %s needs leader but has none",
                   area->name);
            log_string(log_buf);
        }
    }
}

/*****************************************************************************
 * Area Classification - Determine if area needs a leader
 *****************************************************************************/

AREA_CLASSIFICATION *classify_area(AREA_DATA *area)
{
    AREA_CLASSIFICATION *classification;
    ROOM_INDEX_DATA *room;
    char buf[MAX_STRING_LENGTH];
    int vnum;
    int rooms_count = 0;
    int mob_count = 0;
    int shop_count = 0;
    int building_count = 0;

    if (!area)
        return NULL;

    /* Check if we already classified this area */
    for (int i = 0; i < num_classifications; i++)
    {
        if (area_classifications[i] && area_classifications[i]->area == area)
            return area_classifications[i];
    }

    /* Create new classification */
    classification = (AREA_CLASSIFICATION *)calloc(1, sizeof(AREA_CLASSIFICATION));
    if (!classification)
        return NULL;

    classification->area = area;

    /* Analyze area contents */
    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);
        if (!room)
            continue;

        rooms_count++;

        /* Check for shops */
        if (FALSE)
            shop_count++;

        /* Check for government/important buildings */
        if (room->name)
        {
            char *lower = str_dup(room->name);
            for (char *p = lower; *p; p++)
                *p = tolower(*p);

            if (strstr(lower, "palace") || strstr(lower, "throne") ||
                strstr(lower, "town hall") || strstr(lower, "capitol") ||
                strstr(lower, "temple") || strstr(lower, "guild hall"))
            {
                building_count++;
            }

            DISPOSE(lower);
        }
    }

    /* Count mobs */
    for (vnum = area->low_m_vnum; vnum <= area->hi_m_vnum; vnum++)
    {
        if (get_mob_index(vnum))
            mob_count++;
    }

    classification->population_estimate = mob_count;
    classification->building_count = building_count;
    classification->has_shops = (shop_count > 0);
    classification->has_government_buildings = (building_count > 0);

    /* Classify based on size and features */
    if (rooms_count >= 100 && building_count >= 3 && shop_count >= 2)
    {
        classification->area_type = AREA_TYPE_MAJOR_CITY;
        classification->needs_leader = TRUE;
        classification->suggested_leader_type = LEADER_TYPE_KING;
        classification->classification_reason = str_dup(
            "Major city with extensive infrastructure, multiple shops, government buildings");
    }
    else if (rooms_count >= 30 && (building_count > 0 || shop_count > 0))
    {
        classification->area_type = AREA_TYPE_TOWN;
        classification->needs_leader = TRUE;
        classification->suggested_leader_type = LEADER_TYPE_MAYOR;
        classification->classification_reason = str_dup(
            "Town-sized settlement with shops and buildings");
    }
    else if (rooms_count >= 10 && mob_count >= 5)
    {
        /* Check if it's a dungeon or wilderness */
        if (strstr(area->name, "dungeon") || strstr(area->name, "crypt") ||
            strstr(area->name, "lair") || strstr(area->name, "cave"))
        {
            classification->area_type = AREA_TYPE_DUNGEON;
            classification->needs_leader = FALSE;
            classification->suggested_leader_type = -1;
            classification->classification_reason = str_dup("Dungeon - no settlement");
        }
        else if (strstr(area->name, "forest") || strstr(area->name, "mountain") ||
                 strstr(area->name, "wilderness") || strstr(area->name, "plains"))
        {
            classification->area_type = AREA_TYPE_WILDERNESS;
            classification->needs_leader = FALSE;
            classification->suggested_leader_type = -1;
            classification->classification_reason = str_dup("Wilderness - no settlement");
        }
        else if (strstr(area->name, "orc") || strstr(area->name, "goblin") ||
                 strstr(area->name, "tribal") || strstr(area->name, "camp"))
        {
            classification->area_type = AREA_TYPE_TRIBAL;
            classification->needs_leader = TRUE;
            classification->suggested_leader_type = LEADER_TYPE_CHIEFTAIN;
            classification->classification_reason = str_dup("Tribal settlement");
        }
        else
        {
            classification->area_type = AREA_TYPE_VILLAGE;
            classification->needs_leader = TRUE;
            classification->suggested_leader_type = LEADER_TYPE_ELDER;
            classification->classification_reason = str_dup("Small village settlement");
        }
    }
    else
    {
        classification->area_type = AREA_TYPE_WILDERNESS;
        classification->needs_leader = FALSE;
        classification->suggested_leader_type = -1;
        classification->classification_reason = str_dup("Too small or unpopulated for leadership");
    }

    /* Cache this classification */
    if (num_classifications < 200)
    {
        area_classifications[num_classifications++] = classification;
    }

    return classification;
}

bool area_needs_leader(AREA_DATA *area)
{
    AREA_CLASSIFICATION *classification = classify_area(area);
    return (classification && classification->needs_leader);
}

int detect_appropriate_leader_type(AREA_DATA *area)
{
    AREA_CLASSIFICATION *classification = classify_area(area);
    return classification ? classification->suggested_leader_type : -1;
}

/*****************************************************************************
 * Leader Creation and Management
 *****************************************************************************/

LEADER_AI_DATA *find_leader_in_area(AREA_DATA *area)
{
    LEADER_AI_DATA *leader;

    for (leader = first_leader; leader; leader = leader->next)
    {
        if (leader->controlled_area == area)
            return leader;
    }

    return NULL;
}

void assign_leader_ai(CHAR_DATA *mob, int leader_type, int personality)
{
    LEADER_AI_DATA *leader;
    AREA_DATA *area;
    char buf[MAX_STRING_LENGTH];

    if (!mob || !IS_NPC(mob))
        return;

    area = mob->in_room ? mob->in_room->area : NULL;
    if (!area)
        return;

    /* Check if this mob already has leader AI */
    for (leader = first_leader; leader; leader = leader->next)
    {
        if (leader->mob == mob)
            return; /* Already has AI */
    }

    /* Create new leader */
    leader = (LEADER_AI_DATA *)calloc(1, sizeof(LEADER_AI_DATA));
    if (!leader)
        return;

    leader->mob = mob;
    leader->leader_type = leader_type;
    leader->personality = personality;
    leader->controlled_area = area;

    /* Set title based on leader type */
    switch (leader_type)
    {
        case LEADER_TYPE_KING:
            sprintf(buf, "King of %s", area->name);
            break;
        case LEADER_TYPE_QUEEN:
            sprintf(buf, "Queen of %s", area->name);
            break;
        case LEADER_TYPE_MAYOR:
            sprintf(buf, "Mayor of %s", area->name);
            break;
        case LEADER_TYPE_CHIEFTAIN:
            sprintf(buf, "Chieftain of %s", area->name);
            break;
        case LEADER_TYPE_ARCHMAGE:
            sprintf(buf, "Archmage of %s", area->name);
            break;
        case LEADER_TYPE_ELDER:
            sprintf(buf, "Elder of %s", area->name);
            break;
        case LEADER_TYPE_WARLORD:
            sprintf(buf, "Warlord of %s", area->name);
            break;
        default:
            sprintf(buf, "Leader of %s", area->name);
            break;
    }
    leader->title = str_dup(buf);

    sprintf(buf, "%s %s", mob->short_descr, leader->title);
    leader->formal_name = str_dup(buf);

    /* Initialize resources based on area size and type */
    leader->treasury = number_range(10000, 100000);
    leader->military_power = number_range(10, 100);
    leader->economic_power = area->high_economy;
    leader->population = number_range(50, 500);

    /* Set capabilities */
    leader->can_build = TRUE;
    leader->can_trade = TRUE;
    leader->can_declare_war = (leader_type != LEADER_TYPE_ELDER);
    leader->can_recruit = TRUE;

    /* Initialize relationships */
    leader->num_relations = 0;
    leader->relations = NULL;

    /* Initialize plans */
    leader->num_active_plans = 0;
    leader->active_plans = NULL;

    /* AI settings */
    leader->last_decision_time = current_time;
    leader->decision_interval = number_range(3600, 7200); /* 1-2 hours */
    leader->last_decision = str_dup("Initial planning");

    /* History */
    leader->wars_started = 0;
    leader->buildings_built = 0;
    leader->trade_routes_established = 0;
    leader->years_in_power = 0;

    /* Add to global list */
    LINK(leader, first_leader, last_leader, next, prev);
    num_active_leaders++;

    sprintf(log_buf, "LEADER AI: Assigned to %s in %s",
           mob->short_descr, area->name);
    log_string(log_buf);
}

/*****************************************************************************
 * AI Decision Making
 *****************************************************************************/

void leader_think(LEADER_AI_DATA *leader)
{
    int action;
    char *decision_context;

    if (!leader || !leader->mob)
        return;

    /* Check if it's time to make a decision */
    if (current_time - leader->last_decision_time < leader->decision_interval)
        return;

    /* Update last decision time */
    leader->last_decision_time = current_time;

    /* Choose an action */
    action = leader_choose_action(leader);

    /* Execute the action */
    leader_execute_action(leader, action, NULL);
}

int leader_choose_action(LEADER_AI_DATA *leader)
{
    char *context;
    char *ai_decision;
    int action = ACTION_NOTHING;

    if (!leader)
        return ACTION_NOTHING;

    /* Build context for AI */
    context = build_leader_context(leader);

    /* Try to use Ollama for intelligent decision */
    if (ollama_is_available())
    {
        char prompt[MAX_STRING_LENGTH * 2];

        sprintf(prompt,
            "You are %s, a leader with %s personality. "
            "Context: %s\n\n"
            "Choose ONE strategic action from:\n"
            "BUILD - Construct infrastructure\n"
            "TRADE - Establish trade route\n"
            "RECRUIT - Hire guards/workers\n"
            "WAR - Declare war (if threatened)\n"
            "DIPLOMACY - Form alliance\n"
            "NOTHING - Wait and observe\n\n"
            "Respond with ONLY the action word.",
            leader->formal_name,
            (leader->personality == PERSONALITY_AGGRESSIVE ? "aggressive" :
             leader->personality == PERSONALITY_DIPLOMATIC ? "diplomatic" :
             leader->personality == PERSONALITY_GREEDY ? "greedy" :
             leader->personality == PERSONALITY_WISE ? "wise" :
             leader->personality == PERSONALITY_PARANOID ? "paranoid" : "chaotic"),
            context);

        ai_decision = ollama_request(prompt, 50);

        if (ai_decision)
        {
            /* Parse AI response */
            if (strstr(ai_decision, "BUILD"))
                action = ACTION_BUILD;
            else if (strstr(ai_decision, "TRADE"))
                action = ACTION_TRADE;
            else if (strstr(ai_decision, "RECRUIT"))
                action = ACTION_RECRUIT;
            else if (strstr(ai_decision, "WAR"))
                action = ACTION_DECLARE_WAR;
            else if (strstr(ai_decision, "DIPLOMACY"))
                action = ACTION_DIPLOMACY;
            else
                action = ACTION_NOTHING;

            if (leader->last_decision)
                DISPOSE(leader->last_decision);
            leader->last_decision = str_dup(ai_decision);
        }
    }
    else
    {
        /* Fallback: Choose based on personality */
        switch (leader->personality)
        {
            case PERSONALITY_AGGRESSIVE:
                action = number_range(0, 100) < 40 ? ACTION_DECLARE_WAR : ACTION_RECRUIT;
                break;
            case PERSONALITY_DIPLOMATIC:
                action = number_range(0, 100) < 60 ? ACTION_TRADE : ACTION_DIPLOMACY;
                break;
            case PERSONALITY_GREEDY:
                action = number_range(0, 100) < 70 ? ACTION_TRADE : ACTION_BUILD;
                break;
            case PERSONALITY_WISE:
                action = number_range(0, 100) < 50 ? ACTION_BUILD : ACTION_NOTHING;
                break;
            case PERSONALITY_PARANOID:
                action = number_range(0, 100) < 60 ? ACTION_RECRUIT : ACTION_BUILD;
                break;
            default:
                action = number_range(0, 7); /* Random */
                break;
        }
    }

    if (context)
        DISPOSE(context);

    return action;
}

void leader_execute_action(LEADER_AI_DATA *leader, int action, char *details)
{
    char buf[MAX_STRING_LENGTH];
    char beeler_command[MAX_STRING_LENGTH];

    if (!leader || !leader->mob || !leader->controlled_area)
        return;

    switch (action)
    {
        case ACTION_BUILD:
            sprintf(buf, "fortress");
            if (leader->treasury >= 5000)
            {
                leader_build(leader, buf);
                leader->treasury -= 5000;
                leader->buildings_built++;

                /* Record in history */
                sprintf(buf, "%s ordered construction of new fortress in %s",
                       leader->formal_name, leader->controlled_area->name);
                record_world_event(EVENT_BUILDING_BUILT, buf, 6);

                /* Announce */
                sprintf(buf, "%s Orders New Fortress Construction", leader->formal_name);
                smart_announce(buf,
                    "Construction has begun on a new fortress to strengthen defenses.",
                    EVENT_CATEGORY_CONSTRUCTION, ANNOUNCE_PRIORITY_HIGH,
                    leader->controlled_area->name);
            }
            break;

        case ACTION_TRADE:
            /* Find another leader to trade with */
            {
                LEADER_AI_DATA *other = first_leader;
                while (other && (other == leader ||
                       get_relationship_level(leader, other) < RELATION_NEUTRAL))
                {
                    other = other->next;
                }

                if (other)
                {
                    leader_establish_trade(leader, other);
                }
            }
            break;

        case ACTION_RECRUIT:
            leader_recruit(leader, "guards", number_range(5, 20));
            break;

        case ACTION_DECLARE_WAR:
            /* Find hostile target */
            {
                LEADER_AI_DATA *target = first_leader;
                while (target && (target == leader ||
                       get_relationship_level(leader, target) >= RELATION_NEUTRAL))
                {
                    target = target->next;
                }

                if (target && leader->can_declare_war)
                {
                    leader_declare_war(leader, target);
                }
            }
            break;

        case ACTION_DIPLOMACY:
            /* Improve relations with a neighbor */
            {
                LEADER_AI_DATA *other = first_leader;
                if (other && other != leader)
                {
                    update_leader_relationship(leader, other, 10);

                    sprintf(buf, "%s Improves Relations with %s",
                           leader->formal_name, other->formal_name);
                    smart_announce(buf,
                        "Diplomatic envoys have been exchanged, strengthening ties.",
                        EVENT_CATEGORY_DIPLOMACY, ANNOUNCE_PRIORITY_MEDIUM,
                        leader->controlled_area->name);
                }
            }
            break;

        case ACTION_NOTHING:
        default:
            /* Wait and observe */
            break;
    }
}

/*****************************************************************************
 * Specific Actions
 *****************************************************************************/

bool leader_build(LEADER_AI_DATA *leader, char *what_to_build)
{
    char beeler_command[MAX_STRING_LENGTH];

    if (!leader || !leader->can_build)
        return FALSE;

    /* Delegate to Beeler */
    sprintf(beeler_command, "build %s area=%s type=infrastructure rooms=5",
           what_to_build, leader->controlled_area->name);
    beeler_execute_leader_command(leader, beeler_command);

    return TRUE;
}

bool leader_establish_trade(LEADER_AI_DATA *leader, LEADER_AI_DATA *other)
{
    char buf[MAX_STRING_LENGTH];
    int i;

    if (!leader || !other || !leader->can_trade)
        return FALSE;

    /* Check if trade route already exists */
    for (i = 0; i < leader->num_relations; i++)
    {
        if (leader->relations[i].other_leader == other &&
            leader->relations[i].has_trade_route)
        {
            return FALSE; /* Already trading */
        }
    }

    /* Establish trade */
    update_leader_relationship(leader, other, 20);

    /* Mark as having trade route */
    for (i = 0; i < leader->num_relations; i++)
    {
        if (leader->relations[i].other_leader == other)
        {
            leader->relations[i].has_trade_route = TRUE;
            break;
        }
    }

    /* Boost economies */
    leader->economic_power += 50;
    other->economic_power += 50;
    leader->trade_routes_established++;
    other->trade_routes_established++;

    /* Record in history */
    sprintf(buf, "Trade route established between %s and %s",
           leader->controlled_area->name, other->controlled_area->name);
    record_world_event(EVENT_ECONOMIC, buf, 5);

    /* Announce */
    sprintf(buf, "Trade Agreement Between %s and %s",
           leader->controlled_area->name, other->controlled_area->name);
    smart_announce(buf,
        "Merchants now travel freely between the two regions, boosting prosperity.",
        EVENT_CATEGORY_TRADE, ANNOUNCE_PRIORITY_MEDIUM,
        leader->controlled_area->name);

    return TRUE;
}

bool leader_recruit(LEADER_AI_DATA *leader, char *recruit_type, int count)
{
    if (!leader || !leader->can_recruit)
        return FALSE;

    leader->military_power += count;
    leader->treasury -= (count * 100);

    return TRUE;
}

bool leader_declare_war(LEADER_AI_DATA *leader, LEADER_AI_DATA *target)
{
    char buf[MAX_STRING_LENGTH];

    if (!leader || !target || !leader->can_declare_war)
        return FALSE;

    /* Set relationship to war */
    update_leader_relationship(leader, target, -100);

    /* Mark as at war */
    for (int i = 0; i < leader->num_relations; i++)
    {
        if (leader->relations[i].other_leader == target)
        {
            leader->relations[i].at_war = TRUE;
            leader->relations[i].relationship_level = RELATION_AT_WAR;
            break;
        }
    }

    leader->wars_started++;

    /* Record in history */
    sprintf(buf, "%s declared war on %s",
           leader->formal_name, target->formal_name);
    record_world_event(EVENT_WAR_DECLARED, buf, 10);

    /* CRITICAL announcement */
    sprintf(buf, "%s Declares War on %s!",
           leader->formal_name, target->formal_name);
    smart_announce(buf,
        "Hostilities have erupted! Military forces mobilize as war begins.",
        EVENT_CATEGORY_WAR, ANNOUNCE_PRIORITY_CRITICAL,
        leader->controlled_area->name);

    return TRUE;
}

/*****************************************************************************
 * Relationship Management
 *****************************************************************************/

void update_leader_relationship(LEADER_AI_DATA *leader1, LEADER_AI_DATA *leader2, int change)
{
    int i;

    if (!leader1 || !leader2)
        return;

    /* Find existing relationship */
    for (i = 0; i < leader1->num_relations; i++)
    {
        if (leader1->relations[i].other_leader == leader2)
        {
            leader1->relations[i].relationship_level += change;

            /* Clamp */
            if (leader1->relations[i].relationship_level > RELATION_ALLIED)
                leader1->relations[i].relationship_level = RELATION_ALLIED;
            if (leader1->relations[i].relationship_level < RELATION_AT_WAR)
                leader1->relations[i].relationship_level = RELATION_AT_WAR;

            return;
        }
    }

    /* Create new relationship */
    if (!leader1->relations)
    {
        leader1->relations = (struct leader_relation *)calloc(10, sizeof(struct leader_relation));
        leader1->num_relations = 0;
    }

    if (leader1->num_relations < 10)
    {
        leader1->relations[leader1->num_relations].other_leader = leader2;
        leader1->relations[leader1->num_relations].relationship_level = RELATION_NEUTRAL + change;
        leader1->relations[leader1->num_relations].has_trade_route = FALSE;
        leader1->relations[leader1->num_relations].at_war = FALSE;
        leader1->num_relations++;
    }
}

int get_relationship_level(LEADER_AI_DATA *leader1, LEADER_AI_DATA *leader2)
{
    int i;

    if (!leader1 || !leader2)
        return RELATION_NEUTRAL;

    for (i = 0; i < leader1->num_relations; i++)
    {
        if (leader1->relations[i].other_leader == leader2)
            return leader1->relations[i].relationship_level;
    }

    return RELATION_NEUTRAL;
}

bool are_leaders_at_war(LEADER_AI_DATA *leader1, LEADER_AI_DATA *leader2)
{
    return (get_relationship_level(leader1, leader2) == RELATION_AT_WAR);
}

/*****************************************************************************
 * Context Building for AI Decisions
 *****************************************************************************/

char *build_leader_context(LEADER_AI_DATA *leader)
{
    static char context[MAX_STRING_LENGTH * 2];
    char buf[MAX_STRING_LENGTH];

    if (!leader)
        return "";

    sprintf(context, "Treasury: %d gold. Military: %d guards. Economy: %d gold/day. "
                     "Population: %d. ",
           leader->treasury, leader->military_power, leader->economic_power,
           leader->population);

    /* Add relationship info */
    if (leader->num_relations > 0)
    {
        strcat(context, "Relations: ");
        for (int i = 0; i < leader->num_relations && i < 3; i++)
        {
            sprintf(buf, "%s (%s), ",
                   leader->relations[i].other_leader->controlled_area->name,
                   leader->relations[i].relationship_level >= RELATION_FRIENDLY ? "friendly" :
                   leader->relations[i].relationship_level == RELATION_NEUTRAL ? "neutral" :
                   leader->relations[i].at_war ? "AT WAR" : "hostile");
            strcat(context, buf);
        }
    }

    return context;
}

/*****************************************************************************
 * Beeler Integration
 *****************************************************************************/

void beeler_execute_leader_command(LEADER_AI_DATA *leader, char *command)
{
    if (!leader || !command)
        return;

    sprintf(log_buf, "LEADER AI: %s executes: %s",
           leader->formal_name, command);
    log_string(log_buf);

    /* This would call actual Beeler functions */
    /* For now, just log it */
}

/*****************************************************************************
 * Update Loop
 *****************************************************************************/

void leader_ai_update(void)
{
    LEADER_AI_DATA *leader;
    static time_t last_update = 0;

    /* Only update once per game hour */
    if (current_time - last_update < 3600)
        return;

    last_update = current_time;

    /* Update all leaders */
    for (leader = first_leader; leader; leader = leader->next)
    {
        leader_think(leader);
    }
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_leaders(CHAR_DATA *ch, char *argument)
{
    LEADER_AI_DATA *leader;
    char buf[MAX_STRING_LENGTH];

    if (!ch)
        return;

    send_to_char("&WActive Leaders in the World:&w\n\r", ch);
    send_to_char("&c=====================================&w\n\r", ch);

    for (leader = first_leader; leader; leader = leader->next)
    {
        if (!leader->mob || !leader->controlled_area)
            continue;

        sprintf(buf, "&Y%-30s&w %-20s &G%d gold &C%d guards&w\n\r",
               leader->formal_name,
               leader->controlled_area->name,
               leader->treasury,
               leader->military_power);
        send_to_char(buf, ch);
    }

    sprintf(buf, "\n&cTotal leaders: %d&w\n\r", num_active_leaders);
    send_to_char(buf, ch);
}

void do_leaderinfo(CHAR_DATA *ch, char *argument)
{
    LEADER_AI_DATA *leader;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    if (!ch)
        return;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Usage: leaderinfo <leader name or area>\n\r", ch);
        return;
    }

    /* Find leader */
    for (leader = first_leader; leader; leader = leader->next)
    {
        if (!leader->controlled_area)
            continue;

        if (!str_cmp(arg, leader->controlled_area->name) ||
            strstr(leader->formal_name, arg))
        {
            /* Display detailed info */
            sprintf(buf, "\n&W%s&w\n", leader->formal_name);
            send_to_char(buf, ch);
            send_to_char("&c=====================================&w\n\r", ch);

            sprintf(buf, "Domain: &Y%s&w\n\r", leader->controlled_area->name);
            send_to_char(buf, ch);

            sprintf(buf, "Type: &C%s&w  Personality: &G%s&w\n\r",
                   (leader->leader_type == LEADER_TYPE_KING ? "King" :
                    leader->leader_type == LEADER_TYPE_MAYOR ? "Mayor" :
                    leader->leader_type == LEADER_TYPE_CHIEFTAIN ? "Chieftain" :
                    leader->leader_type == LEADER_TYPE_ELDER ? "Elder" : "Leader"),
                   (leader->personality == PERSONALITY_AGGRESSIVE ? "Aggressive" :
                    leader->personality == PERSONALITY_DIPLOMATIC ? "Diplomatic" :
                    leader->personality == PERSONALITY_GREEDY ? "Greedy" :
                    leader->personality == PERSONALITY_WISE ? "Wise" :
                    leader->personality == PERSONALITY_PARANOID ? "Paranoid" : "Chaotic"));
            send_to_char(buf, ch);

            sprintf(buf, "Treasury: &Y%d gold&w\n\r", leader->treasury);
            send_to_char(buf, ch);

            sprintf(buf, "Military: &R%d guards&w\n\r", leader->military_power);
            send_to_char(buf, ch);

            sprintf(buf, "Economy: &G%d gold/day&w\n\r", leader->economic_power);
            send_to_char(buf, ch);

            sprintf(buf, "Population: &C%d&w\n\r", leader->population);
            send_to_char(buf, ch);

            sprintf(buf, "\nStats: %d wars, %d buildings, %d trade routes\n\r",
                   leader->wars_started, leader->buildings_built,
                   leader->trade_routes_established);
            send_to_char(buf, ch);

            if (leader->last_decision)
            {
                sprintf(buf, "\nLast Decision: &c%s&w\n\r", leader->last_decision);
                send_to_char(buf, ch);
            }

            return;
        }
    }

    send_to_char("No leader found with that name or area.\n\r", ch);
}
