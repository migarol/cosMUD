/*****************************************************************************
 * Family Lineage System
 *
 * Mobs can have children, creating family trees and lineages
 * - Inheritance of traits (stats, professions, appearance)
 * - Generation tracking
 * - Family relationships and dynamics
 * - Bloodlines and dynasties (especially for nobles/kings)
 *
 * Integration: world_history_tracker.h for family events
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "family_lineage.h"
#include "world_history_tracker.h"
#include "periodicos.h"
#include "mob_creation_system.h"


FAMILY_TREE *first_family = NULL;
FAMILY_MEMBER *first_member = NULL;
int total_families = 0;
int total_members = 0;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_family_lineage(void)
{
    log_string("Initializing Family Lineage System...");
    first_family = NULL;
    first_member = NULL;
    total_families = 0;
    total_members = 0;
    log_string("Family Lineage System initialized.");
}

void load_families(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "families.dat");

    if ((fp = fopen(filename, "r")) == NULL)
    {
        log_string("No saved families to load.");
        return;
    }

    log_string("Loading family lineages...");
    /* Load family data */
    fclose(fp);
}

void save_families(void)
{
    FILE *fp;
    char filename[256];
    FAMILY_TREE *family;

    sprintf(filename, "%s%s", SYSTEM_DIR, "families.dat");

    if ((fp = fopen(filename, "w")) == NULL)
    {
        log_string("ERROR: Cannot save families!");
        return;
    }

    fprintf(fp, "#FAMILIES\n");

    for (family = first_family; family; family = family->next)
    {
        fprintf(fp, "FamilyName~ %s~\n", family->family_name);
        fprintf(fp, "Generation %d\n", family->current_generation);
        fprintf(fp, "Members %d\n", family->total_members);
        fprintf(fp, "End\n\n");
    }

    fprintf(fp, "#END\n");
    fclose(fp);
}

/*****************************************************************************
 * Family Creation
 *****************************************************************************/

FAMILY_TREE *create_family(char *family_name, FAMILY_MEMBER *founder)
{
    FAMILY_TREE *family;

    CREATE(family, FAMILY_TREE, 1);
    family->family_name = str_dup(family_name);
    family->founder = founder;
    family->total_members = 1;
    family->current_generation = 1;
    family->next = first_family;
    first_family = family;
    total_families++;

    if (founder)
    {
        founder->generation = 1;
        founder->family_name = str_dup(family_name);
    }

    sprintf(log_buf, "FAMILY LINEAGE: Created family '%s'", family_name);
    log_string(log_buf);

    return family;
}

FAMILY_MEMBER *create_family_member(char *name, int mob_vnum)
{
    FAMILY_MEMBER *member;

    CREATE(member, FAMILY_MEMBER, 1);
    member->name = str_dup(name);
    member->mob_vnum = mob_vnum;
    member->mob_ptr = NULL;
    member->father = NULL;
    member->mother = NULL;
    member->spouse = NULL;
    member->children = NULL;
    member->num_children = 0;
    member->generation = 1;
    member->family_name = NULL;
    member->bloodline = NULL;
    member->born = time(NULL);
    member->died = 0;
    member->is_alive = TRUE;
    member->next_in_family = NULL;
    member->next_global = first_member;
    first_member = member;
    total_members++;

    return member;
}

/*****************************************************************************
 * Family Relationships
 *****************************************************************************/

void set_parents(FAMILY_MEMBER *child, FAMILY_MEMBER *father, FAMILY_MEMBER *mother)
{
    if (!child)
        return;

    child->father = father;
    child->mother = mother;

    /* Determine generation */
    if (father)
        child->generation = father->generation + 1;
    else if (mother)
        child->generation = mother->generation + 1;

    /* Inherit family name */
    if (father && father->family_name)
        child->family_name = str_dup(father->family_name);
    else if (mother && mother->family_name)
        child->family_name = str_dup(mother->family_name);

    sprintf(log_buf, "FAMILY LINEAGE: %s is child of %s and %s (Gen %d)", child->name,
               father ? father->name : "unknown",
               mother ? mother->name : "unknown",
               child->generation);
    log_string(log_buf);
}

void add_child_to_parent(FAMILY_MEMBER *parent, FAMILY_MEMBER *child)
{
    if (!parent || !child)
        return;

    /* Expand children array */
    if (parent->num_children == 0)
    {
        CREATE(parent->children, FAMILY_MEMBER *, 10);
    }

    if (parent->num_children < 10)
    {
        parent->children[parent->num_children] = child;
        parent->num_children++;
    }
}

