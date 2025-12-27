/*****************************************************************************
 * Beeler - World Architect - Implementation
 *
 * Beeler builds the world, room by room, with divine precision.
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include "mud.h"
#include "beeler.h"
#include "beeler_architect.h"
#include "mob_home.h"
#include "mob_identity.h"

/* Global context */
BEELER_CONTEXT beeler_world_context;
BEELER_AREA_MOD *first_area_modification = NULL;

/*
 * Initialize architect system
 */
void init_beeler_architect(void)
{
    log_string("  [Architect] Beeler scans the world...");

    /* Build context cache */
    beeler_build_context_cache();

    /* Scan all areas for available space */
    beeler_scan_all_areas();

    log_string("  [Architect] World analysis complete");
}

/*
 * Build context cache for fast lookups
 */
void beeler_build_context_cache(void)
{
    CHAR_DATA *mob;
    AREA_DATA *area;
    ROOM_INDEX_DATA *room;
    int mob_count = 0;
    int area_count = 0;
    int room_count = 0;

    /* Count mobs */
    for (mob = first_char; mob; mob = mob->next)
    {
        if (IS_NPC(mob))
            mob_count++;
    }

    /* Count areas */
    for (area = first_area; area; area = area->next)
        area_count++;

    /* Count rooms */
    for (room = first_room; room; room = room->next)
        room_count++;

    beeler_world_context.total_mobs = mob_count;
    beeler_world_context.total_areas = area_count;
    beeler_world_context.total_rooms = room_count;
    beeler_world_context.last_cache_update = time(NULL);

    sprintf(log_buf, "  [Architect] World: %d areas, %d rooms, %d mobs",
        area_count, room_count, mob_count);
    log_string(log_buf);
}

/*
 * Scan all areas for available vnum ranges
 */
void beeler_scan_all_areas(void)
{
    AREA_DATA *area;
    int available_rooms;

    for (area = first_area; area; area = area->next)
    {
        available_rooms = (area->hi_r_vnum - area->low_r_vnum + 1);

        sprintf(log_buf, "  [Architect] %s: vnums %d-%d (%d slots)",
            area->name,
            area->low_r_vnum,
            area->hi_r_vnum,
            available_rooms);
        log_string(log_buf);
    }
}

/*
 * Find available vnums in an area
 */
int beeler_find_available_vnums(int area_vnum, int count)
{
    AREA_DATA *area;
    ROOM_INDEX_DATA *room;
    int vnum;
    int found_sequential = 0;
    int start_vnum = 0;

    /* Find area */
    for (area = first_area; area; area = area->next)
    {
        if (area->low_r_vnum <= area_vnum && area->hi_r_vnum >= area_vnum)
            break;
    }

    if (!area)
        return 0;

    /* Find sequential available vnums */
    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);

        if (!room)
        {
            /* This vnum is available */
            if (found_sequential == 0)
                start_vnum = vnum;

            found_sequential++;

            if (found_sequential >= count)
                return start_vnum;  /* Found enough sequential vnums */
        }
        else
        {
            /* Reset counter */
            found_sequential = 0;
            start_vnum = 0;
        }
    }

    return 0;  /* Not enough space */
}

/*
 * Check if we can add rooms to an area
 */
bool beeler_can_add_rooms_to_area(int area_vnum, int count)
{
    return (beeler_find_available_vnums(area_vnum, count) > 0);
}

/*
 * Generate a single room
 */
ROOM_GENERATION *beeler_generate_room(int vnum, int home_type, char *mob_name)
{
    ROOM_GENERATION *room;
    extern char *home_type_name(int home_type);

    CREATE(room, ROOM_GENERATION, 1);

    room->vnum = vnum;
    room->sector_type = SECT_INSIDE;
    room->room_flags = 0;  /* No special flags for homes */
    room->ai_generated = TRUE;

    /* Generate name */
    if (mob_name && mob_name[0] != '\0')
    {
        static char name[256];
        sprintf(name, "%s's %s", mob_name, home_type_name(home_type));
        room->name = str_dup(name);
    }
    else
    {
        static char name[256];
        sprintf(name, "A %s", home_type_name(home_type));
        room->name = str_dup(name);
    }

    /* Generate description (AI-powered later) */
    room->description = beeler_generate_room_description(home_type, mob_name, NULL);

    room->num_exits = 0;
    room->num_extras = 0;

    return room;
}

