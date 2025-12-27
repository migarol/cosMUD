/*****************************************************************************
 * Pathfinding System for Mobs
 *
 * Mobs can find routes from their current location to target destinations.
 * Used for custom schedules - Tsythia knows HOW to get to her study.
 *****************************************************************************/

#ifndef MOB_PATHFINDING_H
#define MOB_PATHFINDING_H

#define MAX_PATH_LENGTH 50

/* Path node */
typedef struct path_node {
    int room_vnum;
    int direction;  /* Direction to take from this room */
    struct path_node *next;
} PATH_NODE;

/* Pathfinding result */
typedef struct path_result {
    int from_vnum;
    int to_vnum;
    int path_length;
    PATH_NODE *first_step;
    PATH_NODE *last_step;
    bool path_found;
} PATH_RESULT;

/* Function declarations */
PATH_RESULT *find_path(int from_vnum, int to_vnum, int max_distance);
void free_path(PATH_RESULT *path);
bool mob_move_along_path(CHAR_DATA *mob, int target_vnum);
int get_next_step_to(CHAR_DATA *mob, int target_vnum);
void cache_path(int from, int to, PATH_RESULT *path);
PATH_RESULT *get_cached_path(int from, int to);

#endif /* MOB_PATHFINDING_H */
