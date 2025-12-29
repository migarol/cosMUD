/*****************************************************************************
 * Mob Creation System - NPCs Create Other NPCs and Objects
 *
 * Power level system: who can create what
 * - Blacksmiths create apprentices
 * - Kings hire guards
 * - Merchants create shops
 * Creates actual CHAR_DATA mobs and OBJ_DATA objects
 * Rate limited per mob
 *
 * Integration: world_history_tracker.h, beeler_god_mode.h
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "mob_creation_system.h"
#include "world_history_tracker.h"
#include "beeler_god_mode.h"
#include "periodicos.h"
#include "ollama_integration.h"

/* Power levels - who can create what */
#define POWER_PEASANT       0  /* Can't create anything */
#define POWER_CRAFTSMAN     1  /* Can create apprentices */
#define POWER_MERCHANT      2  /* Can hire workers */
#define POWER_GUILD_MASTER  3  /* Can create guild members */
#define POWER_NOBLE         4  /* Can hire guards, servants */
#define POWER_KING          5  /* Can create armies */

/* Creation tracking */
typedef struct mob_creation_record MOB_CREATION_RECORD;
struct mob_creation_record {
    CHAR_DATA *creator;
    CHAR_DATA *created_mob;
    OBJ_DATA *created_obj;
    time_t when_created;
    char *reason;
    MOB_CREATION_RECORD *next;
};

MOB_CREATION_RECORD *first_creation = NULL;
int total_creations = 0;

/* Rate limiting per mob */
typedef struct mob_creation_limit MOB_CREATION_LIMIT;
struct mob_creation_limit {
    CHAR_DATA *mob;
    int creations_this_week;
    time_t last_reset;
    MOB_CREATION_LIMIT *next;
};

MOB_CREATION_LIMIT *first_limit = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_mob_creation_system(void)
{
    log_string("Initializing Mob Creation System...");
    first_creation = NULL;
    total_creations = 0;
    first_limit = NULL;
    log_string("Mob Creation System initialized.");
}

void load_mob_creations(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "mob_creations.dat");

    if ((fp = fopen(filename, "r")) == NULL)
    {
        log_string("No saved mob creations to load.");
        return;
    }

    log_string("Loading mob creation history...");
    fclose(fp);
}

void save_mob_creations(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "mob_creations.dat");

    if ((fp = fopen(filename, "w")) == NULL)
    {
        log_string("ERROR: Cannot save mob creations!");
        return;
    }

    fprintf(fp, "#MOB_CREATIONS\n");
    /* Save creation records */
    fprintf(fp, "#END\n");
    fclose(fp);
}

/*****************************************************************************
 * Power Level System
 *****************************************************************************/

int get_mob_power_level(CHAR_DATA *mob)
{
    if (!mob || !IS_NPC(mob))
        return POWER_PEASANT;

    /* Check mob description for indicators */
    if (strstr(mob->short_descr, "king") || strstr(mob->short_descr, "King"))
        return POWER_KING;
    if (strstr(mob->short_descr, "lord") || strstr(mob->short_descr, "noble"))
        return POWER_NOBLE;
    if (strstr(mob->short_descr, "guild master") || strstr(mob->short_descr, "guildmaster"))
        return POWER_GUILD_MASTER;
    if (strstr(mob->short_descr, "merchant") || strstr(mob->short_descr, "trader"))
        return POWER_MERCHANT;
    if (strstr(mob->short_descr, "blacksmith") || strstr(mob->short_descr, "craftsman"))
        return POWER_CRAFTSMAN;

    /* Check level */
    if (mob->level >= 50)
        return POWER_NOBLE;
    if (mob->level >= 30)
        return POWER_GUILD_MASTER;
    if (mob->level >= 20)
        return POWER_MERCHANT;
    if (mob->level >= 10)
        return POWER_CRAFTSMAN;

    return POWER_PEASANT;
}

bool can_mob_create(CHAR_DATA *mob, int what_to_create)
{
    int power = get_mob_power_level(mob);

    switch (what_to_create)
    {
        case CREATION_APPRENTICE:
            return (power >= POWER_CRAFTSMAN);
        case CREATION_CITIZEN:
            return (power >= POWER_MERCHANT);
        case CREATION_GUARD:
            return (power >= POWER_NOBLE);
        case CREATION_MERCHANT:
            return (power >= POWER_NOBLE);
        case CREATION_OBJECT:
            return (power >= POWER_CRAFTSMAN);
        default:
            return FALSE;
    }
}

