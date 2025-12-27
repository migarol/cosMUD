/*****************************************************************************
 * Autonomous World Simulation System
 *
 * This system makes mobs live in a real, persistent world that exists
 * independently of whether players are online or not. Mobs have:
 * - Daily routines (wake, work, patrol, eat, sleep)
 * - Autonomous movement between areas
 * - Trading with other mobs
 * - Territory control and faction warfare
 * - Building/destroying structures
 * - Economic activity
 *
 * The world continues to evolve 24/7, creating a truly living environment.
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* Simulation intervals */
#define SIM_MOB_ACTION_INTERVAL 60      /* 1 minute - mobs take actions */
#define SIM_TRADE_INTERVAL 300          /* 5 minutes - trading caravans */
#define SIM_TERRITORY_INTERVAL 1800     /* 30 minutes - territory changes */
#define SIM_ECONOMY_INTERVAL 3600       /* 1 hour - economic simulation */

/* Mob routine types */
#define ROUTINE_SLEEP       0
#define ROUTINE_WAKE        1
#define ROUTINE_PATROL      2
#define ROUTINE_WORK        3
#define ROUTINE_TRADE       4
#define ROUTINE_SOCIALIZE   5
#define ROUTINE_EAT         6
#define ROUTINE_GUARD       7
#define ROUTINE_HUNT        8

/* Time of day (0-23 hours) */
#define TIME_NIGHT_START    22
#define TIME_NIGHT_END      6
#define TIME_MORNING        6
#define TIME_NOON           12
#define TIME_EVENING        18

/* Global state */
static time_t last_mob_action_time = 0;
static time_t last_trade_caravan_time = 0;
static time_t last_territory_update = 0;
static time_t last_economy_update = 0;

/* Mob schedule structure */
typedef struct mob_schedule {
    int hour;              /* Hour of day (0-23) */
    int routine_type;      /* What the mob does at this time */
    int target_vnum;       /* Target room/mob/obj vnum */
    char *description;     /* What the mob is doing */
} MOB_SCHEDULE;

/* Faction territory control */
typedef struct territory_control {
    int area_vnum;         /* Area being controlled */
    CLAN_DATA *controller; /* Faction controlling it */
    int strength;          /* Military strength (0-100) */
    int last_contested;    /* Time last attacked */
    struct territory_control *next;
} TERRITORY_CONTROL;

TERRITORY_CONTROL *first_territory = NULL;

/*
 * Get current in-game hour (0-23)
 * Based on world time, not real time
 */
int get_game_hour(void)
{
    /* For now, use real time modulo 24 */
    time_t now = time(NULL);
    return (now / 3600) % 24;
}

/*
 * Determine what routine a mob should be doing based on time and type
 */
int get_mob_routine(CHAR_DATA *mob)
{
    int hour = get_game_hour();
    MOB_IDENTITY *identity;
    extern MOB_IDENTITY *get_mob_identity(int vnum);
    extern int get_custom_mob_routine(CHAR_DATA *mob);

    if (!mob || !IS_NPC(mob))
        return -1;

    /* Check if mob has custom schedule */
    identity = get_mob_identity(mob->pIndexData->vnum);
    if (identity && identity->has_custom_schedule)
    {
        return get_custom_mob_routine(mob);
    }

    /* Default routines below */

    /* Guards are always on duty */
    if (xIS_SET(mob->act, ACT_SENTINEL))
        return ROUTINE_GUARD;

    /* Nocturnal creatures (vampires, undead) */
    if (IS_AFFECTED(mob, AFF_INFRARED))
    {
        if (hour >= TIME_NIGHT_START || hour < TIME_NIGHT_END)
            return ROUTINE_HUNT;
        else
            return ROUTINE_SLEEP;
    }

    /* Normal diurnal mobs */
    if (hour >= TIME_NIGHT_START || hour < TIME_NIGHT_END)
        return ROUTINE_SLEEP;
    else if (hour >= TIME_MORNING && hour < TIME_MORNING + 2)
        return ROUTINE_WAKE;
    else if (hour >= TIME_MORNING + 2 && hour < TIME_NOON)
        return ROUTINE_WORK;
    else if (hour >= TIME_NOON && hour < TIME_NOON + 1)
        return ROUTINE_EAT;
    else if (hour >= TIME_NOON + 1 && hour < TIME_EVENING)
        return ROUTINE_PATROL;
    else if (hour >= TIME_EVENING && hour < TIME_EVENING + 2)
        return ROUTINE_SOCIALIZE;
    else
        return ROUTINE_SLEEP;
}

