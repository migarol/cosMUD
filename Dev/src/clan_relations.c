/*****************************************************************************
 * Clan Relations System - Inter-clan diplomacy, wars, and alliances        *
 * Based on real cosMUD history: RDAF vs Ninjas, OOC conflicts, etc.       *
 *****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

extern CLAN_DATA *first_clan;
extern CLAN_DATA *last_clan;

/* Relation value ranges:
 *  80-100  = Allied (mutual defense pact)
 *  40-79   = Friendly (trade, cooperation)
 *   1-39   = Neutral (default)
 *  -1--39  = Unfriendly (competition)
 * -40--79  = Hostile (cold war)
 * -80--100 = At War (active conflict)
 */

/*
 * Get the relation status string based on value
 */
char *get_relation_status(int value)
{
    if (value >= 80)  return "Allied";
    if (value >= 40)  return "Friendly";
    if (value >= 1)   return "Neutral";
    if (value >= -39) return "Unfriendly";
    if (value >= -79) return "Hostile";
    return "At War";
}

/*
 * Find existing relation between two clans
 */
CLAN_RELATION *find_clan_relation(CLAN_DATA *clan, CLAN_DATA *target)
{
    CLAN_RELATION *rel;

    if (!clan || !target)
        return NULL;

    for (rel = clan->first_relation; rel; rel = rel->next)
    {
        if (rel->with_clan == target)
            return rel;
    }

    return NULL;
}

/*
 * Set relation between two clans (creates if doesn't exist)
 */
void clan_set_relation(CLAN_DATA *clan, CLAN_DATA *target, int value, char *reason)
{
    CLAN_RELATION *rel;
    char buf[MAX_STRING_LENGTH];

    if (!clan || !target)
        return;

    if (clan == target)
        return; /* Can't have relation with self */

    /* Clamp value to -100...100 */
    value = UMAX(-100, UMIN(100, value));

    /* Find or create relation */
    rel = find_clan_relation(clan, target);
    if (!rel)
    {
        CREATE(rel, CLAN_RELATION, 1);
        rel->with_clan = target;
        rel->since = time(NULL);
        LINK(rel, clan->first_relation, clan->last_relation, next, prev);
    }

    rel->value = value;
    if (rel->reason)
        STRFREE(rel->reason);
    rel->reason = STRALLOC(reason ? reason : "No reason given");

    /* Update counters */
    clan->war_declarations = 0;
    clan->active_alliances = 0;
    for (rel = clan->first_relation; rel; rel = rel->next)
    {
        if (rel->value <= -80)
            clan->war_declarations++;
        else if (rel->value >= 80)
            clan->active_alliances++;
    }

    /* Log to clan boards if dramatic change */
    if (value <= -80)
    {
        sprintf(buf, "%s has declared WAR on %s! Reason: %s",
            clan->name, target->name, reason ? reason : "unknown");
        echo_to_all(AT_RED, buf, ECHOTAR_ALL);
    }
    else if (value >= 80)
    {
        sprintf(buf, "%s and %s have formed an ALLIANCE!",
            clan->name, target->name);
        echo_to_all(AT_CYAN, buf, ECHOTAR_ALL);
    }

    save_clan(clan);
}

/*
 * Get relation value between two clans (returns 0 if no relation)
 */
int clan_get_relation(CLAN_DATA *clan, CLAN_DATA *target)
{
    CLAN_RELATION *rel;

    if (!clan || !target)
        return 0;

    rel = find_clan_relation(clan, target);
    return rel ? rel->value : 0;
}

/*
 * Check if two clans are allied
 */
bool clans_are_allied(CLAN_DATA *clan1, CLAN_DATA *clan2)
{
    int rel1, rel2;

    if (!clan1 || !clan2)
        return FALSE;

    /* Must be mutual alliance (both >= 80) */
    rel1 = clan_get_relation(clan1, clan2);
    rel2 = clan_get_relation(clan2, clan1);

    return (rel1 >= 80 && rel2 >= 80);
}

/*
 * Check if two clans are at war
 */
bool clans_at_war(CLAN_DATA *clan1, CLAN_DATA *clan2)
{
    int rel;

    if (!clan1 || !clan2)
        return FALSE;

    /* Either direction counts as war */
    rel = clan_get_relation(clan1, clan2);
    if (rel <= -80)
        return TRUE;

    rel = clan_get_relation(clan2, clan1);
    if (rel <= -80)
        return TRUE;

    return FALSE;
}

/*
 * Declare war on another clan (leader only)
 */
void clan_declare_war(CLAN_DATA *aggressor, CLAN_DATA *target, char *reason)
{
    char buf[MAX_STRING_LENGTH];

    if (!aggressor || !target)
        return;

    if (aggressor == target)
        return;

    /* Set relation to -100 (war) */
    clan_set_relation(aggressor, target, -100, reason);

    /* Target automatically becomes hostile to aggressor */
    if (clan_get_relation(target, aggressor) > -40)
        clan_set_relation(target, aggressor, -80, "Defensive response to war declaration");

    /* Notify world */
    sprintf(buf, "*** WAR DECLARED: %s vs %s ***", aggressor->name, target->name);
    echo_to_all(AT_RED, buf, ECHOTAR_ALL);

    sprintf(buf, "Reason: %s", reason ? reason : "No reason given");
    echo_to_all(AT_RED, buf, ECHOTAR_ALL);
}