/*****************************************************************************
 * Rate Limiting
 *****************************************************************************/

MOB_CREATION_LIMIT *get_or_create_limit(CHAR_DATA *mob)
{
    MOB_CREATION_LIMIT *limit;
    time_t now = time(NULL);

    for (limit = first_limit; limit; limit = limit->next)
    {
        if (limit->mob == mob)
        {
            /* Reset weekly counter */
            if (difftime(now, limit->last_reset) > (7 * 86400))
            {
                limit->creations_this_week = 0;
                limit->last_reset = now;
            }
            return limit;
        }
    }

    /* Create new limit tracker */
    CREATE(limit, MOB_CREATION_LIMIT, 1);
    limit->mob = mob;
    limit->creations_this_week = 0;
    limit->last_reset = now;
    limit->next = first_limit;
    first_limit = limit;

    return limit;
}

bool mob_creation_rate_limited(CHAR_DATA *mob)
{
    MOB_CREATION_LIMIT *limit = get_or_create_limit(mob);
    int max_per_week;

    /* Max creations per week based on power level */
    switch (get_mob_power_level(mob))
    {
        case POWER_KING:         max_per_week = 10; break;
        case POWER_NOBLE:        max_per_week = 5; break;
        case POWER_GUILD_MASTER: max_per_week = 3; break;
        case POWER_MERCHANT:     max_per_week = 2; break;
        case POWER_CRAFTSMAN:    max_per_week = 1; break;
        default:                 max_per_week = 0; break;
    }

    if (limit->creations_this_week >= max_per_week)
    {
        sprintf(log_buf, "MOB CREATION: Rate limit - %s has created %d/%d this week", mob->short_descr, limit->creations_this_week, max_per_week);
    log_string(log_buf);
        return TRUE;
    }

    return FALSE;
}

void record_mob_creation_event(CHAR_DATA *creator)
{
    MOB_CREATION_LIMIT *limit = get_or_create_limit(creator);
    limit->creations_this_week++;
}

/*****************************************************************************
 * NPC Creation - Create actual CHAR_DATA
 *****************************************************************************/

CHAR_DATA *mob_create_npc(CHAR_DATA *creator, int npc_type, char *reason)
{
    CHAR_DATA *new_mob;
    MOB_INDEX_DATA *pMobIndex;
    char short_desc[MAX_STRING_LENGTH];
    char long_desc[MAX_STRING_LENGTH];
    static int next_mob_vnum = 29000; /* Dynamic mob vnums */

    if (!creator || !IS_NPC(creator))
        return NULL;

    /* Check permissions */
    if (!can_mob_create(creator, npc_type))
    {
        sprintf(log_buf, "MOB CREATION: %s lacks power to create type %d", creator->short_descr, npc_type);
    log_string(log_buf);
        return NULL;
    }

    /* Check rate limit */
    if (mob_creation_rate_limited(creator))
        return NULL;

    /* Find free vnum */
    while (get_mob_index(next_mob_vnum))
        next_mob_vnum++;

    /* Generate description based on type */
    switch (npc_type)
    {
        case CREATION_APPRENTICE:
            sprintf(short_desc, "an apprentice of %s", creator->short_descr);
            sprintf(long_desc, "An apprentice works diligently here.");
            break;
        case CREATION_CITIZEN:
            sprintf(short_desc, "a worker");
            sprintf(long_desc, "A worker toils here.");
            break;
        case CREATION_GUARD:
            sprintf(short_desc, "a guard");
            sprintf(long_desc, "A guard stands watch here.");
            break;
        case CREATION_MERCHANT:
            sprintf(short_desc, "a servant");
            sprintf(long_desc, "A servant attends to duties here.");
            break;
        case CREATION_GUARD:
            sprintf(short_desc, "a soldier");
            sprintf(long_desc, "A soldier stands at attention.");
            break;
        default:
            sprintf(short_desc, "a new NPC");
            sprintf(long_desc, "A new NPC is here.");
            break;
    }

    /* Create MOB_INDEX */
    CREATE(pMobIndex, MOB_INDEX_DATA, 1);
    pMobIndex->vnum = next_mob_vnum;
    pMobIndex->player_name = str_dup("npc created");
    pMobIndex->short_descr = str_dup(short_desc);
    pMobIndex->long_descr = str_dup(long_desc);
    pMobIndex->description = str_dup("This NPC was created dynamically.");
    xSET_BIT(pMobIndex->act, ACT_IS_NPC);
    pMobIndex->affected_by = 0;
    pMobIndex->pShop = NULL;
    pMobIndex->spec_fun = NULL;
    pMobIndex->count = 0;

    /* Set stats based on type */
    pMobIndex->level = creator->level / 2;
    pMobIndex->alignment = creator->alignment;

    /* Create actual mob */
    new_mob = create_mobile(pMobIndex);
    if (!new_mob)
    {
        log_string("ERROR: Failed to create mobile!");
        return NULL;
    }

    /* Place in same room as creator */
    if (creator->in_room)
        char_to_room(new_mob, creator->in_room);

    /* Track creation */
    record_mob_creation_event(creator);

    /* Create creation record */
    MOB_CREATION_RECORD *record;
    CREATE(record, MOB_CREATION_RECORD, 1);
    record->creator = creator;
    record->created_mob = new_mob;
    record->created_obj = NULL;
    record->when_created = time(NULL);
    record->reason = str_dup(reason);
    record->next = first_creation;
    first_creation = record;
    total_creations++;

    /* Record in world history */
    record_mob_creation(creator->short_descr, new_mob->short_descr, reason);

    sprintf(log_buf, "MOB CREATION: %s created %s (vnum %d) - %s", creator->short_descr, new_mob->short_descr, next_mob_vnum, reason);
    log_string(log_buf);

    /* Announce significant creations */
    if (npc_type == CREATION_GUARD || npc_type == CREATION_GUARD)
    {
        char headline[MAX_STRING_LENGTH];
        char body[MAX_STRING_LENGTH];

        sprintf(headline, "%s Recruits New Forces", creator->short_descr);
        sprintf(body, "%s has recruited additional %s. Reason: %s",
                creator->short_descr,
                npc_type == CREATION_GUARD ? "soldiers" : "guards",
                reason);

        smart_announce(headline, body, EVENT_CATEGORY_CONSTRUCTION,
                      ANNOUNCE_PRIORITY_MEDIUM,
                      creator->in_room ? creator->in_room->area->name : "Unknown");
    }

    next_mob_vnum++;
    return new_mob;
}