/*
 * Generate room description
 */
char *beeler_generate_room_description(int home_type, char *mob_name, char *mob_personality)
{
    static char desc[MAX_STRING_LENGTH];

    /* TODO: Call Ollama for AI-generated descriptions */
    /* For now, template-based */

    switch (home_type)
    {
        case HOME_TYPE_HOVEL:
            sprintf(desc, "This cramped, dingy hovel barely keeps out the elements.\n"
                         "The walls are cracked, the floor is dirt, and the air is stale.\n"
                         "A single threadbare blanket lies in the corner.\n");
            break;

        case HOME_TYPE_APARTMENT:
            sprintf(desc, "This small but functional apartment provides basic shelter.\n"
                         "Simple furnishings line the walls - a bed, a table, a chair.\n"
                         "Clean but modest, it serves its purpose.\n");
            break;

        case HOME_TYPE_HOUSE:
            sprintf(desc, "This comfortable house speaks of middle-class stability.\n"
                         "Well-maintained furnishings fill the space efficiently.\n"
                         "A fireplace crackles warmly in the corner.\n");
            break;

        case HOME_TYPE_MANOR:
            sprintf(desc, "This impressive manor displays wealth and taste.\n"
                         "Fine furnishings, art on the walls, and polished floors.\n"
                         "Every detail speaks of success and status.\n");
            break;

        case HOME_TYPE_PALACE:
            sprintf(desc, "This magnificent palace chamber is fit for royalty.\n"
                         "Opulent decorations, priceless art, and luxurious furnishings.\n"
                         "The very air seems to shimmer with power and prestige.\n");
            break;

        default:
            sprintf(desc, "A room.\n");
            break;
    }

    return str_dup(desc);
}

/*
 * Generate residential district
 */
DISTRICT_GENERATION *beeler_generate_residential_district(char *area_name, int home_type, int num_homes)
{
    DISTRICT_GENERATION *district;
    AREA_DATA *area;
    int start_vnum;
    int i;
    extern char *home_type_name(int home_type);

    /* Find area */
    area = beeler_find_area_by_name(area_name);
    if (!area)
    {
        sprintf(log_buf, "[BEELER] ERROR: Area '%s' not found", area_name);
        log_string(log_buf);
        return NULL;
    }

    /* Find available vnums */
    start_vnum = beeler_find_available_vnums(area->low_r_vnum, num_homes + 5);
    if (start_vnum == 0)
    {
        sprintf(log_buf, "[BEELER] ERROR: Not enough space in %s for %d homes",
            area_name, num_homes);
        log_string(log_buf);
        return NULL;
    }

    /* Create district */
    CREATE(district, DISTRICT_GENERATION, 1);

    static char dist_name[256];
    sprintf(dist_name, "%s District", home_type_name(home_type));
    district->district_name = str_dup(dist_name);
    district->area_vnum = area->low_r_vnum;
    district->starting_vnum = start_vnum;
    district->num_rooms = num_homes + 1;  /* +1 for entrance */
    district->home_type = home_type;
    district->layout_type = str_dup("street");  /* Simple layout */

    /* Allocate room array */
    CREATE(district->rooms, ROOM_GENERATION *, district->num_rooms);

    /* Generate entrance room */
    district->rooms[0] = beeler_generate_room(start_vnum, -1, NULL);
    DISPOSE(district->rooms[0]->name);
    district->rooms[0]->name = str_dup(district->district_name);
    DISPOSE(district->rooms[0]->description);
    static char entrance_desc[512];
    sprintf(entrance_desc,
        "This is the entrance to the %s.\n"
        "Homes line the street to the north and south.\n",
        district->district_name);
    district->rooms[0]->description = str_dup(entrance_desc);

    /* Generate homes */
    for (i = 1; i < district->num_rooms; i++)
    {
        district->rooms[i] = beeler_generate_room(start_vnum + i, home_type, NULL);
    }

    sprintf(log_buf, "[BEELER] Generated district '%s' with %d homes (vnums %d-%d)",
        district->district_name, num_homes, start_vnum, start_vnum + num_homes);
    log_string(log_buf);

    return district;
}