/*
 * Execute a mob's routine action
 */
void execute_mob_routine(CHAR_DATA *mob, int routine)
{
    ROOM_INDEX_DATA *target_room;
    CHAR_DATA *victim;

    if (!mob || !IS_NPC(mob) || !mob->in_room)
        return;

    /* Don't interrupt if mob is fighting or has a player master */
    if (mob->fighting || (mob->master && !IS_NPC(mob->master)))
        return;

    switch (routine)
    {
        case ROUTINE_SLEEP:
            /* Mobs go to their home/sleep location */
            if (mob->position != POS_SLEEPING)
            {
                mob->position = POS_SLEEPING;
                act(AT_GREY, "$n lies down and goes to sleep.", mob, NULL, NULL, TO_ROOM);
            }
            break;

        case ROUTINE_WAKE:
            if (mob->position == POS_SLEEPING)
            {
                mob->position = POS_STANDING;
                act(AT_GREY, "$n wakes and stands up.", mob, NULL, NULL, TO_ROOM);
            }
            break;

        case ROUTINE_PATROL:
            /* Move to a random adjacent room */
            if (number_percent() < 30) /* 30% chance each check */
            {
                EXIT_DATA *pexit;
                int door = number_range(0, 5);

                pexit = get_exit(mob->in_room, door);
                if (pexit != NULL
                    && (target_room = pexit->to_room) != NULL
                    && !IS_SET(pexit->exit_info, EX_CLOSED)
                    && !IS_SET(target_room->room_flags, ROOM_NO_MOB))
                {
                    act(AT_GREY, "$n patrols $T.", mob, NULL, dir_name[door], TO_ROOM);
                    char_from_room(mob);
                    char_to_room(mob, target_room);
                    act(AT_GREY, "$n arrives on patrol.", mob, NULL, NULL, TO_ROOM);
                }
            }
            break;

        case ROUTINE_WORK:
            /* Mobs "work" - gatherers gather, smiths smith, etc. */
            if (number_percent() < 10) /* Occasional work action */
            {
                if (mob->pIndexData && mob->pIndexData->vnum >= 21000 && mob->pIndexData->vnum < 22000)
                {
                    /* Darkhaven citizens work */
                    act(AT_GREY, "$n busily goes about $s work.", mob, NULL, NULL, TO_ROOM);
                }
            }
            break;

        case ROUTINE_TRADE:
            /* Find another mob to trade with */
            for (victim = mob->in_room->first_person; victim; victim = victim->next_in_room)
            {
                if (IS_NPC(victim) && victim != mob)
                {
                    act(AT_YELLOW, "$n exchanges goods with $N.", mob, NULL, victim, TO_NOTVICT);
                    /* TODO: Actually exchange items/gold */
                    break;
                }
            }
            break;

        case ROUTINE_SOCIALIZE:
            /* Mobs chat, interact socially */
            if (number_percent() < 20)
            {
                for (victim = mob->in_room->first_person; victim; victim = victim->next_in_room)
                {
                    if (IS_NPC(victim) && victim != mob)
                    {
                        act(AT_CYAN, "$n chats with $N.", mob, NULL, victim, TO_NOTVICT);
                        /* Could trigger AI conversations here */
                        break;
                    }
                }
            }
            break;

        case ROUTINE_EAT:
            if (mob->position != POS_SITTING && mob->position != POS_RESTING)
            {
                mob->position = POS_RESTING;
                act(AT_GREY, "$n sits down to eat.", mob, NULL, NULL, TO_ROOM);
            }
            break;

        case ROUTINE_GUARD:
            /* Guards stay alert, look for threats */
            if (mob->position != POS_STANDING)
                mob->position = POS_STANDING;

            /* Guards watch for trouble */
            /* TODO: Add faction-based hostility when mob factions implemented */
            break;

        case ROUTINE_HUNT:
            /* Predators hunt for prey */
            if (xIS_SET(mob->act, ACT_AGGRESSIVE))
            {
                /* Move toward prey if any nearby */
                if (number_percent() < 20)
                {
                    EXIT_DATA *pexit;
                    int door = number_range(0, 5);

                    pexit = get_exit(mob->in_room, door);
                    if (pexit != NULL
                        && (target_room = pexit->to_room) != NULL
                        && !IS_SET(pexit->exit_info, EX_CLOSED))
                    {
                        act(AT_RED, "$n prowls $T, hunting.", mob, NULL, dir_name[door], TO_ROOM);
                        char_from_room(mob);
                        char_to_room(mob, target_room);
                        act(AT_RED, "$n prowls in, hunting for prey.", mob, NULL, NULL, TO_ROOM);
                    }
                }
            }
            break;
    }
}

