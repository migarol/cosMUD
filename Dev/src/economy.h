/*****************************************************************************
 * Economy System - STUB for SmaugFUSS compatibility
 *
 * TODO: Full economy system to be integrated later
 *****************************************************************************/

#ifndef ECONOMY_H
#define ECONOMY_H

/* Professions */
#define PROF_NONE           0
#define PROF_FARMER         1
#define PROF_MINER          2
#define PROF_LUMBERJACK     3
#define MAX_PROFESSION      10
#define PROF_HERBALIST      4

/* Economic Events */
#define EVENT_NONE          0
#define EVENT_PLAGUE        1
#define EVENT_DROUGHT       2
#define EVENT_BUMPER_CROP   3
#define EVENT_MINE_COLLAPSE 4
#define EVENT_WOLF_ATTACK   5
#define EVENT_DISCOVERY     6

/* Resources */
#define RESOURCE_NONE       0
#define RESOURCE_FOOD       1
#define RESOURCE_WOOD       2
#define RESOURCE_STONE      3
#define MAX_RESOURCES       10

/* Forward struct declaration */
typedef struct area_economy AREA_ECONOMY;

struct area_economy {
    AREA_DATA *area;
    char *region_name;
    int prosperity_level;
    int supply[MAX_RESOURCES];
    int demand[MAX_RESOURCES];
    /* TODO: Add more economy fields later */
};

/* Function prototypes - stubs for now */
AREA_ECONOMY *get_area_economy(AREA_DATA *area);
void trigger_specific_event(AREA_ECONOMY *aecon, int event_type);
int get_profession_by_name(const char *name);
void set_npc_profession(CHAR_DATA *mob, int profession);
const char *profession_name(int prof);
int get_resource_by_name(const char *name);
const char *resource_name(int resource);
void update_prices(AREA_ECONOMY *aecon);
int get_profession_for_resource(int resource);

#endif /* ECONOMY_H */
