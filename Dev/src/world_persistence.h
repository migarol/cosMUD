/****************************************************************************
 * World Persistence System - Header
 ****************************************************************************/

#ifndef WORLD_PERSISTENCE_H
#define WORLD_PERSISTENCE_H

/* Global variables */
extern int total_autocreated_mobs;
extern time_t last_world_save;

/* Functions */
void save_autocreated_mobs(void);
void load_autocreated_mobs(void);
int cleanup_autocreated_mobs(void);
int count_autocreated_mobs(void);
void delete_autocreated_files(void);

#endif