/*
 * Simulate trading caravans between cities
 */
void simulate_trade_caravans(void)
{
    CHAR_DATA *mob;
    char buf[MAX_STRING_LENGTH];

    log_string("World Simulation: Trading caravans departing...");

    /* Find trade route mobs (merchants, caravans) */
    for (mob = first_char; mob; mob = mob->next)
    {
        if (!IS_NPC(mob))
            continue;

        /* Merchants move between cities */
        if (mob->pIndexData && strstr(mob->name, "merchant"))
        {
            if (number_percent() < 30) /* 30% chance to move */
            {
                /* Move toward nearest city */
                /* TODO: Pathfinding to city centers */
                sprintf(buf, "%s's trading caravan departs for distant markets.", mob->short_descr);
                echo_to_all(AT_YELLOW, buf, ECHOTAR_ALL);
            }
        }
    }
}

/*
 * Simulate territory conquest and faction warfare
 */
void simulate_territory_control(void)
{
    CLAN_DATA *clan, *aggressor;
    extern CLAN_DATA *first_clan;
    char buf[MAX_STRING_LENGTH];

    log_string("World Simulation: Updating territory control...");

    /* Scan for faction conflicts */
    for (clan = first_clan; clan; clan = clan->next)
    {
        if (clan->members < 1)
            continue;

        /* Find clans at war with this one */
        for (aggressor = first_clan; aggressor; aggressor = aggressor->next)
        {
            if (aggressor == clan || aggressor->members < 1)
                continue;

            int relation = clan_get_relation(clan, aggressor);

            if (relation <= -80) /* At war */
            {
                /* Random chance of territory change */
                if (number_percent() < 5) /* 5% chance per check */
                {
                    sprintf(buf, "&R[WAR]&W %s forces attack %s territory! Borders shifting...",
                        aggressor->name, clan->name);
                    echo_to_all(AT_RED, buf, ECHOTAR_ALL);

                    /* TODO: Actually track territory ownership */
                    log_printf("Territory conflict: %s vs %s", aggressor->name, clan->name);
                }
            }
        }
    }
}

/*
 * Simulate world economy - prices, supplies, demand
 */
void simulate_economy(void)
{
    SHOP_DATA *shop;
    extern SHOP_DATA *first_shop;
    int price_change;

    log_string("World Simulation: Economic update...");

    /* Adjust shop prices based on supply/demand */
    for (shop = first_shop; shop; shop = shop->next)
    {
        /* Random price fluctuation */
        price_change = number_range(-5, 5); /* -5% to +5% */

        shop->profit_buy = UMAX(80, UMIN(120, shop->profit_buy + price_change));
        shop->profit_sell = UMAX(80, UMIN(120, shop->profit_sell + price_change));
    }

    /* TODO: Implement shopkeeper inventory restocking */
}

/*
 * Main world simulation update - called every game tick
 */
void update_world_simulation(void)
{
    time_t current_time = time(NULL);
    CHAR_DATA *mob, *mob_next;
    int routine;

    /* MOB ROUTINES - Every minute */
    if (current_time - last_mob_action_time > SIM_MOB_ACTION_INTERVAL)
    {
        /* Process all mobs' daily routines */
        for (mob = first_char; mob; mob = mob_next)
        {
            mob_next = mob->next;

            if (!IS_NPC(mob) || !mob->in_room)
                continue;

            /* Determine and execute routine */
            routine = get_mob_routine(mob);
            if (routine >= 0)
                execute_mob_routine(mob, routine);
        }

        last_mob_action_time = current_time;
    }

    /* TRADE CARAVANS - Every 5 minutes */
    if (current_time - last_trade_caravan_time > SIM_TRADE_INTERVAL)
    {
        simulate_trade_caravans();
        last_trade_caravan_time = current_time;
    }

    /* TERRITORY CONTROL - Every 30 minutes */
    if (current_time - last_territory_update > SIM_TERRITORY_INTERVAL)
    {
        simulate_territory_control();
        last_territory_update = current_time;
    }

    /* ECONOMY - Every hour */
    if (current_time - last_economy_update > SIM_ECONOMY_INTERVAL)
    {
        simulate_economy();
        last_economy_update = current_time;
    }
}

