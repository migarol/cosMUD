/*****************************************************************************
 * Mob Housing System - Implementation
 *
 * Every mob deserves a place to call home.
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>
#include "mud.h"
#include "mob_home.h"
#include "mob_identity.h"

/* Global lists */
MOB_HOME *first_mob_home = NULL;
INN_DATA *first_inn = NULL;
DISTRICT_DATA *first_district = NULL;

/* Database paths */
#define HOME_DIR "../data/mob_homes/"
#define INN_DIR "../data/inns/"
#define DISTRICT_DIR "../data/districts/"

/*
 * Initialize housing system
 */
void init_housing_system(void)
{
    log_string("Initializing Mob Housing System...");
    log_string("  - Mob homes: ENABLED");
    log_string("  - Inn system: ENABLED");
    log_string("  - Residential districts: ENABLED");

    /* Create directories if they don't exist */
    system("mkdir -p " HOME_DIR);
    system("mkdir -p " INN_DIR);
    system("mkdir -p " DISTRICT_DIR);

    /* Load existing data */
    load_all_homes();
    load_all_inns();
    load_all_districts();

    log_string("Housing System: READY");
}

/*
 * Create a new mob home
 */
MOB_HOME *create_mob_home(int mob_vnum, int home_vnum, int home_type)
{
    MOB_HOME *home;

    CREATE(home, MOB_HOME, 1);

    home->mob_vnum = mob_vnum;
    home->home_vnum = home_vnum;
    home->home_type = home_type;
    home->home_name = NULL;
    home->home_description = NULL;
    home->owned = FALSE;
    home->rent_cost = 0;
    home->rent_paid_until = 0;
    home->num_furnishings = 0;
    home->furnishing_descriptions = NULL;
    home->district = NULL;
    home->building_name = NULL;
    home->num_neighbors = 0;
    home->neighbor_vnums = NULL;
    home->last_visited = 0;
    home->times_visited = 0;
    home->next = NULL;

    /* Add to global list */
    home->next = first_mob_home;
    first_mob_home = home;

    return home;
}

/*
 * Get mob's home
 */
MOB_HOME *get_mob_home(int mob_vnum)
{
    MOB_HOME *home;

    for (home = first_mob_home; home; home = home->next)
    {
        if (home->mob_vnum == mob_vnum)
            return home;
    }

    return NULL;
}

/*
 * Determine appropriate home type for mob based on identity
 */
int determine_appropriate_home_type(CHAR_DATA *mob)
{
    MOB_IDENTITY *identity;
    extern MOB_IDENTITY *get_mob_identity(int vnum);

    if (!mob || !IS_NPC(mob))
        return HOME_TYPE_APARTMENT;

    identity = get_mob_identity(mob->pIndexData->vnum);

    if (!identity)
        return HOME_TYPE_APARTMENT;  /* Default */

    /* Unique/boss mobs get better homes */
    if (identity->awareness_level >= 4)
        return HOME_TYPE_PALACE;
    else if (identity->awareness_level == 3)
        return HOME_TYPE_MANOR;
    else if (identity->awareness_level == 2)
        return HOME_TYPE_HOUSE;
    else if (identity->awareness_level == 1)
        return HOME_TYPE_APARTMENT;
    else
        return HOME_TYPE_HOVEL;

    /* Special cases */
    if (xIS_SET(mob->act, ACT_PRACTICE) || xIS_SET(mob->act, ACT_TRAIN))
        return HOME_TYPE_HOUSE;  /* Shopkeepers get houses */

    if (xIS_SET(mob->act, ACT_SENTINEL) && strstr(mob->name, "guard"))
        return HOME_TYPE_BARRACKS;  /* Guards in barracks */

    return HOME_TYPE_APARTMENT;
}

/*
 * Assign home to mob (called by AI God or on first spawn)
 */
