/****************************************************************************
 * System Stubs - Placeholder implementations for disabled systems
 * These stubs allow compilation when certain systems are commented out
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* Persistent Memory stubs */
void init_persistent_memory(void)
{
    log_string("NOTE: Persistent memory system disabled");
}

void load_all_mob_memories(void)
{
    /* No-op */
}

void save_all_mob_memories(void)
{
    /* No-op */
}

/* Player World Impact stubs */
void init_player_world_impact(void)
{
    log_string("NOTE: Player world impact system disabled");
}

/* Resource Distribution stubs */
void init_resource_distribution(void)
{
    log_string("NOTE: Resource distribution system disabled");
}

char **determine_exportable_resources(AREA_DATA *area, int *count)
{
    if (count)
        *count = 0;
    return NULL;
}

/* Family system stubs */
void family_system_update(void)
{
    /* No-op - use family_lineage_update instead */
}

/* Trade system stubs */
void trade_route_update(void)
{
    /* No-op - use global_trade_update instead */
}

/* Geography/location stubs */
int calculate_area_distance(AREA_DATA *from, AREA_DATA *to)
{
    /* Simple stub - return a default distance */
    return 100;
}

AREA_DATA *get_area_by_filename(char *filename)
{
    AREA_DATA *area;

    for (area = first_area; area; area = area->next)
    {
        if (area->filename && !str_cmp(area->filename, filename))
            return area;
    }
    return NULL;
}

ROOM_INDEX_DATA *suggest_best_location_for(char *what)
{
    ROOM_INDEX_DATA *room;

    /* Stub - return first room */
    room = get_room_index(100);
    if (!room)
        room = get_room_index(1);  /* Try room 1 if 100 doesn't exist */

    return room;
}
