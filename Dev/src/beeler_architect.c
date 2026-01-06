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
#include "world_context.h"
#include "leader_ai.h"
#include "periodicos.h"
#include "organic_creation.h"

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

    /* Count rooms - iterate through all areas and their vnum ranges */
    for (area = first_area; area; area = area->next)
    {
        int vnum;
        for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
        {
            if (get_room_index(vnum))
                room_count++;
        }
    }

    beeler_world_context.total_mobs = mob_count;
    beeler_world_context.total_areas = area_count;
    beeler_world_context.total_rooms = room_count;
    beeler_world_context.last_cache_update = time(NULL);

    {


        char log_buf[256];


        {
            char log_buf[256];
        sprintf(log_buf, "  [Architect] World: %d areas, %d rooms, %d mobs",
        area_count, room_count, mob_count);


        log_string(log_buf);
        }


    }
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

        {


            char log_buf[256];


            {
                char log_buf[256];
            sprintf(log_buf, "  [Architect] %s: vnums %d-%d (%d slots)",
            area->name,
            area->low_r_vnum,
            area->hi_r_vnum,
            available_rooms);


            log_string(log_buf);
            }


        }
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
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] ERROR: Area '%s' not found", area_name);

            log_string(log_buf);
            }

        }
        return NULL;
    }

    /* Find available vnums */
    start_vnum = beeler_find_available_vnums(area->low_r_vnum, num_homes + 5);
    if (start_vnum == 0)
    {
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] ERROR: Not enough space in %s for %d homes",
            area_name, num_homes);

            log_string(log_buf);
            }

        }
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

    {


        char log_buf[256];


        {
            char log_buf[256];
        sprintf(log_buf, "[BEELER] Generated district '%s' with %d homes (vnums %d-%d)",
        district->district_name, num_homes, start_vnum, start_vnum + num_homes);


        log_string(log_buf);
        }


    }

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
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] WARNING: Could not determine city for %s", mob->name);

            log_string(log_buf);
            }

        }
        return 0;
    }

    /* Determine home type */
    home_type = determine_appropriate_home_type(mob);

    /* Find or create home in that city */
    home_vnum = beeler_find_or_create_home(mob, city_name, home_type);

    if (home_vnum > 0)
    {
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] Assigned %s a %s in %s (vnum %d)",
            mob->name,
            home_type_name(home_type),
            city_name,
            home_vnum);

            log_string(log_buf);
            }

        }
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
    {

        char log_buf[256];

        {
            char log_buf[256];
        sprintf(log_buf, "[BEELER] TODO: Need to build new %s district in %s",
        home_type_name(home_type), city_name);

        log_string(log_buf);
        }

    }

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

    {


        char log_buf[256];


        {
            char log_buf[256];
        sprintf(log_buf, "[BEELER] Executing immortal command: %s %s", command, arguments);


        log_string(log_buf);
        }


    }

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
 * Build district - creates rooms in area file
 */
bool beeler_build_district(DISTRICT_GENERATION *district)
{
    AREA_DATA *area;
    char area_filename[512];
    int i;
    int rooms_created = 0;

    if (!district)
    {
        log_string("[BEELER] ERROR: beeler_build_district called with NULL district");
        return FALSE;
    }

    /* Find area */
    for (area = first_area; area; area = area->next)
    {
        if (area->low_r_vnum <= district->area_vnum && area->hi_r_vnum >= district->area_vnum)
            break;
    }

    if (!area || !area->filename)
    {
        log_string("[BEELER] ERROR: Could not find area for district");
        return FALSE;
    }

    /* Build full path to area file */
    sprintf(area_filename, "../area/%s", area->filename);

    {


        char log_buf[256];


        {
            char log_buf[256];
        sprintf(log_buf, "[BEELER] Building district '%s' with %d rooms in %s",
        district->district_name, district->num_rooms, area_filename);


        log_string(log_buf);
        }


    }

    /* Insert each room into the area file */
    for (i = 0; i < district->num_rooms; i++)
    {
        if (!district->rooms[i])
        {
            {

                char log_buf[256];

                {
                    char log_buf[256];
                sprintf(log_buf, "[BEELER] WARNING: NULL room at index %d", i);

                log_string(log_buf);
                }

            }
            continue;
        }

        if (beeler_insert_room_in_area_file(area_filename, district->rooms[i]))
        {
            rooms_created++;
        }
        else
        {
            {

                char log_buf[256];

                {
                    char log_buf[256];
                sprintf(log_buf, "[BEELER] ERROR: Failed to insert room #%d", district->rooms[i]->vnum);

                log_string(log_buf);
                }

            }
        }
    }

    {


        char log_buf[256];


        {
            char log_buf[256];
        sprintf(log_buf, "[BEELER] District build complete: %d/%d rooms created",
        rooms_created, district->num_rooms);


        log_string(log_buf);
        }


    }

    return (rooms_created > 0);
}

