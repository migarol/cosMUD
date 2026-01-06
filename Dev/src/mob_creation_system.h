/****************************************************************************
 * Mob Creation System - Mobs create other mobs and objects
 * Blacksmiths create apprentices, kings appoint guards, etc.
 ****************************************************************************/

#ifndef MOB_CREATION_SYSTEM_H
#define MOB_CREATION_SYSTEM_H

/* Creation types */
typedef enum {
    CREATION_APPRENTICE,  /* Skilled mob trains helper */
    CREATION_GUARD,       /* Leader hires guard */
    CREATION_MERCHANT,    /* Create trader */
    CREATION_CITIZEN,     /* Generic NPC */
    CREATION_CHILD,       /* Family system */
    CREATION_OBJECT       /* Craft unique item */
} CREATION_TYPE;

/* Functions */
void init_mob_creation_system(void);
bool mob_can_create_type(CHAR_DATA *creator, CREATION_TYPE type);
CHAR_DATA *mob_create_npc(CHAR_DATA *creator, CREATION_TYPE type, char *description);
OBJ_DATA *mob_create_unique_object(CHAR_DATA *crafter, char *description);
void mob_creation_update(void);

#endif