/*****************************************************************************
 * Object Creation - Create actual OBJ_DATA
 *****************************************************************************/

OBJ_DATA *mob_create_object(CHAR_DATA *creator, int obj_type, char *name, char *description)
{
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    static int next_obj_vnum = 29500; /* Dynamic object vnums */

    if (!creator || !IS_NPC(creator))
        return NULL;

    /* Check permissions */
    if (!can_mob_create(creator, obj_type))
        return NULL;

    /* Check rate limit */
    if (mob_creation_rate_limited(creator))
        return NULL;

    /* Find free vnum */
    while (get_obj_index(next_obj_vnum))
        next_obj_vnum++;

    /* Create OBJ_INDEX */
    CREATE(pObjIndex, OBJ_INDEX_DATA, 1);
    pObjIndex->vnum = next_obj_vnum;
    pObjIndex->name = str_dup(name);
    pObjIndex->short_descr = str_dup(name);
    pObjIndex->description = str_dup(description);

    /* Set type and stats based on obj_type */
    switch (obj_type)
    {
        case CREATION_OBJECT:
            pObjIndex->item_type = ITEM_TRASH;
            pObjIndex->cost = 10;
            pObjIndex->weight = 1;
            break;

        case CREATION_OBJECT:
            pObjIndex->item_type = ITEM_ARMOR;
            pObjIndex->cost = 100;
            pObjIndex->weight = 10;
            break;

        default:
            pObjIndex->item_type = ITEM_TRASH;
            pObjIndex->cost = 1;
            pObjIndex->weight = 1;
            break;
    }

    pObjIndex->wear_flags = ITEM_TAKE;
    pObjIndex->count = 0;

    /* Create actual object */
    obj = create_object(pObjIndex, 0);
    if (!obj)
    {
        log_string("ERROR: Failed to create object!");
        return NULL;
    }

    /* Place in creator's inventory or room */
    if (creator->in_room)
        obj_to_room(obj, creator->in_room);
    else
        obj_to_char(obj, creator);

    /* Track creation */
    record_mob_creation_event(creator);

    /* Create creation record */
    MOB_CREATION_RECORD *record;
    CREATE(record, MOB_CREATION_RECORD, 1);
    record->creator = creator;
    record->created_mob = NULL;
    record->created_obj = obj;
    record->when_created = time(NULL);
    record->reason = str_dup("crafted");
    record->next = first_creation;
    first_creation = record;
    total_creations++;

    sprintf(log_buf, "MOB CREATION: %s created object '%s' (vnum %d)", creator->short_descr, name, next_obj_vnum);
    log_string(log_buf);

    next_obj_vnum++;
    return obj;
}