void assign_home_to_mob(CHAR_DATA *mob)
{
    MOB_HOME *home;
    MOB_IDENTITY *identity;
    DISTRICT_DATA *district;
    int home_vnum;
    int home_type;
    extern MOB_IDENTITY *get_mob_identity(int vnum);

    if (!mob || !IS_NPC(mob))
        return;

    /* Check if already has home */
    home = get_mob_home(mob->pIndexData->vnum);
    if (home)
        return;  /* Already has a home */

    identity = get_mob_identity(mob->pIndexData->vnum);
    if (!identity)
        return;  /* No identity = no home (for now) */

    /* Determine home type */
    home_type = determine_appropriate_home_type(mob);

    /* Find appropriate district */
    /* TODO: Implement district finding based on mob's area */
    district = NULL;  /* For now */

    /* Find available home vnum */
    /* TODO: Implement finding available room in district */
    home_vnum = 0;  /* For now, stub */

    if (home_vnum == 0)
    {
        sprintf(log_buf, "WARNING: Could not find home for %s (vnum %d)",
            mob->name, mob->pIndexData->vnum);
        log_string(log_buf);
        return;
    }

    /* Create home */
    home = create_mob_home(mob->pIndexData->vnum, home_vnum, home_type);

    /* Generate personalized details */
    home->home_name = generate_home_name(mob, home_type);
    home->home_description = generate_home_description(mob, home_type);
    generate_furnishings(home, mob);

    /* Set rent if not owned */
    if (home_type < HOME_TYPE_MANOR)
    {
        home->owned = FALSE;
        home->rent_cost = calculate_home_rent(home_type, district);
    }
    else
    {
        home->owned = TRUE;  /* Famous mobs own their homes */
    }

    /* Save */
    save_mob_home(home);

    sprintf(log_buf, "[HOUSING] %s assigned home: %s (%s)",
        mob->name,
        home->home_name ? home->home_name : "unnamed",
        home_type_name(home_type));
    log_string(log_buf);
}

/*
 * Generate home name
 */
char *generate_home_name(CHAR_DATA *mob, int home_type)
{
    static char name[256];
    MOB_IDENTITY *identity;
    extern MOB_IDENTITY *get_mob_identity(int vnum);

    identity = get_mob_identity(mob->pIndexData->vnum);

    /* Use mob name + home type */
    if (identity && identity->who_am_i)
    {
        /* Extract just the name part */
        sprintf(name, "%s's %s", mob->short_descr, home_type_name(home_type));
    }
    else
    {
        sprintf(name, "A %s %s", mob->short_descr, home_type_name(home_type));
    }

    return str_dup(name);
}

/*
 * Generate home description (AI-powered in future)
 */
char *generate_home_description(CHAR_DATA *mob, int home_type)
{
    static char desc[MAX_STRING_LENGTH];
    MOB_IDENTITY *identity;
    extern MOB_IDENTITY *get_mob_identity(int vnum);

    identity = get_mob_identity(mob->pIndexData->vnum);

    /* For now, generate basic description */
    /* TODO: Call Ollama to generate personalized description */

    sprintf(desc, "This %s reflects the personality of its occupant.\n\r",
        home_type_name(home_type));

    if (identity)
    {
        if (identity->awareness_level >= 3)
            strcat(desc, "Every detail speaks of intelligence and purpose.\n\r");
        else if (identity->awareness_level == 2)
            strcat(desc, "Organized and functional, suited to its owner's needs.\n\r");
        else
            strcat(desc, "Simple and practical, nothing more than necessary.\n\r");

        /* Add based on capabilities */
        if (identity->capabilities & MOB_CAN_WRITE_BOOKS)
            strcat(desc, "Books and scrolls are scattered throughout.\n\r");

        if (identity->capabilities & MOB_CAN_CRAFT_ITEMS)
            strcat(desc, "Tools and materials for crafting fill one corner.\n\r");
    }

    return str_dup(desc);
}

/*
 * Generate furnishings
 */