/*
 * Find area by name
 */
AREA_DATA *beeler_find_area_by_name(char *name)
{
    AREA_DATA *area;

    for (area = first_area; area; area = area->next)
    {
        if (!str_cmp(area->name, name))
            return area;

        /* Partial match */
        if (strstr(area->name, name) || strstr(name, area->name))
            return area;
    }

    return NULL;
}

/*
 * Assign home intelligently based on mob location
 */
int beeler_assign_home_intelligently(CHAR_DATA *mob)
{
    MOB_IDENTITY *identity;
    AREA_DATA *area;
    char *city_name;
    int home_type;
    int home_vnum;
    extern MOB_IDENTITY *get_mob_identity(int vnum);
    extern int determine_appropriate_home_type(CHAR_DATA *mob);

    if (!mob || !IS_NPC(mob))
        return 0;

    identity = get_mob_identity(mob->pIndexData->vnum);
    if (!identity)
        return 0;

    /* Determine city based on mob's area */
    city_name = beeler_determine_home_city(mob);
    if (!city_name)
    {
        sprintf(log_buf, "[BEELER] WARNING: Could not determine city for %s", mob->name);
        log_string(log_buf);
        return 0;
    }

    /* Determine home type */
    home_type = determine_appropriate_home_type(mob);

    /* Find or create home in that city */
    home_vnum = beeler_find_or_create_home(mob, city_name, home_type);

    if (home_vnum > 0)
    {
        sprintf(log_buf, "[BEELER] Assigned %s a %s in %s (vnum %d)",
            mob->name,
            home_type_name(home_type),
            city_name,
            home_vnum);
        log_string(log_buf);
    }

    return home_vnum;
}

/*
 * Determine home city for mob
 */
char *beeler_determine_home_city(CHAR_DATA *mob)
{
    AREA_DATA *area;

    if (!mob || !mob->in_room)
        return str_dup("Unknown");

    area = mob->in_room->area;
    if (!area)
        return str_dup("Unknown");

    /* Check area name for known cities */
    if (strstr(area->name, "Darkhaven"))
        return str_dup("Darkhaven");
    else if (strstr(area->name, "Midgaard"))
        return str_dup("Midgaard");
    else if (strstr(area->name, "Ofcol"))
        return str_dup("Ofcol");
    else if (strstr(area->name, "New Thalos"))
        return str_dup("New Thalos");
    else if (strstr(area->name, "Titan"))
        return str_dup("New Titan");

    /* Default to area name */
    return str_dup(area->name);
}

/*
 * Find or create home for mob
 */
int beeler_find_or_create_home(CHAR_DATA *mob, char *city_name, int home_type)
{
    AREA_DATA *area;
    DISTRICT_DATA *district;
    int home_vnum;
    extern DISTRICT_DATA *first_district;
    extern DISTRICT_DATA *get_district_for_room(int room_vnum);
    extern int find_available_home_in_district(DISTRICT_DATA *district);

    /* Find area for this city */
    area = beeler_find_area_by_name(city_name);
    if (!area)
        return 0;

    /* Look for existing district of this type */
    for (district = first_district; district; district = district->next)
    {
        if (district->area_vnum == area->low_r_vnum &&
            district->district_type == home_type)
        {
            /* Found matching district - find available home */
            home_vnum = find_available_home_in_district(district);
            if (home_vnum > 0)
                return home_vnum;
        }
    }

    /* No existing district or all full - need to create new one */
    /* TODO: Trigger Beeler to build new district */
    sprintf(log_buf, "[BEELER] TODO: Need to build new %s district in %s",
        home_type_name(home_type), city_name);
    log_string(log_buf);

    return 0;
}

