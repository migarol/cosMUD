/****************************************************************************
 * Living World Systems - Make the MUD feel ALIVE
 *
 * Features:
 * 1. NPC Memory System (reputation, last interactions)
 * 2. World Events (invasions, festivals, eclipses)
 * 3. Day/Night Effects (mob behavior, spawns, bonuses)
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "mud.h"

/* ========== NPC MEMORY SYSTEM ========== */

#define MAX_NPC_MEMORIES 100
#define MEMORY_FILE_PATH "../npc_memory/%d.mem"
#define REP_HOSTILE -50
#define REP_FRIENDLY 50

/* NPC_MEMORY structure is now in mud.h */

NPC_MEMORY *npc_memory_list = NULL;

/* Load NPC's memory about a player */
NPC_MEMORY *load_npc_memory(int mob_vnum, char *player_name)
{
    FILE *fp;
    char filename[256];
    NPC_MEMORY *mem;

    sprintf(filename, MEMORY_FILE_PATH, mob_vnum);

    if ((fp = fopen(filename, "r")) == NULL)
        return NULL;

    while (!feof(fp))
    {
        mem = (NPC_MEMORY *)malloc(sizeof(NPC_MEMORY));

        if (fscanf(fp, "%s %d %ld %d %d\n%[^\n]\n",
                   mem->player_name,
                   &mem->reputation,
                   &mem->last_interaction,
                   &mem->times_killed_by,
                   &mem->times_helped,
                   mem->last_action) == 6)
        {
            if (!strcmp(mem->player_name, player_name))
            {
                fclose(fp);
                return mem;
            }
        }
        free(mem);
    }

    fclose(fp);
    return NULL;
}

/* Save NPC's memory */
void save_npc_memory(int mob_vnum, NPC_MEMORY *mem)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, MEMORY_FILE_PATH, mob_vnum);

    /* Create directory if doesn't exist */
    system("mkdir -p ../npc_memory");

    if ((fp = fopen(filename, "a")) == NULL)
    {
        bug("save_npc_memory: couldn't open file %s", filename);
        return;
    }

    fprintf(fp, "%s %d %ld %d %d\n%s\n",
            mem->player_name,
            mem->reputation,
            mem->last_interaction,
            mem->times_killed_by,
            mem->times_helped,
            mem->last_action);

    fclose(fp);
}

/* Update NPC memory when player interacts */
void update_npc_memory(CHAR_DATA *mob, CHAR_DATA *ch, char *action, int rep_change)
{
    NPC_MEMORY *mem;

    if (IS_NPC(ch) || !IS_NPC(mob))
        return;

    mem = load_npc_memory(mob->pIndexData->vnum, ch->name);

    if (!mem)
    {
        mem = (NPC_MEMORY *)malloc(sizeof(NPC_MEMORY));
        strcpy(mem->player_name, ch->name);
        mem->reputation = 0;
        mem->times_killed_by = 0;
        mem->times_helped = 0;
    }

    mem->reputation += rep_change;
    mem->reputation = URANGE(-100, mem->reputation, 100);
    mem->last_interaction = current_time;
    strncpy(mem->last_action, action, 255);

    if (strstr(action, "killed"))
        mem->times_killed_by++;
    if (strstr(action, "helped"))
        mem->times_helped++;

    save_npc_memory(mob->pIndexData->vnum, mem);
    free(mem);
}

/* Get NPC's greeting based on memory */
char *get_npc_greeting(CHAR_DATA *mob, CHAR_DATA *ch)
{
    NPC_MEMORY *mem;
    static char greeting[256];

    if (IS_NPC(ch) || !IS_NPC(mob))
        return "Hello there.";

    mem = load_npc_memory(mob->pIndexData->vnum, ch->name);

    if (!mem)
        return "Greetings, stranger.";

    if (mem->reputation <= REP_HOSTILE)
        sprintf(greeting, "YOU! I remember you, %s! Get out!", ch->name);
    else if (mem->reputation >= REP_FRIENDLY)
        sprintf(greeting, "Ah, my friend %s! Good to see you again!", ch->name);
    else if (mem->times_killed_by > 0)
        sprintf(greeting, "You've killed me before... I don't forget.");
    else
        sprintf(greeting, "Hello, %s.", ch->name);

    free(mem);
    return greeting;
}