void marry(FAMILY_MEMBER *spouse1, FAMILY_MEMBER *spouse2)
{
    if (!spouse1 || !spouse2)
        return;

    spouse1->spouse = spouse2;
    spouse2->spouse = spouse1;

    sprintf(log_buf, "FAMILY LINEAGE: %s married %s", spouse1->name, spouse2->name);
    log_string(log_buf);

    /* Announce marriage */
    smart_announce(
        "Noble Marriage",
        "Two families unite in matrimony",
        EVENT_CATEGORY_CULTURE,
        ANNOUNCE_PRIORITY_MEDIUM,
        "Kingdom"
    );

    /* Record in history */
    record_world_event(EVENT_FAMILY, "Marriage ceremony", 7);
}

/*****************************************************************************
 * Birth System - Create offspring
 *****************************************************************************/

FAMILY_MEMBER *birth_child(FAMILY_MEMBER *father, FAMILY_MEMBER *mother, char *child_name)
{
    FAMILY_MEMBER *child;
    CHAR_DATA *child_mob;
    char short_desc[MAX_STRING_LENGTH];

    /* Create family member record */
    child = create_family_member(child_name, 0);
    set_parents(child, father, mother);

    /* Add to parent's children lists */
    if (father)
        add_child_to_parent(father, child);
    if (mother)
        add_child_to_parent(mother, child);

    /* Inherit traits */
    if (father && mother)
    {
        child->inherited_strength = (father->inherited_strength + mother->inherited_strength) / 2;
        child->inherited_intelligence = (father->inherited_intelligence + mother->inherited_intelligence) / 2;
        child->inherited_charisma = (father->inherited_charisma + mother->inherited_charisma) / 2;

        /* Random variation */
        child->inherited_strength += number_range(-2, 2);
        child->inherited_intelligence += number_range(-2, 2);
        child->inherited_charisma += number_range(-2, 2);
    }
    else
    {
        /* Default traits */
        child->inherited_strength = 50;
        child->inherited_intelligence = 50;
        child->inherited_charisma = 50;
    }

    /* Inherit profession (chance) */
    if (father && father->inherited_profession && number_percent() < 60)
    {
        child->inherited_profession = str_dup(father->inherited_profession);
    }

    /* Create actual mob if parents exist in world */
    if (father && father->mob_ptr && father->mob_ptr->in_room)
    {
        sprintf(short_desc, "%s, child of %s", child_name, father->name);

        child_mob = mob_create_npc(father->mob_ptr, CREATION_CHILD, "birth");
        if (child_mob)
        {
            child->mob_vnum = child_mob->pIndexData->vnum;
            child->mob_ptr = child_mob;
        }
    }

    sprintf(log_buf, "FAMILY LINEAGE: Birth! %s born to %s and %s", child_name,
               father ? father->name : "unknown",
               mother ? mother->name : "unknown");
    log_string(log_buf);

    /* Announce significant births (nobles, royalty) */
    if (father && strstr(father->name, "King"))
    {
        smart_announce(
            "Royal Birth",
            "The kingdom celebrates the birth of an heir",
            EVENT_CATEGORY_CULTURE,
            ANNOUNCE_PRIORITY_HIGH,
            "Kingdom"
        );
    }

    /* Record in history */
    record_world_event(EVENT_FAMILY, "Birth announcement", 6);

    return child;
}

/*****************************************************************************
 * Death and Succession
 *****************************************************************************/

void record_death(FAMILY_MEMBER *member)
{
    if (!member)
        return;

    member->died = time(NULL);
    member->is_alive = FALSE;

    sprintf(log_buf, "FAMILY LINEAGE: Death recorded for %s", member->name);
    log_string(log_buf);

    /* Check for succession needs */
    if (strstr(member->name, "King") && member->num_children > 0)
    {
        /* Find eldest child */
        FAMILY_MEMBER *heir = member->children[0];

        sprintf(log_buf, "FAMILY LINEAGE: Succession - %s inherits from %s", heir->name, member->name);
    log_string(log_buf);

        smart_announce(
            "Royal Succession",
            "The crown passes to a new ruler",
            EVENT_CATEGORY_CULTURE,
            ANNOUNCE_PRIORITY_CRITICAL,
            "Kingdom"
        );
    }

    /* Record in history */
    record_world_event(EVENT_FAMILY, "Death in the family", 7);
}

/*****************************************************************************
 * Lineage Queries
 *****************************************************************************/

FAMILY_TREE *find_family(char *family_name)
{
    FAMILY_TREE *family;

    for (family = first_family; family; family = family->next)
    {
        if (!str_cmp(family->family_name, family_name))
            return family;
    }

    return NULL;
}

FAMILY_MEMBER *find_member_by_name(char *name)
{
    FAMILY_MEMBER *member;

    for (member = first_member; member; member = member->next_global)
    {
        if (!str_cmp(member->name, name))
            return member;
    }

    return NULL;
}