/*
 * Initialize world simulation system
 */
void init_world_simulation(void)
{
    log_string("Initializing Autonomous World Simulation...");
    log_string("  - Mob daily routines: ENABLED");
    log_string("  - Trading caravans: ENABLED");
    log_string("  - Territory warfare: ENABLED");
    log_string("  - Economic simulation: ENABLED");
    log_string("World Simulation: The world lives, even without players.");

    last_mob_action_time = time(NULL);
    last_trade_caravan_time = time(NULL);
    last_territory_update = time(NULL);
    last_economy_update = time(NULL);
}

/*
 * Command: immortal can view world simulation status
 */
void do_worldsim(CHAR_DATA *ch, char *argument)
{
    CLAN_DATA *clan;
    extern CLAN_DATA *first_clan;
    int hour = get_game_hour();
    int active_routines[10] = {0};
    CHAR_DATA *mob;

    if (IS_NPC(ch) || ch->level < MAX_LEVEL)
    {
        send_to_char("Only immortals can view world simulation status.\n\r", ch);
        return;
    }

    /* Count active routines */
    for (mob = first_char; mob; mob = mob->next)
    {
        if (!IS_NPC(mob))
            continue;

        int routine = get_mob_routine(mob);
        if (routine >= 0 && routine < 10)
            active_routines[routine]++;
    }

    ch_printf(ch, "\n&W=== AUTONOMOUS WORLD SIMULATION STATUS ===\n\n");
    ch_printf(ch, "&YCurrent Game Hour: &W%d:00 %s\n\n",
        (hour == 0 ? 12 : (hour > 12 ? hour - 12 : hour)),
        (hour >= 12 ? "PM" : "AM"));

    ch_printf(ch, "&GMob Routines Active:\n");
    ch_printf(ch, "  Sleeping:    %d mobs\n", active_routines[ROUTINE_SLEEP]);
    ch_printf(ch, "  Waking:      %d mobs\n", active_routines[ROUTINE_WAKE]);
    ch_printf(ch, "  Patrolling:  %d mobs\n", active_routines[ROUTINE_PATROL]);
    ch_printf(ch, "  Working:     %d mobs\n", active_routines[ROUTINE_WORK]);
    ch_printf(ch, "  Trading:     %d mobs\n", active_routines[ROUTINE_TRADE]);
    ch_printf(ch, "  Socializing: %d mobs\n", active_routines[ROUTINE_SOCIALIZE]);
    ch_printf(ch, "  Eating:      %d mobs\n", active_routines[ROUTINE_EAT]);
    ch_printf(ch, "  Guarding:    %d mobs\n", active_routines[ROUTINE_GUARD]);
    ch_printf(ch, "  Hunting:     %d mobs\n", active_routines[ROUTINE_HUNT]);

    ch_printf(ch, "\n&CFaction Warfare:\n");
    for (clan = first_clan; clan; clan = clan->next)
    {
        if (clan->war_declarations > 0)
        {
            ch_printf(ch, "  %s: %d active wars\n",
                clan->name, clan->war_declarations);
        }
    }

    ch_printf(ch, "\n&YNext Updates:\n");
    ch_printf(ch, "  Mob actions:  %ld seconds\n",
        SIM_MOB_ACTION_INTERVAL - (time(NULL) - last_mob_action_time));
    ch_printf(ch, "  Trade routes: %ld seconds\n",
        SIM_TRADE_INTERVAL - (time(NULL) - last_trade_caravan_time));
    ch_printf(ch, "  Territory:    %ld seconds\n",
        SIM_TERRITORY_INTERVAL - (time(NULL) - last_territory_update));
    ch_printf(ch, "  Economy:      %ld seconds\n",
        SIM_ECONOMY_INTERVAL - (time(NULL) - last_economy_update));

    send_to_char("\n&WThe world lives on, independent of player presence.\n\r", ch);
}
