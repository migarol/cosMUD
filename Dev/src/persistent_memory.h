/****************************************************************************
 * Persistent Memory - Save/load mob memories between reboots
 ****************************************************************************/

#ifndef PERSISTENT_MEMORY_H
#define PERSISTENT_MEMORY_H

void init_persistent_memory(void);
void save_all_mob_memories(void);
void load_all_mob_memories(void);
void save_mob_memory(CHAR_DATA *mob);
void load_mob_memory(CHAR_DATA *mob);

#endif