/*
 * Make peace between two clans (must be mutual)
 */
void clan_make_peace(CLAN_DATA *clan1, CLAN_DATA *clan2)
{
    char buf[MAX_STRING_LENGTH];

    if (!clan1 || !clan2)
        return;

    if (clan1 == clan2)
        return;

    /* Set both to neutral */
    clan_set_relation(clan1, clan2, 0, "Peace treaty");
    clan_set_relation(clan2, clan1, 0, "Peace treaty");

    /* Notify world */
    sprintf(buf, "*** PEACE TREATY: %s and %s end hostilities ***",
        clan1->name, clan2->name);
    echo_to_all(AT_CYAN, buf, ECHOTAR_ALL);
}

/*
 * Player command: view clan relations
 */
void do_clan_relations(CHAR_DATA *ch, char *argument)
{
    CLAN_DATA *clan;
    CLAN_RELATION *rel;
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    if (IS_NPC(ch) || !ch->pcdata->clan)
    {
        send_to_char("You're not in a clan.\n\r", ch);
        return;
    }

    clan = ch->pcdata->clan;

    sprintf(buf, "&W%s Diplomatic Relations:\n\r", clan->name);
    send_to_char(buf, ch);
    send_to_char("&w----------------------------------------\n\r", ch);

    for (rel = clan->first_relation; rel; rel = rel->next)
    {
        char *color;

        if (rel->value >= 80)
            color = "&C"; /* Cyan for allies */
        else if (rel->value >= 40)
            color = "&G"; /* Green for friendly */
        else if (rel->value >= 1)
            color = "&Y"; /* Yellow for neutral */
        else if (rel->value >= -39)
            color = "&O"; /* Orange for unfriendly */
        else if (rel->value >= -79)
            color = "&R"; /* Red for hostile */
        else
            color = "&r"; /* Dark red for war */

        sprintf(buf, "%s%-20s %s%-12s &w(%+4d)  %s\n\r",
            color, rel->with_clan->name,
            color, get_relation_status(rel->value),
            rel->value,
            rel->reason ? rel->reason : "");
        send_to_char(buf, ch);
        count++;
    }

    if (count == 0)
        send_to_char("&wNo diplomatic relations established.\n\r", ch);

    sprintf(buf, "\n\r&wActive Wars: %d    Active Alliances: %d\n\r",
        clan->war_declarations, clan->active_alliances);
    send_to_char(buf, ch);
}

/*
 * Player command: declare war (leader only)
 */
void do_clan_war(CHAR_DATA *ch, char *argument)
{
    CLAN_DATA *clan, *target;
    char arg[MAX_INPUT_LENGTH];
    char reason[MAX_STRING_LENGTH];

    if (IS_NPC(ch) || !ch->pcdata->clan)
    {
        send_to_char("You're not in a clan.\n\r", ch);
        return;
    }

    clan = ch->pcdata->clan;

    /* Only leader can declare war */
    if (str_cmp(ch->name, clan->leader))
    {
        send_to_char("Only the clan leader can declare war.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: clan_war <clan name> <reason>\n\r", ch);
        return;
    }

    target = get_clan(arg);
    if (!target)
    {
        send_to_char("No such clan.\n\r", ch);
        return;
    }

    if (target == clan)
    {
        send_to_char("You cannot declare war on your own clan!\n\r", ch);
        return;
    }

    strcpy(reason, argument);
    if (reason[0] == '\0')
        strcpy(reason, "No reason given");

    /* Check if already at war */
    if (clans_at_war(clan, target))
    {
        send_to_char("You are already at war with that clan!\n\r", ch);
        return;
    }

    clan_declare_war(clan, target, reason);
    send_to_char("War has been declared!\n\r", ch);
}

/*
 * Player command: propose alliance (leader only)
 */
