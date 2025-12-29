/****************************************************************************
 * Resource Distribution - Not all areas have all resources
 * Mountains have ore, coasts have fish, forests have wood
 ****************************************************************************/

#ifndef RESOURCE_DISTRIBUTION_H
#define RESOURCE_DISTRIBUTION_H

typedef enum {
    RESOURCE_ORE,
    RESOURCE_WOOD,
    RESOURCE_FISH,
    RESOURCE_GRAIN,
    RESOURCE_GEMS,
    RESOURCE_HERBS,
    RESOURCE_STONE
} RESOURCE_TYPE;

typedef struct area_resources {
    AREA_DATA *area;
    int resources[20];
    int abundance[20];
} AREA_RESOURCES;

void init_resource_distribution(void);
void assign_resources_by_geography(AREA_DATA *area);
int get_resource_abundance(AREA_DATA *area, RESOURCE_TYPE resource);

#endif