/*
 * Context quick stat
 */
char *beeler_quick_stat(char *query)
{
    static char result[512];

    /* Update context if stale */
    if (time(NULL) - beeler_world_context.last_cache_update > 300)
        beeler_build_context_cache();

    /* Parse query */
    if (strstr(query, "total mobs") || strstr(query, "how many mobs"))
    {
        sprintf(result, "There are %d mobs in the world.", beeler_world_context.total_mobs);
    }
    else if (strstr(query, "total areas") || strstr(query, "how many areas"))
    {
        sprintf(result, "There are %d areas in the world.", beeler_world_context.total_areas);
    }
    else if (strstr(query, "total rooms") || strstr(query, "how many rooms"))
    {
        sprintf(result, "There are %d rooms in the world.", beeler_world_context.total_rooms);
    }
    else
    {
        sprintf(result, "I don't understand that query.");
    }

    return str_dup(result);
}

/*
 * Execute immortal command as Beeler
 */
void beeler_execute_immortal_command(char *command, char *arguments)
{
    /* TODO: This is POWERFUL - Beeler can execute ANY immortal command */
    /* For safety, log everything and require validation */

    sprintf(log_buf, "[BEELER] Executing immortal command: %s %s", command, arguments);
    log_string(log_buf);

    /* TODO: Actual implementation */
    /* This would call interpret() with a virtual Beeler character at level LEVEL_SUPREME */
}

/*
 * Check if Beeler has permission for command
 */
bool beeler_has_permission_for(char *command)
{
    /* Beeler is supreme - he has permission for everything */
    /* But we log and validate for safety */

    /* Blacklist destructive commands that need extra confirmation */
    if (!str_cmp(command, "shutdown") ||
        !str_cmp(command, "copyover") ||
        !str_cmp(command, "force all"))
    {
        log_string("[BEELER] WARNING: Attempted destructive command - requires manual approval");
        return FALSE;
    }

    return TRUE;
}

/*
 * Commands
 */
void do_beeler_build(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    DISTRICT_GENERATION *district;

    if (ch->level < LEVEL_IMMORTAL)
    {
        send_to_char("Only immortals can command Beeler to build.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char("Beeler build commands:\n\r", ch);
        send_to_char("  beeler_build district <area> <type> <count>\n\r", ch);
        send_to_char("  beeler_build inn <area> <name>\n\r", ch);
        send_to_char("  beeler_build home <mob_name>\n\r", ch);
        return;
    }

    if (!str_cmp(arg1, "district"))
    {
        /* beeler_build district Darkhaven apartment 10 */
        if (arg2[0] == '\0')
        {
            send_to_char("Specify: area, home_type, count\n\r", ch);
            return;
        }

        send_to_char("&Y[Beeler begins to reshape reality...]&w\n\r\n\r", ch);

        /* TODO: Parse arguments and generate district */
        send_to_char("District generation not yet fully implemented.\n\r", ch);
    }
}

void do_beeler_context(CHAR_DATA *ch, char *argument)
{
    if (argument && argument[0] != '\0')
    {
        char *result = beeler_quick_stat(argument);
        send_to_char(result, ch);
        send_to_char("\n\r", ch);
    }
    else
    {
        send_to_char("\n\r&Y=== Beeler's World Context ===&w\n\r\n\r", ch);

        sprintf(log_buf, "Total Areas: %d\n\r", beeler_world_context.total_areas);
        send_to_char(log_buf, ch);

        sprintf(log_buf, "Total Rooms: %d\n\r", beeler_world_context.total_rooms);
        send_to_char(log_buf, ch);

        sprintf(log_buf, "Total Mobs: %d\n\r", beeler_world_context.total_mobs);
        send_to_char(log_buf, ch);

        sprintf(log_buf, "\nLast Updated: %s\n\r", ctime(&beeler_world_context.last_cache_update));
        send_to_char(log_buf, ch);
    }
}