/*****************************************************************************
 * Automatic Creation Triggers
 *****************************************************************************/

void trigger_mob_creation_based_on_need(AREA_DATA *area)
{
    CHAR_DATA *mob;
    AREA_VITAL_SIGNS *vitals;

    if (!area)
        return;

    vitals = beeler_get_area_health(area);
    if (!vitals)
        return;

    /* Safety low? Kings/nobles create guards */
    if (vitals->safety_score < 40)
    {
        for (mob = first_char; mob; mob = mob->next)
        {
            if (IS_NPC(mob) &&
                mob->in_room &&
                mob->in_room->area == area &&
                get_mob_power_level(mob) >= POWER_NOBLE)
            {
                if (!mob_creation_rate_limited(mob))
                {
                    mob_create_npc(mob, CREATION_GUARD, "Increase area security");
                    sprintf(log_buf, "AUTO CREATION: %s hired guard due to low safety", mob->short_descr);
    log_string(log_buf);
                    break; /* One per update */
                }
            }
        }
    }

    /* Population low? Create workers */
    if (vitals->population < 20)
    {
        for (mob = first_char; mob; mob = mob->next)
        {
            if (IS_NPC(mob) &&
                mob->in_room &&
                mob->in_room->area == area &&
                get_mob_power_level(mob) >= POWER_MERCHANT)
            {
                if (!mob_creation_rate_limited(mob))
                {
                    mob_create_npc(mob, CREATION_CITIZEN, "Increase workforce");
                    sprintf(log_buf, "AUTO CREATION: %s hired worker due to low population", mob->short_descr);
    log_string(log_buf);
                    break;
                }
            }
        }
    }
}

/*****************************************************************************
 * Update Loop
 *****************************************************************************/

void mob_creation_update(void)
{
    AREA_DATA *area;
    static time_t last_check = 0;
    time_t now = time(NULL);

    /* Check every hour */
    if (difftime(now, last_check) < 3600)
        return;

    last_check = now;

    /* Check each area for creation needs */
    for (area = first_area; area; area = area->next)
    {
        if (number_percent() < 20) /* Random chance */
            trigger_mob_creation_based_on_need(area);
    }
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_mobcreate(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *new_mob;
    int npc_type;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char("Usage: mobcreate <type> <reason>\n\r", ch);
        send_to_char("Types: apprentice, worker, guard, servant, soldier\n\r", ch);
        return;
    }

    if (!str_cmp(arg1, "apprentice"))
        npc_type = CREATION_APPRENTICE;
    else if (!str_cmp(arg1, "worker"))
        npc_type = CREATION_CITIZEN;
    else if (!str_cmp(arg1, "guard"))
        npc_type = CREATION_GUARD;
    else if (!str_cmp(arg1, "servant"))
        npc_type = CREATION_MERCHANT;
    else if (!str_cmp(arg1, "soldier"))
        npc_type = CREATION_GUARD;
    else
    {
        send_to_char("Invalid type.\n\r", ch);
        return;
    }

    new_mob = mob_create_npc(ch, npc_type, arg2[0] ? arg2 : "testing");

    if (new_mob)
        send_to_char("NPC created!\n\r", ch);
    else
        send_to_char("Failed to create NPC.\n\r", ch);
}

void do_mobcreations(CHAR_DATA *ch, char *argument)
{
    MOB_CREATION_RECORD *record;
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    send_to_char("&c=== Mob Creation History ===&w\n\r\n\r", ch);

    sprintf(buf, "Total creations: %d\n\r\n\r", total_creations);
    send_to_char(buf, ch);

    for (record = first_creation; record && count < 20; record = record->next)
    {
        if (record->created_mob)
        {
            sprintf(buf, "%s created %s - %s\n\r",
                    record->creator->short_descr,
                    record->created_mob->short_descr,
                    record->reason);
        }
        else if (record->created_obj)
        {
            sprintf(buf, "%s created %s\n\r",
                    record->creator->short_descr,
                    record->created_obj->short_descr);
        }
        send_to_char(buf, ch);
        count++;
    }
}