/*
 * Commands
 */
void do_beeler_build(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    DISTRICT_GENERATION *district;
    int home_type = HOME_TYPE_APARTMENT;
    int num_homes = 5;

    if (ch->level < LEVEL_IMMORTAL)
    {
        send_to_char("Only immortals can command Beeler to build.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);

    if (arg1[0] == '\0')
    {
        send_to_char("&Y=== Beeler Build Commands ===&w\n\r\n\r", ch);
        send_to_char("&Gbeeler_build district <area> <type> <count>&w\n\r", ch);
        send_to_char("  Types: hovel, apartment, house, manor, palace\n\r", ch);
        send_to_char("  Example: beeler_build district Darkhaven apartment 10\n\r\n\r", ch);
        send_to_char("&Gbeeler_build room <area> <vnum> <type>&w\n\r", ch);
        send_to_char("  Build a single room in an area\n\r", ch);
        send_to_char("  Example: beeler_build room Darkhaven 10500 house\n\r\n\r", ch);
        return;
    }

    if (!str_cmp(arg1, "district"))
    {
        /* beeler_build district Darkhaven apartment 10 */
        if (arg2[0] == '\0' || arg3[0] == '\0')
        {
            send_to_char("Syntax: beeler_build district <area> <type> <count>\n\r", ch);
            send_to_char("Types: hovel, apartment, house, manor, palace\n\r", ch);
            return;
        }

        /* Parse home type */
        if (!str_cmp(arg3, "hovel"))
            home_type = HOME_TYPE_HOVEL;
        else if (!str_cmp(arg3, "apartment"))
            home_type = HOME_TYPE_APARTMENT;
        else if (!str_cmp(arg3, "house"))
            home_type = HOME_TYPE_HOUSE;
        else if (!str_cmp(arg3, "manor"))
            home_type = HOME_TYPE_MANOR;
        else if (!str_cmp(arg3, "palace"))
            home_type = HOME_TYPE_PALACE;
        else
        {
            send_to_char("Invalid home type. Use: hovel, apartment, house, manor, palace\n\r", ch);
            return;
        }

        /* Parse count */
        if (arg4[0] != '\0')
            num_homes = atoi(arg4);

        if (num_homes < 1 || num_homes > 50)
        {
            send_to_char("Count must be between 1 and 50.\n\r", ch);
            return;
        }

        send_to_char("\n\r&Y[Beeler's eyes glow with creative power...]&w\n\r", ch);
        send_to_char("&CReality bends to his will...&w\n\r\n\r", ch);

        /* Generate district */
        district = beeler_generate_residential_district(arg2, home_type, num_homes);
        if (!district)
        {
            send_to_char("&RBeeler shakes his head. The area cannot support this construction.&w\n\r", ch);
            return;
        }

        /* Build district (write to area file) */
        if (beeler_build_district(district))
        {
            ch_printf(ch, "\n\r&G[SUCCESS]&w Beeler has built the %s!\n\r", district->district_name);
            ch_printf(ch, "&YRooms created: %d (vnums %d-%d)&w\n\r",
                district->num_rooms,
                district->starting_vnum,
                district->starting_vnum + district->num_rooms - 1);
            ch_printf(ch, "\n\r&WUse '&Cgoto %d&W' to visit the district.&w\n\r", district->starting_vnum);
            ch_printf(ch, "&RRemember to COPYOVER to load the new rooms into memory!&w\n\r\n\r", ch);
        }
        else
        {
            send_to_char("&RBeeler's power falters. The construction has failed.&w\n\r", ch);
        }

        /* TODO: Free district memory */
    }
    else if (!str_cmp(arg1, "room"))
    {
        /* beeler_build room Darkhaven 10500 house */
        AREA_DATA *area;
        ROOM_GENERATION *room;
        char area_filename[512];
        int vnum;

        if (arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0')
        {
            send_to_char("Syntax: beeler_build room <area> <vnum> <type>\n\r", ch);
            return;
        }

        /* Find area */
        area = beeler_find_area_by_name(arg2);
        if (!area)
        {
            ch_printf(ch, "Area '%s' not found.\n\r", arg2);
            return;
        }

        vnum = atoi(arg3);
        if (vnum < area->low_r_vnum || vnum > area->hi_r_vnum)
        {
            ch_printf(ch, "Vnum %d is outside area range (%d-%d).\n\r",
                vnum, area->low_r_vnum, area->hi_r_vnum);
            return;
        }

        /* Parse home type */
        if (!str_cmp(arg4, "hovel"))
            home_type = HOME_TYPE_HOVEL;
        else if (!str_cmp(arg4, "apartment"))
            home_type = HOME_TYPE_APARTMENT;
        else if (!str_cmp(arg4, "house"))
            home_type = HOME_TYPE_HOUSE;
        else if (!str_cmp(arg4, "manor"))
            home_type = HOME_TYPE_MANOR;
        else if (!str_cmp(arg4, "palace"))
            home_type = HOME_TYPE_PALACE;
        else
        {
            send_to_char("Invalid home type.\n\r", ch);
            return;
        }

        /* Generate single room */
        room = beeler_generate_room(vnum, home_type, NULL);
        if (!room)
        {
            send_to_char("Failed to generate room.\n\r", ch);
            return;
        }

        /* Build path to area file */
        sprintf(area_filename, "../area/%s", area->filename);

        send_to_char("&YBeeler weaves reality...&w\n\r", ch);

        /* Insert room */
        if (beeler_insert_room_in_area_file(area_filename, room))
        {
            ch_printf(ch, "&G[SUCCESS]&w Room #%d created in %s\n\r", vnum, area->name);
            ch_printf(ch, "&RRemember to COPYOVER to load the new room!&w\n\r", ch);
        }
        else
        {
            send_to_char("&R[FAILED]&w Could not create room.\n\r", ch);
        }

        /* TODO: Free room memory */
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

        {
            char log_buf[256];
            sprintf(log_buf, "Total Areas: %d\n\r", beeler_world_context.total_areas);
            send_to_char(log_buf, ch);
        }

        {
            char log_buf[256];
            sprintf(log_buf, "Total Rooms: %d\n\r", beeler_world_context.total_rooms);
            send_to_char(log_buf, ch);
        }

        {
            char log_buf[256];
            sprintf(log_buf, "Total Mobs: %d\n\r", beeler_world_context.total_mobs);
            send_to_char(log_buf, ch);
        }

        {
            char log_buf[256];
            sprintf(log_buf, "\nLast Updated: %s\n\r", ctime(&beeler_world_context.last_cache_update));
            send_to_char(log_buf, ch);
        }
    }
}

/*
 * Command: beeler populate <area> - Generate identities for all mobs in area
 */
void do_beeler_populate(CHAR_DATA *ch, char *argument)
{
    AREA_DATA *area;
    MOB_INDEX_DATA *pMobIndex;
    char arg[MAX_INPUT_LENGTH];
    int vnum, count = 0, skipped = 0;
    extern void generate_mob_identity(CHAR_DATA *mob);
    extern MOB_IDENTITY *get_mob_identity(int vnum);

    if (IS_NPC(ch))
    {
        send_to_char("NPCs cannot use this command.\n\r", ch);
        return;
    }

    if (get_trust(ch) < LEVEL_IMMORTAL)
    {
        send_to_char("Only immortals can use Beeler's bulk operations.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: beeler populate <area name or filename>\n\r", ch);
        send_to_char("\n\rExample: beeler populate Darkhaven\n\r", ch);
        send_to_char("         beeler populate midgaard.are\n\r", ch);
        return;
    }

    /* Find area by name or filename */
    area = NULL;
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

    ch_printf(ch, "\n\r&C[BEELER]&w Awakening consciousness in area: &Y%s&w\n\r", area->name);
    ch_printf(ch, "&CMob range: %d to %d&w\n\r\n\r", area->low_m_vnum, area->hi_m_vnum);

    /* Iterate through all mob vnums in this area */
    for (vnum = area->low_m_vnum; vnum <= area->hi_m_vnum; vnum++)
    {
        pMobIndex = get_mob_index(vnum);

        if (!pMobIndex)
            continue;

        /* Skip if already has identity */
        if (get_mob_identity(vnum))
        {
            skipped++;
            continue;
        }

        /* Create a temporary mob instance for identity generation */
        CHAR_DATA *temp_mob = create_mobile(pMobIndex);

        if (!temp_mob)
            continue;

        /* Generate identity */
        char_to_room(temp_mob, ch->in_room);  /* Temporarily place in room */
        generate_mob_identity(temp_mob);
        extract_char(temp_mob, TRUE);  /* Remove temp mob */

        count++;

        /* Progress indicator every 10 mobs */
        if (count % 10 == 0)
            ch_printf(ch, "&G.&w");
    }

    /* Final summary */
    ch_printf(ch, "\n\r\n\r&Y[BEELER]&w Awakening complete!\n\r");
    ch_printf(ch, "&GIdentities generated: %d&w\n\r", count);
    ch_printf(ch, "&YAlready had identity: %d&w\n\r", skipped);
    ch_printf(ch, "&CTotal mobs processed: %d&w\n\r\n\r", count + skipped);

    {
        char log_buf[256];
        sprintf(log_buf, "[BEELER] %s populated area %s with %d identities",
                ch->name, area->name, count);
        log_string(log_buf);
    }
}

/* Stub implementation - TODO: Implement district home allocation */
int find_available_home_in_district(DISTRICT_DATA *district)
{
    /* TODO: Search district for available home vnum */
    return 0;  /* Return 0 for now (no home found) */
}

/*
 * Backup area file before modification
 */
bool beeler_backup_area_file(char *filename)
{
    char backup_name[512];
    char command[1024];
    FILE *fp_test;

    if (!filename || filename[0] == '\0')
    {
        log_string("[BEELER] ERROR: beeler_backup_area_file called with NULL filename");
        return FALSE;
    }

    /* Check if file exists */
    fp_test = fopen(filename, "r");
    if (!fp_test)
    {
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] WARNING: Cannot backup %s - file doesn't exist", filename);

            log_string(log_buf);
            }

        }
        return FALSE;
    }
    fclose(fp_test);

    /* Create backup filename with timestamp */
    sprintf(backup_name, "%s.bak", filename);

    /* Use system cp command to preserve file */
    sprintf(command, "cp -f \"%s\" \"%s\"", filename, backup_name);

    if (system(command) != 0)
    {
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] ERROR: Failed to backup %s to %s", filename, backup_name);

            log_string(log_buf);
            }

        }
        return FALSE;
    }

    {


        char log_buf[256];


        {
            char log_buf[256];
        sprintf(log_buf, "[BEELER] Backed up %s to %s", filename, backup_name);


        log_string(log_buf);
        }


    }

    return TRUE;
}

