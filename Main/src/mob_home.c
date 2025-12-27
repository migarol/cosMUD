/***************************************************************************
 * MOB Home System
 *
 * Handles mob housing - where they live, rent, and return to
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"
#include "mob_ai.h"

/***************************************************************************
 * Create a new mob home structure
 ***************************************************************************/
MOB_HOME_DATA *create_mob_home(void)
{
    MOB_HOME_DATA *home;

    CREATE(home, MOB_HOME_DATA, 1);

    home->home_vnum = -1;
    home->home_type = HOME_NONE;
    home->home_name = NULL;
    home->rent_cost = 0;
    home->rent_paid_until = 0;
    home->times_visited = 0;
    home->last_visit = 0;

    return home;
}

/***************************************************************************
 * Free a mob home structure
 ***************************************************************************/
void free_mob_home(MOB_HOME_DATA *home)
{
    if (!home)
        return;

    if (home->home_name)
        STRFREE(home->home_name);

    DISPOSE(home);
}

/***************************************************************************
 * Get home type name
 ***************************************************************************/
const char *get_home_type_name(sh_int home_type)
{
    switch(home_type)
    {
        case HOME_NONE:      return "None";
        case HOME_HOUSE:     return "House";
        case HOME_APARTMENT: return "Apartment";
        case HOME_INN_ROOM:  return "Inn Room";
        case HOME_SHOP:      return "Shop";
        case HOME_GUILD:     return "Guild";
        case HOME_TEMPLE:    return "Temple";
        case HOME_CAVE:      return "Cave";
        default:             return "Unknown";
    }
}

/***************************************************************************
 * Parse home type from string
 ***************************************************************************/
sh_int parse_home_type(const char *name)
{
    if (!str_cmp(name, "house"))      return HOME_HOUSE;
    if (!str_cmp(name, "apartment"))  return HOME_APARTMENT;
    if (!str_cmp(name, "inn_room"))   return HOME_INN_ROOM;
    if (!str_cmp(name, "shop"))       return HOME_SHOP;
    if (!str_cmp(name, "guild"))      return HOME_GUILD;
    if (!str_cmp(name, "temple"))     return HOME_TEMPLE;
    if (!str_cmp(name, "cave"))       return HOME_CAVE;

    return HOME_NONE;
}

/***************************************************************************
 * Save mob home to JSON file
 ***************************************************************************/
void save_mob_home(MOB_INDEX_DATA *pMob)
{
    FILE *fp;
    char filename[256];
    MOB_HOME_DATA *home;

    if (!pMob || !pMob->ai_home)
        return;

    home = pMob->ai_home;

    sprintf(filename, "../ai_data/homes/%d.json", pMob->vnum);

    if (!(fp = fopen(filename, "w")))
    {
        bug("save_mob_home: cannot open %s", filename);
        return;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"mob_vnum\": %d,\n", pMob->vnum);
    fprintf(fp, "  \"home_vnum\": %d,\n", home->home_vnum);
    fprintf(fp, "  \"home_type\": %d,\n", home->home_type);

    if (home->home_name)
        fprintf(fp, "  \"home_name\": \"%s\",\n", home->home_name);

    fprintf(fp, "  \"rent_cost\": %d,\n", home->rent_cost);
    fprintf(fp, "  \"rent_paid_until\": %ld,\n", (long)home->rent_paid_until);
    fprintf(fp, "  \"times_visited\": %d,\n", home->times_visited);
    fprintf(fp, "  \"last_visit\": %ld\n", (long)home->last_visit);
    fprintf(fp, "}\n");

    fclose(fp);
}

/***************************************************************************
 * Load mob home from JSON file
 ***************************************************************************/
void load_mob_home(MOB_INDEX_DATA *pMob)
{
    FILE *fp;
    char filename[256];
    char line[1024];
    MOB_HOME_DATA *home;

    sprintf(filename, "../ai_data/homes/%d.json", pMob->vnum);

    if (!(fp = fopen(filename, "r")))
        return; /* No home file, that's okay */

    home = create_mob_home();
    pMob->ai_home = home;

    /* Simple line-by-line parser */
    while (fgets(line, sizeof(line), fp))
    {
        char *key, *value;
        char *p;

        if (!strchr(line, ':'))
            continue;

        key = line;
        while (*key == ' ' || *key == '\t' || *key == '"')
            key++;

        value = strchr(line, ':');
        if (!value)
            continue;

        *value++ = '\0';
        while (*value == ' ' || *value == '\t' || *value == '"')
            value++;

        p = value + strlen(value) - 1;
        while (p > value && (*p == '\n' || *p == '\r' || *p == '"' || *p == ',' || *p == ' '))
            *p-- = '\0';

        if (!str_cmp(key, "home_vnum"))
            home->home_vnum = atoi(value);
        else if (!str_cmp(key, "home_type"))
            home->home_type = atoi(value);
        else if (!str_cmp(key, "home_name"))
            home->home_name = STRALLOC(value);
        else if (!str_cmp(key, "rent_cost"))
            home->rent_cost = atoi(value);
        else if (!str_cmp(key, "rent_paid_until"))
            home->rent_paid_until = atol(value);
        else if (!str_cmp(key, "times_visited"))
            home->times_visited = atoi(value);
        else if (!str_cmp(key, "last_visit"))
            home->last_visit = atol(value);
    }

    fclose(fp);
}

