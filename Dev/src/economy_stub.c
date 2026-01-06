/*****************************************************************************
 * Economy System - STUB implementations
 *
 * TODO: Full economy system to be integrated later
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mud.h"
#include "economy.h"

/* Stub implementation */
AREA_ECONOMY *get_area_economy(AREA_DATA *area)
{
    /* TODO: Return actual economy data */
    return NULL;
}

/* Stub implementation */
void trigger_specific_event(AREA_ECONOMY *aecon, int event_type)
{
    /* TODO: Implement economic events */
    /* For now, do nothing */
    return;
}

/* Stub implementation */
int get_profession_by_name(const char *name)
{
    if (!name)
        return PROF_NONE;

    if (str_cmp(name, "farmer") == 0)
        return PROF_FARMER;
    if (str_cmp(name, "miner") == 0)
        return PROF_MINER;
    if (str_cmp(name, "lumberjack") == 0)
        return PROF_LUMBERJACK;

    return PROF_NONE;
}

/* Stub implementation */
void set_npc_profession(CHAR_DATA *mob, int profession)
{
    /* TODO: Store profession in mob data */
    return;
}

/* Stub implementation */
const char *profession_name(int prof)
{
    static char none_str[] = "None";
    static char farmer_str[] = "Farmer";
    static char miner_str[] = "Miner";
    static char lumberjack_str[] = "Lumberjack";
    static char unknown_str[] = "Unknown";

    switch (prof)
    {
        case PROF_NONE:       return none_str;
        case PROF_FARMER:     return farmer_str;
        case PROF_MINER:      return miner_str;
        case PROF_LUMBERJACK: return lumberjack_str;
        default:              return unknown_str;
    }
}

/* Stub implementation */
int get_resource_by_name(const char *name)
{
    if (!name)
        return RESOURCE_NONE;

    if (str_cmp(name, "food") == 0)
        return RESOURCE_FOOD;
    if (str_cmp(name, "wood") == 0)
        return RESOURCE_WOOD;
    if (str_cmp(name, "stone") == 0)
        return RESOURCE_STONE;

    return RESOURCE_NONE;
}

/* Stub implementation */
const char *resource_name(int resource)
{
    static char none_str[] = "None";
    static char food_str[] = "Food";
    static char wood_str[] = "Wood";
    static char stone_str[] = "Stone";
    static char unknown_str[] = "Unknown";

    switch (resource)
    {
        case RESOURCE_NONE:  return none_str;
        case RESOURCE_FOOD:  return food_str;
        case RESOURCE_WOOD:  return wood_str;
        case RESOURCE_STONE: return stone_str;
        default:             return unknown_str;
    }
}

/* Stub implementation */
void update_prices(AREA_ECONOMY *aecon)
{
    /* TODO: Implement price updates */
    return;
}

/* Stub implementation */
int get_profession_for_resource(int resource)
{
    switch (resource)
    {
        case RESOURCE_FOOD:  return PROF_FARMER;
        case RESOURCE_WOOD:  return PROF_LUMBERJACK;
        case RESOURCE_STONE: return PROF_MINER;
        default:             return PROF_NONE;
    }
}