/*
 * Read entire area file into memory
 */
bool beeler_read_area_file(char *filename, char **content)
{
    FILE *fp;
    long file_size;
    char *buffer;
    size_t bytes_read;

    if (!filename || !content)
    {
        log_string("[BEELER] ERROR: beeler_read_area_file called with NULL parameter");
        return FALSE;
    }

    fp = fopen(filename, "r");
    if (!fp)
    {
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] ERROR: Cannot open %s for reading", filename);

            log_string(log_buf);
            }

        }
        return FALSE;
    }

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size <= 0 || file_size > 10000000)  /* 10MB max */
    {
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] ERROR: Invalid file size for %s: %ld bytes", filename, file_size);

            log_string(log_buf);
            }

        }
        fclose(fp);
        return FALSE;
    }

    /* Allocate buffer */
    buffer = (char *)malloc(file_size + 1);
    if (!buffer)
    {
        log_string("[BEELER] ERROR: Out of memory reading area file");
        fclose(fp);
        return FALSE;
    }

    /* Read entire file */
    bytes_read = fread(buffer, 1, file_size, fp);
    buffer[bytes_read] = '\0';

    fclose(fp);

    if (bytes_read != file_size)
    {
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] WARNING: Read %zu bytes, expected %ld", bytes_read, file_size);

            log_string(log_buf);
            }

        }
    }

    *content = buffer;

    {


        char log_buf[256];


        {
            char log_buf[256];
        sprintf(log_buf, "[BEELER] Read %zu bytes from %s", bytes_read, filename);


        log_string(log_buf);
        }


    }

    return TRUE;
}

