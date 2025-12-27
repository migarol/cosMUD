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
 * Create a new inn
 */
INN_DATA *create_inn(int inn_vnum, char *name)
{
    INN_DATA *inn;

    CREATE(inn, INN_DATA, 1);

    inn->inn_vnum = inn_vnum;
    inn->inn_name = str_dup(name ? name : "The Inn");
    inn->inn_keeper_vnum = 0;
    inn->room_cost_per_night = 50;  /* Default 50 gold/night */
    inn->meal_cost = 10;
    inn->stable_cost = 20;

    /* Default 5 rooms */
    inn->num_rooms = 5;
    CREATE(inn->room_vnums, int, inn->num_rooms);
    CREATE(inn->room_occupied, bool, inn->num_rooms);
    CREATE(inn->room_occupants, int, inn->num_rooms);
    CREATE(inn->checkout_times, time_t, inn->num_rooms);

    inn->on_road = FALSE;
    inn->in_city = TRUE;
    inn->has_stables = FALSE;
    inn->has_tavern = TRUE;
    inn->has_bath = FALSE;
    inn->has_secure_storage = FALSE;
    inn->quality = 50;
    inn->safety = 50;

    /* Add to global list */
    inn->next = first_inn;
    first_inn = inn;

    return inn;
}

/*
 * Get inn by vnum
 */
INN_DATA *get_inn(int inn_vnum)
{
    INN_DATA *inn;

    for (inn = first_inn; inn; inn = inn->next)
    {
        if (inn->inn_vnum == inn_vnum)
            return inn;
    }

    return NULL;
}

/*
 * Save inn to disk (JSON format)
 */
