/*****************************************************************************
 * Pathfinding System for Mobs
 *
 * Simple BFS pathfinding so mobs know HOW to get to their destinations.
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mud.h"
#include "mob_pathfinding.h"

/* Path cache to avoid recalculating */
#define PATH_CACHE_SIZE 1000
typedef struct cached_path {
    int from_vnum;
    int to_vnum;
    PATH_RESULT *path;
    time_t cached_time;
    struct cached_path *next;
} CACHED_PATH;

CACHED_PATH *first_cached_path = NULL;
int total_cached_paths = 0;

/*
 * Find path from one room to another using BFS
 */
PATH_RESULT *find_path(int from_vnum, int to_vnum, int max_distance)
{
    ROOM_INDEX_DATA *from_room, *to_room, *current_room, *next_room;
    PATH_RESULT *result;
    PATH_NODE *path_node;
    EXIT_DATA *pexit;
    int *visited;
    int *parent_room;
    int *parent_dir;
    int visited_count = 0;
    int queue[1000];
    int queue_start = 0, queue_end = 0;
    int current_vnum, dir;
    bool found = FALSE;

    /* Get rooms */
    from_room = get_room_index(from_vnum);
    to_room = get_room_index(to_vnum);

    if (!from_room || !to_room)
        return NULL;

    /* Already there */
    if (from_vnum == to_vnum)
    {
        CREATE(result, PATH_RESULT, 1);
        result->from_vnum = from_vnum;
        result->to_vnum = to_vnum;
        result->path_length = 0;
        result->path_found = TRUE;
        return result;
    }

    /* Allocate tracking arrays */
    CREATE(visited, int, 100000);
    CREATE(parent_room, int, 100000);
    CREATE(parent_dir, int, 100000);

    /* Initialize */
    memset(visited, 0, sizeof(int) * 100000);
    memset(parent_room, -1, sizeof(int) * 100000);
    memset(parent_dir, -1, sizeof(int) * 100000);

    /* BFS */
    queue[queue_end++] = from_vnum;
    visited[from_vnum] = 1;

    while (queue_start < queue_end && visited_count < max_distance)
    {
        current_vnum = queue[queue_start++];
        current_room = get_room_index(current_vnum);

        if (!current_room)
            continue;

        /* Found it! */
        if (current_vnum == to_vnum)
        {
            found = TRUE;
            break;
        }

        /* Explore neighbors */
        for (dir = 0; dir < 6; dir++)
        {
            pexit = get_exit(current_room, dir);
            if (!pexit || !pexit->to_room)
                continue;

            next_room = pexit->to_room;

            /* Skip if closed door or no-mob */
            if (IS_SET(pexit->exit_info, EX_CLOSED))
                continue;
            if (xIS_SET(next_room->room_flags, ROOM_NO_MOB))
                continue;

            /* Skip if already visited */
            if (visited[next_room->vnum])
                continue;

            /* Mark visited and add to queue */
            visited[next_room->vnum] = 1;
            parent_room[next_room->vnum] = current_vnum;
            parent_dir[next_room->vnum] = dir;
            queue[queue_end++] = next_room->vnum;
            visited_count++;

            if (queue_end >= 1000)
                break;
        }
    }

    /* Build result */
    CREATE(result, PATH_RESULT, 1);
    result->from_vnum = from_vnum;
    result->to_vnum = to_vnum;
    result->path_found = found;
    result->path_length = 0;
    result->first_step = NULL;
    result->last_step = NULL;

    if (found)
    {
        /* Backtrack to build path */
        int path_vnums[MAX_PATH_LENGTH];
        int path_dirs[MAX_PATH_LENGTH];
        int path_len = 0;
        int backtrack = to_vnum;

        while (backtrack != from_vnum && path_len < MAX_PATH_LENGTH)
        {
            path_vnums[path_len] = backtrack;
            path_dirs[path_len] = parent_dir[backtrack];
            backtrack = parent_room[backtrack];
            path_len++;
        }

        /* Reverse path and create nodes */
        result->path_length = path_len;
        for (int i = path_len - 1; i >= 0; i--)
        {
            CREATE(path_node, PATH_NODE, 1);
            path_node->room_vnum = path_vnums[i];
            path_node->direction = path_dirs[i];
            path_node->next = NULL;

            if (!result->first_step)
            {
                result->first_step = path_node;
                result->last_step = path_node;
            }
            else
            {
                result->last_step->next = path_node;
                result->last_step = path_node;
            }
        }
    }

    /* Cleanup */
    DISPOSE(visited);
    DISPOSE(parent_room);
    DISPOSE(parent_dir);

    return result;
}

/*
 * Free path result
 */
void free_path(PATH_RESULT *path)
{
    PATH_NODE *node, *node_next;

    if (!path)
        return;

    for (node = path->first_step; node; node = node_next)
    {
        node_next = node->next;
        DISPOSE(node);
    }

    DISPOSE(path);
}

/*
 * Get next direction to move toward target
 */
int get_next_step_to(CHAR_DATA *mob, int target_vnum)
{
    PATH_RESULT *path;
    int direction = -1;

    if (!mob || !mob->in_room)
        return -1;

    /* Check cache first */
    path = get_cached_path(mob->in_room->vnum, target_vnum);

    if (!path)
    {
        /* Calculate new path */
        path = find_path(mob->in_room->vnum, target_vnum, 30);

        if (path && path->path_found)
        {
            /* Cache it */
            cache_path(mob->in_room->vnum, target_vnum, path);
        }
    }

    if (path && path->path_found && path->first_step)
    {
        direction = path->first_step->direction;
    }

    return direction;
}

/*
 * Move mob one step toward target
 */
bool mob_move_along_path(CHAR_DATA *mob, int target_vnum)
{
    int direction;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;

    direction = get_next_step_to(mob, target_vnum);

    if (direction < 0 || direction > 5)
        return FALSE;

    pexit = get_exit(mob->in_room, direction);
    if (!pexit || !pexit->to_room)
        return FALSE;

    to_room = pexit->to_room;

    /* Move */
    act(AT_GREY, "$n heads $T.", mob, NULL, dir_name[direction], TO_ROOM);
    char_from_room(mob);
    char_to_room(mob, to_room);
    act(AT_GREY, "$n arrives.", mob, NULL, NULL, TO_ROOM);

    return TRUE;
}

/*
 * Cache a path
 */
void cache_path(int from, int to, PATH_RESULT *path)
{
    CACHED_PATH *cache;

    /* Limit cache size */
    if (total_cached_paths >= PATH_CACHE_SIZE)
    {
        /* TODO: Remove oldest entries */
        return;
    }

    CREATE(cache, CACHED_PATH, 1);
    cache->from_vnum = from;
    cache->to_vnum = to;
    cache->path = path;
    cache->cached_time = time(NULL);

    cache->next = first_cached_path;
    first_cached_path = cache;
    total_cached_paths++;
}

/*
 * Get cached path
 */
PATH_RESULT *get_cached_path(int from, int to)
{
    CACHED_PATH *cache;
    time_t now = time(NULL);

    for (cache = first_cached_path; cache; cache = cache->next)
    {
        if (cache->from_vnum == from && cache->to_vnum == to)
        {
            /* Paths expire after 1 hour */
            if (now - cache->cached_time < 3600)
                return cache->path;
        }
    }

    return NULL;
}
