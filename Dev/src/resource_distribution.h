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
/* Resource node - a source of resources in an area */
typedef struct resource_node RESOURCE_NODE;
struct resource_node {
    AREA_DATA *area;
    int resource_type;
    int abundance;              /* 0-100 */
    int quality;                /* 0-100 */

    /* Depletion tracking */
    int current_supply;         /* How much left */
    int max_supply;             /* Original amount */
    bool renewable;             /* Regenerates? */
    int regeneration_rate;      /* Per month */

    /* Extraction */
    int extraction_rate;        /* Per day */
    int last_harvest;
    bool depleted;

    /* Economic */
    int market_price;
    int demand;                 /* 0-100 */

    RESOURCE_NODE *next;
};

void init_resource_distribution(void);
void distribute_resources_by_geography(void);
void resource_depletion_update(void);
RESOURCE_NODE *create_resource_node(AREA_DATA *area, int resource_type, int abundance);