void do_clan_ally(CHAR_DATA *ch, char *argument)
{
    CLAN_DATA *clan, *target;
    char arg[MAX_INPUT_LENGTH];
    int our_rel, their_rel;

    if (IS_NPC(ch) || !ch->pcdata->clan)
    {
        send_to_char("You're not in a clan.\n\r", ch);
        return;
    }

    clan = ch->pcdata->clan;

    /* Only leader can make alliances */
    if (str_cmp(ch->name, clan->leader))
    {
        send_to_char("Only the clan leader can form alliances.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: clan_ally <clan name>\n\r", ch);
        return;
    }

    target = get_clan(arg);
    if (!target)
    {
        send_to_char("No such clan.\n\r", ch);
        return;
    }

    if (target == clan)
    {
        send_to_char("You cannot ally with your own clan!\n\r", ch);
        return;
    }

    /* Check if at war */
    if (clans_at_war(clan, target))
    {
        send_to_char("You cannot ally with a clan you're at war with! Make peace first.\n\r", ch);
        return;
    }

    our_rel = clan_get_relation(clan, target);
    their_rel = clan_get_relation(target, clan);

    /* Set our relation to allied */
    clan_set_relation(clan, target, 90, "Alliance proposed");

    if (their_rel >= 80)
    {
        send_to_char("Alliance formed! Both clans are now allied.\n\r", ch);
    }
    else
    {
        send_to_char("Alliance proposal sent. Awaiting their acceptance.\n\r", ch);
        send_to_char("(They must also use 'clan_ally' to complete the alliance)\n\r", ch);
    }
}

/*
 * Player command: make peace (leader only)
 */
void do_clan_peace(CHAR_DATA *ch, char *argument)
{
    CLAN_DATA *clan, *target;
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch) || !ch->pcdata->clan)
    {
        send_to_char("You're not in a clan.\n\r", ch);
        return;
    }

    clan = ch->pcdata->clan;

    /* Only leader can make peace */
    if (str_cmp(ch->name, clan->leader))
    {
        send_to_char("Only the clan leader can negotiate peace.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: clan_peace <clan name>\n\r", ch);
        return;
    }

    target = get_clan(arg);
    if (!target)
    {
        send_to_char("No such clan.\n\r", ch);
        return;
    }

    if (target == clan)
    {
        send_to_char("You cannot make peace with your own clan!\n\r", ch);
        return;
    }

    clan_make_peace(clan, target);
    send_to_char("Peace has been declared.\n\r", ch);
}

/*
 * Initialize real faction relations based on cosMUD history
 */
void init_clan_relations(void)
{
    CLAN_DATA *clan, *rdaf, *ninjas, *ooc, *rangers, *drow, *elves;
    CLAN_RELATION *rel;
    char *pipe;
    char target_name[256];
    char real_reason[MAX_STRING_LENGTH];

    log_string("Initializing historical clan relations...");

    /* First pass: Resolve loaded relation target clans */
    for (clan = first_clan; clan; clan = clan->next)
    {
        for (rel = clan->first_relation; rel; rel = rel->next)
        {
            if (!rel->with_clan && rel->reason)
            {
                /* Parse "TARGET_CLAN_NAME|reason" format */
                pipe = strchr(rel->reason, '|');
                if (pipe)
                {
                    int len = pipe - rel->reason;
                    strncpy(target_name, rel->reason, len);
                    target_name[len] = '\0';
                    strcpy(real_reason, pipe + 1);

                    /* Find target clan */
                    rel->with_clan = get_clan(target_name);

                    /* Update reason to real reason */
                    STRFREE(rel->reason);
                    rel->reason = STRALLOC(real_reason);
                }
            }
        }

        /* Update counters */
        clan->war_declarations = 0;
        clan->active_alliances = 0;
        for (rel = clan->first_relation; rel; rel = rel->next)
        {
            if (rel->value <= -80)
                clan->war_declarations++;
            else if (rel->value >= 80)
                clan->active_alliances++;
        }
    }

    /* Find real clans from history */
    rdaf = get_clan("RDAF");
    ninjas = get_clan("Shadow Clan");
    ooc = get_clan("OOC");
    rangers = get_clan("Rangers");
    drow = get_clan("Drow Empire");
    elves = get_clan("Elven Nations");

    /* RDAF vs Ninjas - Ninja Invasion of May 2002 */
    if (rdaf && ninjas)
    {
        clan_declare_war(rdaf, ninjas, "Ninja invasion of Darkhaven, May 2002 - 60-70 ninjas led by Snake Eyes attacked the city");
        log_string("  - RDAF vs Shadow Clan: WAR (historical)");
    }

    /* RDAF vs OOC - Cold War (Argon's infiltration) */
    if (rdaf && ooc)
    {
        clan_set_relation(rdaf, ooc, -50, "Cold war - Argon's infiltration attempts with new PK characters");
        clan_set_relation(ooc, rdaf, -45, "Territorial competition");
        log_string("  - RDAF vs OOC: HOSTILE (cold war)");
    }

    /* RDAF + Rangers - Allied defenders against ninjas */
    if (rdaf && rangers)
    {
        clan_set_relation(rdaf, rangers, 85, "Allied defenders against ninja invasion");
        clan_set_relation(rangers, rdaf, 85, "Mutual defense pact");
        log_string("  - RDAF + Rangers: ALLIED");
    }

    /* Drow vs Surface Elves - Eternal hatred (from Lolth deity file) */
    if (drow && elves)
    {
        clan_declare_war(drow, elves, "Eternal hatred - Lolth despises all surface dwelling elves");
        log_string("  - Drow Empire vs Elven Nations: WAR (eternal)");
    }

    log_string("Clan relations initialized.");
}