/*
 * Write content to area file
 */
bool beeler_write_area_file(char *filename, char *content)
{
    FILE *fp;
    size_t content_len;
    size_t bytes_written;

    if (!filename || !content)
    {
        log_string("[BEELER] ERROR: beeler_write_area_file called with NULL parameter");
        return FALSE;
    }

    /* Backup first */
    if (!beeler_backup_area_file(filename))
    {
        log_string("[BEELER] WARNING: Proceeding without backup (file may not exist yet)");
    }

    fp = fopen(filename, "w");
    if (!fp)
    {
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] ERROR: Cannot open %s for writing", filename);

            log_string(log_buf);
            }

        }
        return FALSE;
    }

    content_len = strlen(content);
    bytes_written = fwrite(content, 1, content_len, fp);

    fclose(fp);

    if (bytes_written != content_len)
    {
        {

            char log_buf[256];

            {
                char log_buf[256];
            sprintf(log_buf, "[BEELER] ERROR: Wrote %zu bytes, expected %zu", bytes_written, content_len);

            log_string(log_buf);
            }

        }
        return FALSE;
    }

    {


        char log_buf[256];


        {
            char log_buf[256];
        sprintf(log_buf, "[BEELER] Wrote %zu bytes to %s", bytes_written, filename);


        log_string(log_buf);
        }


    }

    return TRUE;
}

