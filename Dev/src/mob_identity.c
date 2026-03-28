/*****************************************************************************
 * Mob Identity System - MINIMAL WORKING VERSION
 *
 * Provides get/create/save for mob identities.
 * Used by beeler_assign.c and universal_mob_ai.c
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"
#include "mob_identity.h"

/* Global lists */
MOB_IDENTITY *first_mob_identity = NULL;
MOB_MEMORY *first_mob_memory = NULL;
MOB_CREATION *first_mob_creation = NULL;

/*****************************************************************************
 * Core Functions
 *****************************************************************************/

MOB_IDENTITY *get_mob_identity(int vnum)
{
    MOB_IDENTITY *id;

    for (id = first_mob_identity; id; id = id->next)
    {
        if (id->mob_vnum == vnum)
            return id;
    }

    return NULL;
}

MOB_IDENTITY *create_mob_identity(int vnum)
{
    MOB_IDENTITY *identity;

    /* Check if already exists */
    identity = get_mob_identity(vnum);
    if (identity)
        return identity;

    CREATE(identity, MOB_IDENTITY, 1);
    identity->mob_vnum = vnum;
    identity->who_am_i = str_dup("Unknown");
    identity->what_i_do = str_dup("Unknown");
    identity->how_i_do_it = str_dup("Unknown");
    identity->where_i_live = str_dup("Unknown");
    identity->where_i_go = str_dup("Unknown");
    identity->my_purpose = str_dup("Unknown");
    identity->capabilities = 0;
    identity->awareness_level = AWARENESS_BASIC;
    identity->mobility = 50;
    identity->writing_style = str_dup("simple");
    identity->books_written = 0;
    identity->craft_specialty = str_dup("none");
    identity->items_crafted = 0;
    identity->trade_skill = 50;
    identity->gold_earned = 0;
    identity->ai_prompt = str_dup("");
    identity->has_custom_schedule = FALSE;
    identity->custom_schedule = NULL;

    /* Add to list */
    identity->next = first_mob_identity;
    first_mob_identity = identity;

    return identity;
}

void save_mob_identity(MOB_IDENTITY *identity)
{
    /* Identities are saved through universal_mob_ai persistence system */
    if (!identity)
        return;
}

void load_mob_identity(int vnum)
{
    /* Loaded through universal_mob_ai persistence */
}

/*****************************************************************************
 * Stub Functions - satisfy linker for unused features
 *****************************************************************************/

void init_mob_identity_system(void) { }

MOB_MEMORY *get_mob_memory(int vnum) { return NULL; }

void add_mob_memory(int vnum, int type, char *target, char *summary, int emotional_impact) { }

MEMORY_ENTRY *find_memory_about(int vnum, char *target_name) { return NULL; }

void save_mob_memory(int vnum) { }

void load_mob_memory(int vnum) { }

void mob_write_book(CHAR_DATA *mob, char *topic) { }

void mob_craft_item(CHAR_DATA *mob, char *item_type) { }

void track_mob_creation(int creator_vnum, int object_vnum, char *name, bool is_book) { }

CUSTOM_SCHEDULE *create_custom_schedule(void) { return NULL; }

void add_schedule_entry(CUSTOM_SCHEDULE *sched, int start, int end, char *activity, char *location) { }

SCHEDULE_ENTRY *get_current_schedule(CUSTOM_SCHEDULE *sched, int hour) { return NULL; }

int get_custom_mob_routine(CHAR_DATA *mob) { return -1; }

void generate_mob_identity(CHAR_DATA *mob) { }

void generate_mob_schedule(MOB_IDENTITY *identity) { }

char *generate_conversation_summary(CHAR_DATA *mob, CHAR_DATA *with, char *conversation)
{
    return str_dup("They had a conversation.");
}

void do_mobidentity(CHAR_DATA *ch, char *argument)
{
    send_to_char("Mob identity system: minimal mode.\n\r", ch);
}

void do_mobmemory(CHAR_DATA *ch, char *argument)
{
    send_to_char("Mob memory system: not active.\n\r", ch);
}

void do_genpersonality(CHAR_DATA *ch, char *argument)
{
    send_to_char("Use 'beeler assign <vnum>' instead.\n\r", ch);
}