/***************************************************************************
 * Find an available home in an area
 ***************************************************************************/
ROOM_INDEX_DATA *find_available_home(AREA_DATA *area, sh_int home_type)
{
    ROOM_INDEX_DATA *room;
    int vnum;

    if (!area)
        return NULL;

    /* Scan rooms in the area looking for suitable homes */
    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);
        if (!room)
            continue;

        /* Check if room fits the home type criteria */
        /* This is simplified - in full version would check room flags, */
        /* sector type, etc. */

        /* For now, just check if it's not already someone's home */
        /* and if the name suggests it could be a home */

        if (home_type == HOME_HOUSE || home_type == HOME_APARTMENT)
        {
            if (strstr(room->name, "House") ||
                strstr(room->name, "house") ||
                strstr(room->name, "Home") ||
                strstr(room->name, "home") ||
                strstr(room->name, "Apartment") ||
                strstr(room->name, "apartment"))
            {
                /* TODO: Check if already assigned */
                return room;
            }
        }
        else if (home_type == HOME_SHOP)
        {
            if (strstr(room->name, "Shop") || strstr(room->name, "shop") ||
                strstr(room->name, "Store") || strstr(room->name, "store"))
            {
                return room;
            }
        }
    }

    return NULL;
}

/***************************************************************************
 * Assign a home to a mob
 ***************************************************************************/
bool assign_home_to_mob(MOB_INDEX_DATA *pMob, int home_vnum, sh_int home_type)
{
    ROOM_INDEX_DATA *room;
    MOB_HOME_DATA *home;

    room = get_room_index(home_vnum);
    if (!room)
    {
        bug("assign_home_to_mob: invalid vnum %d", home_vnum);
        return FALSE;
    }

    /* Create home if doesn't exist */
    if (!pMob->ai_home)
    {
        pMob->ai_home = create_mob_home();
    }

    home = pMob->ai_home;
    home->home_vnum = home_vnum;
    home->home_type = home_type;

    if (home->home_name)
        STRFREE(home->home_name);

    home->home_name = STRALLOC(room->name);

    save_mob_home(pMob);

    return TRUE;
}

/***************************************************************************
 * Display mob home info
 ***************************************************************************/
void show_mob_home(CHAR_DATA *ch, MOB_INDEX_DATA *pMob)
{
    MOB_HOME_DATA *home;
    ROOM_INDEX_DATA *room;
    char buf[MAX_STRING_LENGTH];
    char time_buf[80];

    if (!pMob->ai_home)
    {
        send_to_char("This mob has no home.\n\r", ch);
        return;
    }

    home = pMob->ai_home;

    send_to_char("&W=== MOB HOME ===&D\n\r\n\r", ch);

    room = get_room_index(home->home_vnum);

    sprintf(buf, "&CHome Name:&w %s\n\r",
            home->home_name ? home->home_name : "&R(not set)&w");
    send_to_char(buf, ch);

    sprintf(buf, "&CHome Vnum:&w %d %s\n\r",
            home->home_vnum,
            room ? "" : "&R(INVALID!)&w");
    send_to_char(buf, ch);

    sprintf(buf, "&CHome Type:&w %s\n\r",
            get_home_type_name(home->home_type));
    send_to_char(buf, ch);

    if (home->rent_cost > 0)
    {
        sprintf(buf, "&CRent Cost:&w %d gold/week\n\r", home->rent_cost);
        send_to_char(buf, ch);

        if (home->rent_paid_until > 0)
        {
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S",
                    localtime(&home->rent_paid_until));
            sprintf(buf, "&CRent Paid Until:&w %s\n\r", time_buf);
            send_to_char(buf, ch);
        }
    }

    sprintf(buf, "&CTimes Visited:&w %d\n\r", home->times_visited);
    send_to_char(buf, ch);

    if (home->last_visit > 0)
    {
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S",
                localtime(&home->last_visit));
        sprintf(buf, "&CLast Visit:&w %s\n\r", time_buf);
        send_to_char(buf, ch);
    }

    send_to_char("\n\r", ch);
}