void generate_furnishings(MOB_HOME *home, CHAR_DATA *mob)
{
    MOB_IDENTITY *identity;
    int num_items = 0;
    extern MOB_IDENTITY *get_mob_identity(int vnum);

    identity = get_mob_identity(mob->pIndexData->vnum);

    if (!identity)
        return;

    /* Determine number of furnishings based on home type */
    switch (home->home_type)
    {
        case HOME_TYPE_HOVEL:      num_items = 2; break;
        case HOME_TYPE_APARTMENT:  num_items = 4; break;
        case HOME_TYPE_HOUSE:      num_items = 6; break;
        case HOME_TYPE_MANOR:      num_items = 10; break;
        case HOME_TYPE_PALACE:     num_items = 15; break;
        case HOME_TYPE_BARRACKS:   num_items = 3; break;
        case HOME_TYPE_DORMITORY:  num_items = 3; break;
        default:                   num_items = 4; break;
    }

    /* Allocate array */
    CREATE(home->furnishing_descriptions, char *, num_items);
    home->num_furnishings = num_items;

    /* Generate descriptions */
    /* TODO: AI-generate based on personality */
    /* For now, placeholders */
    int i;
    for (i = 0; i < num_items; i++)
    {
        home->furnishing_descriptions[i] = str_dup("A piece of furniture.");
    }
}

/*
 * Home type to string
 */
char *home_type_name(int home_type)
{
    switch (home_type)
    {
        case HOME_TYPE_HOVEL:      return "Hovel";
        case HOME_TYPE_APARTMENT:  return "Apartment";
        case HOME_TYPE_HOUSE:      return "House";
        case HOME_TYPE_MANOR:      return "Manor";
        case HOME_TYPE_PALACE:     return "Palace";
        case HOME_TYPE_INN_ROOM:   return "Inn Room";
        case HOME_TYPE_BARRACKS:   return "Barracks";
        case HOME_TYPE_DORMITORY:  return "Dormitory";
        default:                   return "Unknown";
    }
}

/*
 * Calculate rent cost
 */
int calculate_home_rent(int home_type, DISTRICT_DATA *district)
{
    int base_rent = 0;

    switch (home_type)
    {
        case HOME_TYPE_HOVEL:      base_rent = 10; break;
        case HOME_TYPE_APARTMENT:  base_rent = 50; break;
        case HOME_TYPE_HOUSE:      base_rent = 200; break;
        case HOME_TYPE_INN_ROOM:   base_rent = 25; break;
        case HOME_TYPE_BARRACKS:   base_rent = 0; break;  /* Free for guards */
        case HOME_TYPE_DORMITORY:  base_rent = 20; break;
        default:                   base_rent = 50; break;
    }

    /* Adjust for district quality */
    if (district && district->avg_rent > 0)
        base_rent = (base_rent + district->avg_rent) / 2;

    return base_rent;
}

/*
 * Save mob home to file
 */
void save_mob_home(MOB_HOME *home)
{
    FILE *fp;
    char filename[256];
    int i;

    if (!home)
        return;

    sprintf(filename, "%s%d.json", HOME_DIR, home->mob_vnum);

    fp = fopen(filename, "w");
    if (!fp)
    {
        sprintf(log_buf, "ERROR: Could not save home for vnum %d", home->mob_vnum);
        log_string(log_buf);
        return;
    }

    /* Write JSON format */
    fprintf(fp, "{\n");
    fprintf(fp, "  \"mob_vnum\": %d,\n", home->mob_vnum);
    fprintf(fp, "  \"home_vnum\": %d,\n", home->home_vnum);
    fprintf(fp, "  \"home_type\": %d,\n", home->home_type);
    fprintf(fp, "  \"home_name\": \"%s\",\n", home->home_name ? home->home_name : "");
    fprintf(fp, "  \"owned\": %s,\n", home->owned ? "true" : "false");
    fprintf(fp, "  \"rent_cost\": %d,\n", home->rent_cost);

    if (home->home_description)
    {
        fprintf(fp, "  \"description\": \"");
        /* Escape quotes in description */
        char *p;
        for (p = home->home_description; *p; p++)
        {
            if (*p == '"')
                fprintf(fp, "\\\"");
            else if (*p == '\n')
                fprintf(fp, "\\n");
            else if (*p == '\r')
                continue;  /* Skip */
            else
                fprintf(fp, "%c", *p);
        }
        fprintf(fp, "\",\n");
    }

    fprintf(fp, "  \"num_furnishings\": %d,\n", home->num_furnishings);

    if (home->num_furnishings > 0)
    {
        fprintf(fp, "  \"furnishings\": [\n");
        for (i = 0; i < home->num_furnishings; i++)
        {
            fprintf(fp, "    \"%s\"%s\n",
                home->furnishing_descriptions[i] ? home->furnishing_descriptions[i] : "",
                i < home->num_furnishings - 1 ? "," : "");
        }
        fprintf(fp, "  ],\n");
    }

    fprintf(fp, "  \"last_visited\": %ld,\n", (long)home->last_visited);
    fprintf(fp, "  \"times_visited\": %d\n", home->times_visited);
    fprintf(fp, "}\n");

    fclose(fp);
}