void save_inn(INN_DATA *inn)
{
    FILE *fp;
    char filename[256];
    int i;

    if (!inn)
        return;

    sprintf(filename, "%s%d.json", INN_DIR, inn->inn_vnum);

    fp = fopen(filename, "w");
    if (!fp)
    {
        sprintf(log_buf, "ERROR: Could not save inn %d", inn->inn_vnum);
        log_string(log_buf);
        return;
    }

    /* Write JSON */
    fprintf(fp, "{\n");
    fprintf(fp, "  \"inn_vnum\": %d,\n", inn->inn_vnum);
    fprintf(fp, "  \"inn_name\": \"%s\",\n", inn->inn_name ? inn->inn_name : "");
    fprintf(fp, "  \"inn_keeper_vnum\": %d,\n", inn->inn_keeper_vnum);
    fprintf(fp, "  \"room_cost_per_night\": %d,\n", inn->room_cost_per_night);
    fprintf(fp, "  \"num_rooms\": %d,\n", inn->num_rooms);

    if (inn->num_rooms > 0)
    {
        fprintf(fp, "  \"room_vnums\": [");
        for (i = 0; i < inn->num_rooms; i++)
        {
            fprintf(fp, "%d%s", inn->room_vnums[i],
                i < inn->num_rooms - 1 ? ", " : "");
        }
        fprintf(fp, "],\n");

        fprintf(fp, "  \"room_occupied\": [");
        for (i = 0; i < inn->num_rooms; i++)
        {
            fprintf(fp, "%s%s", inn->room_occupied[i] ? "true" : "false",
                i < inn->num_rooms - 1 ? ", " : "");
        }
        fprintf(fp, "],\n");

        fprintf(fp, "  \"room_occupants\": [");
        for (i = 0; i < inn->num_rooms; i++)
        {
            fprintf(fp, "%d%s", inn->room_occupants[i],
                i < inn->num_rooms - 1 ? ", " : "");
        }
        fprintf(fp, "],\n");

        fprintf(fp, "  \"checkout_times\": [");
        for (i = 0; i < inn->num_rooms; i++)
        {
            fprintf(fp, "%ld%s", (long)inn->checkout_times[i],
                i < inn->num_rooms - 1 ? ", " : "");
        }
        fprintf(fp, "]\n");
    }

    fprintf(fp, "}\n");
    fclose(fp);
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
 * Check if inn has vacancy
 */
bool inn_has_vacancy(INN_DATA *inn)
{
    int i;

    if (!inn || !inn->room_occupied)
        return FALSE;

    for (i = 0; i < inn->num_rooms; i++)
    {
        if (!inn->room_occupied[i])
            return TRUE;
    }

    return FALSE;
}

/*
 * Rent a room at an inn
 * Returns room index (0 to num_rooms-1) on success, -1 on failure
 */
int inn_rent_room(INN_DATA *inn, CHAR_DATA *ch, int nights)
{
    int i;
    time_t checkout_time;

    if (!inn || !ch || !inn->room_occupied)
        return -1;

    /* Find first available room */
    for (i = 0; i < inn->num_rooms; i++)
    {
        if (!inn->room_occupied[i])
        {
            /* Mark room as occupied */
            inn->room_occupied[i] = TRUE;

            /* Set occupant - for players, use a negative value or special ID */
            /* For now, we'll use 0 to indicate player occupation */
            if (inn->room_occupants)
                inn->room_occupants[i] = 0;  /* 0 = player occupied */

            /* Calculate checkout time (nights * 24 hours) */
            checkout_time = time(NULL) + (nights * 24 * 3600);
            if (inn->checkout_times)
                inn->checkout_times[i] = checkout_time;

            /* Save inn data */
            save_inn(inn);

            return i;
        }
    }

    return -1;  /* No vacancy */
}

/*
 * Inn checkout
 */
void inn_checkout(INN_DATA *inn, CHAR_DATA *ch)
{
    int i;

    if (!inn || !ch)
        return;

    /* Find player's rented room */
    for (i = 0; i < inn->num_rooms; i++)
    {
        if (inn->room_occupied[i] && inn->room_occupants[i] == 0)
        {
            /* Clear room */
            inn->room_occupied[i] = FALSE;
            inn->room_occupants[i] = 0;

            if (inn->checkout_times)
                inn->checkout_times[i] = 0;

            save_inn(inn);

            sprintf(log_buf, "%s checked out of inn %s room %d",
                ch->name, inn->inn_name, i);
            log_string(log_buf);

            break;
        }
    }
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
    INN_DATA *inn = NULL;
    ROOM_INDEX_DATA *room;
    int nights = 1;
    int cost;
    int room_index;
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
    {
        send_to_char("NPCs don't need to rent inn rooms.\n\r", ch);
        return;
    }

    /* Find inn in current area */
    for (inn = first_inn; inn; inn = inn->next)
    {
        room = get_room_index(inn->inn_vnum);
        if (room && room->area == ch->in_room->area)
            break;
    }

    if (!inn)
    {
        send_to_char("There's no inn in this area.\n\r", ch);
        send_to_char("Use 'innlist' to find nearby inns.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    /* Parse number of nights */
    if (arg[0] != '\0')
    {
        nights = atoi(arg);
        if (nights < 1 || nights > 30)
        {
            send_to_char("You can rent for 1 to 30 nights.\n\r", ch);
            return;
        }
    }

    /* Check if inn has vacancy */
    if (!inn_has_vacancy(inn))
    {
        send_to_char("Sorry, the inn is fully booked.\n\r", ch);
        return;
    }

    /* Calculate cost */
    cost = inn->room_cost_per_night * nights;

    if (ch->gold < cost)
    {
        ch_printf(ch, "You need %d gold to rent for %d night%s.\n\r",
            cost, nights, nights == 1 ? "" : "s");
        return;
    }

    /* Rent the room */
    room_index = inn_rent_room(inn, ch, nights);

    if (room_index < 0)
    {
        send_to_char("Failed to rent room. Please try again.\n\r", ch);
        return;
    }

    /* Charge gold */
    ch->gold -= cost;

    ch_printf(ch, "\n\r&YThe innkeeper smiles warmly.&w\n\r");
    ch_printf(ch, "\"That'll be %d gold for %d night%s.\"\n\r\n\r",
        cost, nights, nights == 1 ? "" : "s");
    ch_printf(ch, "&GYou've rented room #%d.&w\n\r", inn->room_vnums[room_index]);
    ch_printf(ch, "Use '&Cgoto %d&w' to visit your room.\n\r\n\r", inn->room_vnums[room_index]);

    sprintf(log_buf, "%s rented inn room for %d nights at %s",
        ch->name, nights, inn->inn_name);
    log_string(log_buf);
}

/*
 * Command: innlist - list nearby inns
 */
void do_innlist(CHAR_DATA *ch, char *argument)
{
    INN_DATA *inn;
    ROOM_INDEX_DATA *room;
    int count = 0;
    int vacancy;

    if (IS_NPC(ch))
    {
        send_to_char("NPCs don't need this command.\n\r", ch);
        return;
    }

    send_to_char("\n\r&Y=== Inns in This Realm ===&w\n\r\n\r", ch);
    send_to_char("&CName                        Location           Cost/Night  Vacancy&w\n\r", ch);
    send_to_char("&C------------------------------------------------------------------------&w\n\r", ch);

    for (inn = first_inn; inn; inn = inn->next)
    {
        room = get_room_index(inn->inn_vnum);
        if (!room)
            continue;

        vacancy = 0;
        if (inn->room_occupied)
        {
            int i;
            for (i = 0; i < inn->num_rooms; i++)
            {
                if (!inn->room_occupied[i])
                    vacancy++;
            }
        }

        ch_printf(ch, "%-28s %-18s %4d gold   %d/%d\n\r",
            inn->inn_name ? inn->inn_name : "The Inn",
            room->area ? room->area->name : "Unknown",
            inn->room_cost_per_night,
            vacancy,
            inn->num_rooms);

        count++;
    }

    if (count == 0)
    {
        send_to_char("No inns found. Innkeepers should create some!\n\r", ch);
    }
    else
    {
        ch_printf(ch, "\n\r&YTotal inns: %d&w\n\r", count);
        send_to_char("Use '&Cinnrent <nights>&w' to rent a room.\n\r\n\r", ch);
    }
}

/* Stub implementation - TODO: Implement pathfinding */
bool mob_move_along_path(CHAR_DATA *mob, int target_vnum)
{
    /* TODO: Implement A* pathfinding or similar */
    return FALSE;  /* Return FALSE for now (can't find path) */
}

/*
 * Command: homeslist [area] - List all homes (admin command)
 */
void do_homeslist(CHAR_DATA *ch, char *argument)
{
    MOB_HOME *home;
    AREA_DATA *area = NULL;
    ROOM_INDEX_DATA *room;
    MOB_INDEX_DATA *mob;
    int count = 0;
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
    {
        send_to_char("NPCs cannot use this command.\n\r", ch);
        return;
    }

    if (get_trust(ch) < LEVEL_IMMORTAL)
    {
        send_to_char("Only immortals can view the homes list.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    /* If area specified, find it */
    if (arg[0] != '\0')
    {
        for (area = first_area; area; area = area->next)
        {
            if (!str_cmp(area->name, arg) || !str_cmp(area->filename, arg))
                break;
        }

        if (!area)
        {
            ch_printf(ch, "Area '%s' not found.\n\r", arg);
            return;
        }

        ch_printf(ch, "\n\r&Y=== Homes in %s ===&w\n\r\n\r", area->name);
    }
    else
    {
        send_to_char("\n\r&Y=== All Homes in World ===&w\n\r\n\r", ch);
    }

    send_to_char("&CVnum  Owner (vnum)                Type       Occupied  District&w\n\r", ch);
    send_to_char("&C---------------------------------------------------------------------------&w\n\r", ch);

    for (home = first_mob_home; home; home = home->next)
    {
        /* Filter by area if specified */
        if (area)
        {
            room = get_room_index(home->home_vnum);
            if (!room || room->area != area)
                continue;
        }

        /* Get owner info */
        mob = get_mob_index(home->mob_vnum);

        ch_printf(ch, "&G%-6d&w %-25s %-10s %-9s %s\n\r",
            home->home_vnum,
            mob ? mob->short_descr : "(none)",
            home_type_name(home->home_type),
            home->owned ? "&GOwned&w" : "&YRent&w",
            home->district ? home->district : "None");

        count++;
    }

    if (count == 0)
    {
        if (area)
            ch_printf(ch, "No homes found in %s.\n\r", area->name);
        else
            send_to_char("No homes found in the world.\n\r", ch);
    }
    else
    {
        ch_printf(ch, "\n\r&YTotal homes: %d&w\n\r\n\r", count);
    }
}

/*
 * Command: homeinfo <vnum> - Show detailed home info (admin command)
 */
void do_homeinfo(CHAR_DATA *ch, char *argument)
{
    MOB_HOME *home;
    ROOM_INDEX_DATA *room;
    MOB_INDEX_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int vnum;
    int i;

    if (IS_NPC(ch))
    {
        send_to_char("NPCs cannot use this command.\n\r", ch);
        return;
    }

    if (get_trust(ch) < LEVEL_IMMORTAL)
    {
        send_to_char("Only immortals can view home info.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: homeinfo <room_vnum>\n\r", ch);
        return;
    }

    vnum = atoi(arg);

    /* Search for home by vnum (could be mob_vnum or home_vnum) */
    for (home = first_mob_home; home; home = home->next)
    {
        if (home->home_vnum == vnum || home->mob_vnum == vnum)
            break;
    }

    if (!home)
    {
        ch_printf(ch, "No home found for vnum %d.\n\r", vnum);
        return;
    }

    room = get_room_index(home->home_vnum);
    mob = get_mob_index(home->mob_vnum);

    send_to_char("\n\r&Y=== Home Information ===&w\n\r\n\r", ch);

    ch_printf(ch, "&GRoom Vnum:&w      %d\n\r", home->home_vnum);

    if (room)
    {
        ch_printf(ch, "&GRoom Name:&w      %s\n\r", room->name);
        ch_printf(ch, "&GArea:&w           %s\n\r", room->area ? room->area->name : "Unknown");
    }

    ch_printf(ch, "&GHome Type:&w      %s\n\r", home_type_name(home->home_type));
    ch_printf(ch, "&GHome Name:&w      %s\n\r", home->home_name ? home->home_name : "None");

    if (mob)
    {
        ch_printf(ch, "&GOwner:&w          %s (vnum %d)\n\r",
            mob->short_descr, home->mob_vnum);
    }
    else
    {
        ch_printf(ch, "&GOwner:&w          None\n\r");
    }

    ch_printf(ch, "&GDistrict:&w       %s\n\r",
        home->district ? home->district : "None");

    ch_printf(ch, "&GOwnership:&w      %s\n\r",
        home->owned ? "&GOwned&w" : "&YRenting&w");

    ch_printf(ch, "&GRent Cost:&w      %d gold/week\n\r", home->rent_cost);

    if (home->num_furnishings > 0)
    {
        send_to_char("\n\r&YFurnishings:&w\n\r", ch);
        for (i = 0; i < home->num_furnishings; i++)
        {
            if (home->furnishing_descriptions && home->furnishing_descriptions[i])
            {
                ch_printf(ch, "  - %s\n\r", home->furnishing_descriptions[i]);
            }
        }
    }

    send_to_char("\n\r", ch);
}

/*
 * Command: homeassign <mob_vnum> <room_vnum> - Manually assign home (admin command)
 */
void do_homeassign(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int mob_vnum, room_vnum;
    MOB_INDEX_DATA *pMobIndex;
    ROOM_INDEX_DATA *pRoomIndex;
    MOB_HOME *home;

    if (IS_NPC(ch))
    {
        send_to_char("NPCs cannot use this command.\n\r", ch);
        return;
    }

    if (get_trust(ch) < LEVEL_IMMORTAL)
    {
        send_to_char("Only immortals can assign homes.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Syntax: homeassign <mob_vnum> <room_vnum>\n\r", ch);
        send_to_char("Example: homeassign 3001 10500\n\r", ch);
        return;
    }

    mob_vnum = atoi(arg1);
    room_vnum = atoi(arg2);

    pMobIndex = get_mob_index(mob_vnum);
    if (!pMobIndex)
    {
        ch_printf(ch, "Mob vnum %d does not exist.\n\r", mob_vnum);
        return;
    }

    pRoomIndex = get_room_index(room_vnum);
    if (!pRoomIndex)
    {
        ch_printf(ch, "Room vnum %d does not exist.\n\r", room_vnum);
        return;
    }

    /* Check if home already exists for this room */
    for (home = first_mob_home; home; home = home->next)
    {
        if (home->home_vnum == room_vnum)
            break;
    }

    if (!home)
    {
        /* Create new home */
        home = create_mob_home(mob_vnum, room_vnum, HOME_TYPE_APARTMENT);
        if (!home)
        {
            send_to_char("Failed to create home.\n\r", ch);
            return;
        }
    }
    else
    {
        /* Update existing home's owner */
        home->mob_vnum = mob_vnum;
    }

    /* Mark as owned */
    home->owned = TRUE;

    ch_printf(ch, "&G[SUCCESS]&w Assigned room %d (%s) to mob %d (%s)\n\r",
        room_vnum, pRoomIndex->name,
        mob_vnum, pMobIndex->short_descr);

    /* Save homes */
    save_mob_home(home);

    sprintf(log_buf, "%s assigned home %d to mob %d", ch->name, room_vnum, mob_vnum);
    log_string(log_buf);
}

/*
 * Command: homeunassign <room_vnum> - Free up a home (admin command)
 */
void do_homeunassign(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    int room_vnum;
    MOB_HOME *home;

    if (IS_NPC(ch))
    {
        send_to_char("NPCs cannot use this command.\n\r", ch);
        return;
    }

    if (get_trust(ch) < LEVEL_IMMORTAL)
    {
        send_to_char("Only immortals can unassign homes.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: homeunassign <room_vnum>\n\r", ch);
        return;
    }

    room_vnum = atoi(arg);

    /* Find home by room vnum */
    for (home = first_mob_home; home; home = home->next)
    {
        if (home->home_vnum == room_vnum)
            break;
    }

    if (!home)
    {
        ch_printf(ch, "Room %d is not registered as a home.\n\r", room_vnum);
        return;
    }

    ch_printf(ch, "Unassigned home %d (was owned by mob %d)\n\r",
        room_vnum, home->mob_vnum);

    /* Clear owner */
    home->mob_vnum = 0;
    home->owned = FALSE;

    /* Save */
    save_mob_home(home);

    sprintf(log_buf, "%s unassigned home %d", ch->name, room_vnum);
    log_string(log_buf);
}