/*
 * Insert room into area file
 * This parses the area file, finds the #ROOMS section, and inserts the room before #0
 */
bool beeler_insert_room_in_area_file(char *area_file, ROOM_GENERATION *room)
{
    char *file_content = NULL;
    char *new_content = NULL;
    char *rooms_section_start;
    char *rooms_section_end;
    char room_data[MAX_STRING_LENGTH * 2];
    size_t new_size;
    int i;

    if (!area_file || !room)
    {
        log_string("[BEELER] ERROR: beeler_insert_room_in_area_file called with NULL parameter");
        return FALSE;
    }

    /* Read the area file */
    if (!beeler_read_area_file(area_file, &file_content))
    {
        return FALSE;
    }

    /* Find #ROOMS section */
    rooms_section_start = strstr(file_content, "#ROOMS");
    if (!rooms_section_start)
    {
        log_string("[BEELER] ERROR: No #ROOMS section found in area file");
        free(file_content);
        return FALSE;
    }

    /* Find the #0 that ends the ROOMS section */
    /* It's the first #0 after #ROOMS */
    rooms_section_end = strstr(rooms_section_start, "\n#0\n");
    if (!rooms_section_end)
    {
        /* Try without surrounding newlines */
        rooms_section_end = strstr(rooms_section_start, "#0");
        if (!rooms_section_end)
        {
            log_string("[BEELER] ERROR: No #0 terminator found in #ROOMS section");
            free(file_content);
            return FALSE;
        }
    }

    /* Generate room data in area file format */
    sprintf(room_data, "#%d\n%s~\n%s~\n0 %d %d\n",
        room->vnum,
        room->name ? room->name : "Unnamed Room",
        room->description ? room->description : "",
        room->room_flags,
        room->sector_type);

    /* Add exits */
    for (i = 0; i < room->num_exits; i++)
    {
        char exit_data[512];
        sprintf(exit_data, "D%d\n~\n~\n0 -1 %d\n",
            room->exit_dirs[i],
            room->exit_to_vnums[i]);
        strcat(room_data, exit_data);
    }

    /* Add extra descriptions */
    for (i = 0; i < room->num_extras; i++)
    {
        char extra_data[1024];
        sprintf(extra_data, "E\n%s~\n%s~\n",
            room->extra_keywords[i] ? room->extra_keywords[i] : "",
            room->extra_descriptions[i] ? room->extra_descriptions[i] : "");
        strcat(room_data, extra_data);
    }

    /* Add room terminator */
    strcat(room_data, "S\n");

    /* Calculate new content size */
    new_size = strlen(file_content) + strlen(room_data) + 100;
    new_content = (char *)malloc(new_size);
    if (!new_content)
    {
        log_string("[BEELER] ERROR: Out of memory creating new area file content");
        free(file_content);
        return FALSE;
    }

    /* Build new content: everything before #0 + room data + #0 + everything after */
    size_t prefix_len = rooms_section_end - file_content;
    strncpy(new_content, file_content, prefix_len);
    new_content[prefix_len] = '\0';

    strcat(new_content, room_data);
    strcat(new_content, rooms_section_end);

    /* Write modified content */
    if (!beeler_write_area_file(area_file, new_content))
    {
        free(file_content);
        free(new_content);
        return FALSE;
    }

    free(file_content);
    free(new_content);

    {


        char log_buf[256];


        {
            char log_buf[256];
        sprintf(log_buf, "[BEELER] Successfully inserted room #%d into %s", room->vnum, area_file);


        log_string(log_buf);
        }


    }

    return TRUE;
}