/*
 * Simple JSON string parser - extracts value between quotes
 */
static char *parse_json_string(char *line)
{
    char *start, *end;
    static char value[MAX_STRING_LENGTH];

    start = strchr(line, '"');
    if (!start) return NULL;
    start++; /* Skip opening quote */

    end = start;
    while (*end && *end != '"')
    {
        if (*end == '\\' && *(end+1) == '"')
            end += 2; /* Skip escaped quote */
        else if (*end == '\\' && *(end+1) == 'n')
        {
            /* Handle \n */
            end += 2;
        }
        else
            end++;
    }

    if (*end != '"') return NULL;

    /* Copy and unescape */
    char *src = start;
    char *dst = value;
    while (src < end)
    {
        if (*src == '\\' && *(src+1) == '"')
        {
            *dst++ = '"';
            src += 2;
        }
        else if (*src == '\\' && *(src+1) == 'n')
        {
            *dst++ = '\n';
            src += 2;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';

    return str_dup(value);
}

/*
 * Parse JSON integer value
 */
static int parse_json_int(char *line)
{
    char *colon = strchr(line, ':');
    if (!colon) return 0;
    return atoi(colon + 1);
}

/*
 * Parse JSON boolean value
 */
static bool parse_json_bool(char *line)
{
    return (strstr(line, "true") != NULL);
}

/*
 * Load a single mob home from JSON file
 */
static MOB_HOME *load_mob_home_file(char *filename)
{
    FILE *fp;
    char line[MAX_STRING_LENGTH];
    MOB_HOME *home = NULL;
    bool in_furnishings = FALSE;
    int furn_count = 0;

    fp = fopen(filename, "r");
    if (!fp) return NULL;

    CREATE(home, MOB_HOME, 1);

    while (fgets(line, sizeof(line), fp))
    {
        /* Trim whitespace */
        char *p = line;
        while (*p && isspace(*p)) p++;

        if (strstr(p, "\"mob_vnum\":"))
            home->mob_vnum = parse_json_int(p);
        else if (strstr(p, "\"home_vnum\":"))
            home->home_vnum = parse_json_int(p);
        else if (strstr(p, "\"home_type\":"))
            home->home_type = parse_json_int(p);
        else if (strstr(p, "\"home_name\":"))
            home->home_name = parse_json_string(p);
        else if (strstr(p, "\"owned\":"))
            home->owned = parse_json_bool(p);
        else if (strstr(p, "\"rent_cost\":"))
            home->rent_cost = parse_json_int(p);
        else if (strstr(p, "\"description\":"))
            home->home_description = parse_json_string(p);
        else if (strstr(p, "\"num_furnishings\":"))
        {
            home->num_furnishings = parse_json_int(p);
            if (home->num_furnishings > 0)
                CREATE(home->furnishing_descriptions, char *, home->num_furnishings);
        }
        else if (strstr(p, "\"furnishings\":"))
        {
            in_furnishings = TRUE;
            furn_count = 0;
        }
        else if (in_furnishings && furn_count < home->num_furnishings)
        {
            char *value = parse_json_string(p);
            if (value)
                home->furnishing_descriptions[furn_count++] = value;
        }
        else if (strstr(p, "],"))
            in_furnishings = FALSE;
        else if (strstr(p, "\"last_visited\":"))
            home->last_visited = (time_t)parse_json_int(p);
        else if (strstr(p, "\"times_visited\":"))
            home->times_visited = parse_json_int(p);
    }

    fclose(fp);

    /* Add to global list */
    home->next = first_mob_home;
    first_mob_home = home;

    return home;
}

/*
 * Load all homes from disk
 */
void load_all_homes(void)
{
    DIR *dir;
    struct dirent *entry;
    char filepath[512];
    int count = 0;

    dir = opendir(HOME_DIR);
    if (!dir)
    {
        log_string("  - No mob homes directory, skipping");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strstr(entry->d_name, ".json"))
        {
            sprintf(filepath, "%s%s", HOME_DIR, entry->d_name);
            if (load_mob_home_file(filepath))
                count++;
        }
    }

    closedir(dir);

    sprintf(log_buf, "  - Loaded %d mob homes", count);
    log_string(log_buf);
}

/*
 * Load all inns from disk
 */
void load_all_inns(void)
{
    /* TODO: Implement loading from JSON files */
    log_string("  - Loading inns... (TODO)");
}

/*
 * Load all districts from disk
 */
void load_all_districts(void)
{
    /* TODO: Implement loading from JSON files */
    log_string("  - Loading residential districts... (TODO)");
}

/*
 * Mob goes home (pathfinding to home_vnum)
 */
void mob_go_home(CHAR_DATA *mob)
{
    MOB_HOME *home;
    extern bool mob_move_along_path(CHAR_DATA *mob, int target_vnum);

    if (!mob || !IS_NPC(mob))
        return;

    home = get_mob_home(mob->pIndexData->vnum);
    if (!home)
        return;

    /* Use pathfinding to go home */
    if (mob->in_room && mob->in_room->vnum != home->home_vnum)
    {
        mob_move_along_path(mob, home->home_vnum);
    }

    /* Update statistics */
    if (mob->in_room && mob->in_room->vnum == home->home_vnum)
    {
        home->last_visited = time(NULL);
        home->times_visited++;
    }
}

/*
 * Check if mob is at home
 */
bool is_mob_at_home(CHAR_DATA *mob)
{
    MOB_HOME *home;

    if (!mob || !IS_NPC(mob) || !mob->in_room)
        return FALSE;

    home = get_mob_home(mob->pIndexData->vnum);
    if (!home)
        return FALSE;

    return (mob->in_room->vnum == home->home_vnum);
}

/*
 * Command: home - go to your home
 */
void do_home(CHAR_DATA *ch, char *argument)
{
    MOB_HOME *home;

    if (IS_NPC(ch))
    {
        send_to_char("NPCs don't use this command - they go home automatically.\n\r", ch);
        return;
    }

    /* TODO: Implement player homes */
    send_to_char("Player housing coming soon!\n\r", ch);
}

/*
 * Command: innrent - rent inn room
 */
void do_innrent(CHAR_DATA *ch, char *argument)
{
    /* TODO: Implement inn rental */
    send_to_char("Inn rental system coming soon!\n\r", ch);
}

/*
 * Command: innlist - list nearby inns
 */
void do_innlist(CHAR_DATA *ch, char *argument)
{
    /* TODO: Implement inn listing */
    send_to_char("Inn listing coming soon!\n\r", ch);
}

/* Stub implementation - TODO: Implement pathfinding */
bool mob_move_along_path(CHAR_DATA *mob, int target_vnum)
{
    /* TODO: Implement A* pathfinding or similar */
    return FALSE;  /* Return FALSE for now (can't find path) */
}