/* ========== WORLD EVENTS SYSTEM ========== */

#define EVENT_CHECK_INTERVAL 7200  /* Check every 2 hours */

typedef struct world_event_data WORLD_EVENT;
struct world_event_data
{
    char *name;
    char *description;
    int probability;   /* 1-100 */
    int duration;      /* Minutes */
    void (*event_func)(void);
};

time_t last_event_check = 0;
WORLD_EVENT *current_event = NULL;
time_t event_end_time = 0;

/* Event: Orc Invasion */
void event_orc_invasion(void)
{
    DESCRIPTOR_DATA *d;

    echo_to_all(AT_RED, "", ECHOTAR_ALL);
    echo_to_all(AT_RED, "&R════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_RED, "&R    ⚔️  ORC INVASION! ⚔️", ECHOTAR_ALL);
    echo_to_all(AT_RED, "&R════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "Hordes of orcs have begun raiding the lands!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "Defend the cities for bonus XP!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);

    /* TODO: Spawn orc mobs in random areas */
}

/* Event: Solar Eclipse */
void event_solar_eclipse(void)
{
    echo_to_all(AT_BLUE, "", ECHOTAR_ALL);
    echo_to_all(AT_BLUE, "&z════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_BLUE, "&z    🌑 SOLAR ECLIPSE 🌑", ECHOTAR_ALL);
    echo_to_all(AT_BLUE, "&z════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "The sun is blotted out by darkness!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "Undead grow stronger! Magic power DOUBLED!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);

    /* TODO: Boost undead mobs, double magic damage */
}

/* Event: Merchant Caravan */
void event_merchant_caravan(void)
{
    echo_to_all(AT_YELLOW, "", ECHOTAR_ALL);
    echo_to_all(AT_YELLOW, "&Y════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_YELLOW, "&Y    🏪 MERCHANT CARAVAN 🏪", ECHOTAR_ALL);
    echo_to_all(AT_YELLOW, "&Y════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "A traveling merchant has arrived!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "Rare items available for limited time!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);

    /* TODO: Spawn merchant NPC with rare items */
}

/* Event: Blood Moon */
void event_blood_moon(void)
{
    echo_to_all(AT_RED, "", ECHOTAR_ALL);
    echo_to_all(AT_RED, "&R════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_RED, "&R    🌕 BLOOD MOON RISES! 🌕", ECHOTAR_ALL);
    echo_to_all(AT_RED, "&R════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "The moon turns crimson red!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "Werewolves transform! PvP damage increased!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);
}

/* Event: Festival */
void event_festival(void)
{
    echo_to_all(AT_YELLOW, "", ECHOTAR_ALL);
    echo_to_all(AT_YELLOW, "&Y════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_YELLOW, "&Y    🎉 FESTIVAL BEGINS! 🎉", ECHOTAR_ALL);
    echo_to_all(AT_YELLOW, "&Y════════════════════════════════════════", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "A grand festival has begun!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "Double XP! Double Gold! Enjoy!", ECHOTAR_ALL);
    echo_to_all(AT_WHITE, "", ECHOTAR_ALL);
}

WORLD_EVENT event_table[] = {
    { "Orc Invasion",    "Orcs raid the lands",           15, 60,  event_orc_invasion },
    { "Solar Eclipse",   "Darkness covers the sun",       10, 30,  event_solar_eclipse },
    { "Blood Moon",      "Crimson moon rises",            10, 45,  event_blood_moon },
    { "Merchant Caravan","Traveling merchants arrive",    20, 90,  event_merchant_caravan },
    { "Festival",        "Grand celebration",             15, 120, event_festival },
    { NULL, NULL, 0, 0, NULL }
};

/* Check and trigger random events */
void check_world_events(void)
{
    int i, roll;
    time_t now = current_time;

    /* If event is active, check if it ended */
    if (current_event && now >= event_end_time)
    {
        echo_to_all(AT_WHITE, "", ECHOTAR_ALL);
        echo_to_all(AT_CYAN, "&C[World Event Ended]", ECHOTAR_ALL);
        ch_printf(NULL, "&CThe %s has ended.", current_event->name);
        echo_to_all(AT_WHITE, "", ECHOTAR_ALL);
        current_event = NULL;
    }

    /* Check for new event */
    if (!current_event && (now - last_event_check) >= EVENT_CHECK_INTERVAL)
    {
        last_event_check = now;

        /* Roll for random event */
        for (i = 0; event_table[i].name != NULL; i++)
        {
            roll = number_range(1, 100);
            if (roll <= event_table[i].probability)
            {
                current_event = &event_table[i];
                event_end_time = now + (event_table[i].duration * 60);

                if (event_table[i].event_func)
                    (*event_table[i].event_func)();

                break;
            }
        }
    }
}

/* ========== DAY/NIGHT EFFECTS ========== */

#define HOUR_SUNRISE 6
#define HOUR_SUNSET  20
#define HOUR_MIDNIGHT 0
#define HOUR_NOON 12

/* Check if it's currently night time */
bool is_night_time(void)
{
    return (time_info.hour >= HOUR_SUNSET || time_info.hour < HOUR_SUNRISE);
}

/* Check if it's dawn or dusk (transition times) */
bool is_twilight(void)
{
    return (time_info.hour == HOUR_SUNRISE || time_info.hour == HOUR_SUNSET);
}

/* Apply day/night combat modifiers */
int get_daynight_combat_modifier(CHAR_DATA *ch)
{
    int modifier = 0;

    if (!ch)
        return 0;

    /* Vampires stronger at night, weaker during day */
    if (IS_VAMPIRE(ch))
    {
        if (is_night_time())
            modifier += 25;  /* +25% damage */
        else
            modifier -= 25;  /* -25% damage in daylight */
    }

    /* Paladins/Holy warriors stronger during day */
    if (!IS_NPC(ch) && (ch->class == CLASS_PALADIN || ch->class == CLASS_CLERIC))
    {
        if (!is_night_time())
            modifier += 15;
    }

    /* Magic is stronger during twilight */
    if (!IS_NPC(ch) && is_twilight() && (ch->class == CLASS_MAGE || ch->class == CLASS_WARLOCK))
        modifier += 20;

    return modifier;
}

/* Check if mob should spawn based on time */
bool can_mob_spawn_now(MOB_INDEX_DATA *mob)
{
    if (!mob)
        return TRUE;

    /* For now, all mobs can spawn at any time */
    /* This can be extended with custom mob flags or race checks */

    return TRUE;
}

/* Apply day/night vision modifiers */
int get_vision_modifier(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    int modifier = 0;

    if (!ch || !room)
        return 0;

    /* Humans have trouble seeing at night in dark rooms */
    if (ch->race == RACE_HUMAN && is_night_time() && IS_SET(room->room_flags, ROOM_DARK))
        modifier -= 30;

    /* Elves see better at night */
    if (ch->race == RACE_ELF && is_night_time())
        modifier += 20;

    /* Dwarves see in darkness */
    if (ch->race == RACE_DWARF)
        modifier += 40;

    return modifier;
}

/* Initialize living world systems */
void init_living_world(void)
{
    last_event_check = current_time;
    current_event = NULL;

    log_string("Living World Systems initialized:");
    log_string("  - NPC Memory System: Active");
    log_string("  - World Events: Active (checks every 2 hours)");
    log_string("  - Day/Night Effects: Active");
}

/* Update function called from update.c */
void update_living_world(void)
{
    /* Check for world events every pulse */
    check_world_events();

    /* TODO: Update NPC schedules based on time of day */
    /* TODO: Migrate mobs based on population/resources */
}