int count_descendants(FAMILY_MEMBER *member)
{
    int count = 0;
    int i;

    if (!member)
        return 0;

    for (i = 0; i < member->num_children; i++)
    {
        if (member->children[i])
        {
            count++;
            count += count_descendants(member->children[i]);
        }
    }

    return count;
}

/*****************************************************************************
 * Dynasty Management - For royal families
 *****************************************************************************/

void establish_dynasty(char *dynasty_name, FAMILY_MEMBER *founder)
{
    FAMILY_TREE *family;

    family = create_family(dynasty_name, founder);
    if (founder)
    {
        founder->bloodline = str_dup(dynasty_name);
    }

    sprintf(log_buf, "FAMILY LINEAGE: Established dynasty '%s'", dynasty_name);
    log_string(log_buf);

    smart_announce(
        "New Dynasty Founded",
        dynasty_name,
        EVENT_CATEGORY_CULTURE,
        ANNOUNCE_PRIORITY_HIGH,
        "Kingdom"
    );
}

/*****************************************************************************
 * Update Loop - Natural progression
 *****************************************************************************/

void family_lineage_update(void)
{
    FAMILY_MEMBER *member;
    static time_t last_check = 0;
    time_t now = time(NULL);

    /* Check monthly */
    if (difftime(now, last_check) < (3600))
        return;

    last_check = now;

    /* Process family events */
    for (member = first_member; member; member = member->next_global)
    {
        if (!member->is_alive)
            continue;

        /* Chance of marriage */
        if (!member->spouse && number_percent() < 5)
        {
            /* Find suitable spouse */
            /* This is simplified - would do proper matchmaking */
        }

        /* Chance of children (if married) */
        if (member->spouse && number_percent() < 10)
        {
            char child_name[MAX_STRING_LENGTH];
            sprintf(child_name, "Child of %s", member->name);
            birth_child(member, member->spouse, child_name);
        }
    }
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_family(CHAR_DATA *ch, char *argument)
{
    FAMILY_MEMBER *member;
    char buf[MAX_STRING_LENGTH];
    int i;

    if (IS_NPC(ch))
    {
        /* Show NPC's family if they have one */
        return;
    }

    send_to_char("&c=== Family Lineages ===&w\n\r\n\r", ch);

    sprintf(buf, "Total families: %d\n\r", total_families);
    send_to_char(buf, ch);
    sprintf(buf, "Total members: %d\n\r\n\r", total_members);
    send_to_char(buf, ch);

    /* Show recent family events */
    send_to_char("&GRecent Family Events:&w\n\r", ch);
    for (member = first_member; member && i < 10; member = member->next_global)
    {
        if (member->num_children > 0)
        {
            sprintf(buf, "%s - %d children\n\r", member->name, member->num_children);
            send_to_char(buf, ch);
            i++;
        }
    }
}

void do_lineage(CHAR_DATA *ch, char *argument)
{
    FAMILY_MEMBER *member;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int i;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Show lineage of whom?\n\r", ch);
        return;
    }

    member = find_member_by_name(arg);
    if (!member)
    {
        send_to_char("No such family member found.\n\r", ch);
        return;
    }

    sprintf(buf, "&c=== Lineage of %s ===&w\n\r\n\r", member->name);
    send_to_char(buf, ch);

    if (member->family_name)
    {
        sprintf(buf, "Family: %s\n\r", member->family_name);
        send_to_char(buf, ch);
    }

    sprintf(buf, "Generation: %d\n\r", member->generation);
    send_to_char(buf, ch);

    if (member->father)
    {
        sprintf(buf, "Father: %s\n\r", member->father->name);
        send_to_char(buf, ch);
    }

    if (member->mother)
    {
        sprintf(buf, "Mother: %s\n\r", member->mother->name);
        send_to_char(buf, ch);
    }

    if (member->spouse)
    {
        sprintf(buf, "Spouse: %s\n\r", member->spouse->name);
        send_to_char(buf, ch);
    }

    if (member->num_children > 0)
    {
        send_to_char("\n\r&GChildren:&w\n\r", ch);
        for (i = 0; i < member->num_children; i++)
        {
            if (member->children[i])
            {
                sprintf(buf, "  %s\n\r", member->children[i]->name);
                send_to_char(buf, ch);
            }
        }
    }

    sprintf(buf, "\n\rTotal descendants: %d\n\r", count_descendants(member));
    send_to_char(buf, ch);
}

void do_birth(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    FAMILY_MEMBER *father, *mother, *child;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Usage: birth <father> <mother> <child_name>\n\r", ch);
        return;
    }

    father = find_member_by_name(arg1);
    mother = find_member_by_name(arg2);

    if (!father || !mother)
    {
        send_to_char("Parent not found.\n\r", ch);
        return;
    }

    child = birth_child(father, mother, argument);

    if (child)
        send_to_char("Child born!\n\r", ch);
    else
        send_to_char("Birth failed.\n\r", ch);
}
