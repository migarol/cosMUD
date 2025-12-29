/*****************************************************************************
 * Resource Distribution System - Geographic Resources
 *
 * Maps resources to geography:
 * - Mountains have ore, gems, stone
 * - Coasts have fish, salt, pearls
 * - Forests have wood, herbs, game
 * - Plains have grain, livestock
 *
 * Scarcity creates trade needs
 * Resource depletion over time
 * Renewable vs non-renewable resources
 *
 * Integration: world_context.h for geography, economy.h for pricing
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "resource_distribution.h"
#include "world_context.h"
#include "economy.h"
#include "periodicos.h"

/* Resource types */
#define RESOURCE_ORE            0
#define RESOURCE_GEMS           1
#define RESOURCE_STONE          2
#define RESOURCE_WOOD           3
#define RESOURCE_HERBS          4
#define RESOURCE_FISH           5
#define RESOURCE_SALT           6
#define RESOURCE_GRAIN          7
#define RESOURCE_LIVESTOCK      8
#define RESOURCE_GAME           9
#define RESOURCE_PEARLS        10
#define RESOURCE_COAL          11
#define MAX_RESOURCE_TYPES     12

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

RESOURCE_NODE *first_resource_node = NULL;
int total_resource_nodes = 0;

/* Resource info table */
struct resource_info {
    char *name;
    bool renewable;
    int base_value;
} resource_table[MAX_RESOURCE_TYPES] = {
    {"iron ore",    FALSE, 50},
    {"gems",        FALSE, 500},
    {"stone",       TRUE,  10},
    {"wood",        TRUE,  20},
    {"herbs",       TRUE,  30},
    {"fish",        TRUE,  15},
    {"salt",        TRUE,  25},
    {"grain",       TRUE,  10},
    {"livestock",   TRUE,  100},
    {"game meat",   TRUE,  40},
    {"pearls",      FALSE, 300},
    {"coal",        FALSE, 40}
};

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_resource_distribution(void)
{
    log_string("Initializing Resource Distribution System...");
    first_resource_node = NULL;
    total_resource_nodes = 0;
    log_string("Resource Distribution System initialized.");
}

void load_resources(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "resources.dat");

    if ((fp = fopen(filename, "r")) == NULL)
    {
        log_string("No saved resources to load.");
        return;
    }

    log_string("Loading resource distribution...");
    fclose(fp);
}

void save_resources(void)
{
    FILE *fp;
    char filename[256];
    RESOURCE_NODE *node;

    sprintf(filename, "%s%s", SYSTEM_DIR, "resources.dat");

    if ((fp = fopen(filename, "w")) == NULL)
    {
        log_string("ERROR: Cannot save resources!");
        return;
    }

    fprintf(fp, "#RESOURCES\n");

    for (node = first_resource_node; node; node = node->next)
    {
        fprintf(fp, "AreaVnum %d\n", node->area->vnum);
        fprintf(fp, "Type %d\n", node->resource_type);
        fprintf(fp, "Abundance %d\n", node->abundance);
        fprintf(fp, "Quality %d\n", node->quality);
        fprintf(fp, "Supply %d %d\n", node->current_supply, node->max_supply);
        fprintf(fp, "End\n\n");
    }

    fprintf(fp, "#END\n");
    fclose(fp);
}

/*****************************************************************************
 * Resource Node Creation
 *****************************************************************************/

RESOURCE_NODE *create_resource_node(AREA_DATA *area, int resource_type, int abundance)
{
    RESOURCE_NODE *node;

    if (!area || resource_type < 0 || resource_type >= MAX_RESOURCE_TYPES)
        return NULL;

    CREATE(node, RESOURCE_NODE, 1);
    node->area = area;
    node->resource_type = resource_type;
    node->abundance = URANGE(0, abundance, 100);
    node->quality = number_range(40, 90);
    node->renewable = resource_table[resource_type].renewable;
    node->regeneration_rate = node->renewable ? abundance / 10 : 0;
    node->max_supply = abundance * 1000;
    node->current_supply = node->max_supply;
    node->extraction_rate = abundance / 5;
    node->last_harvest = 0;
    node->depleted = FALSE;
    node->market_price = resource_table[resource_type].base_value;
    node->demand = 50; /* Medium demand */
    node->next = first_resource_node;
    first_resource_node = node;
    total_resource_nodes++;

    log_string("RESOURCES: Created %s node in %s (abundance %d, quality %d)",
               resource_table[resource_type].name,
               area->name,
               node->abundance,
               node->quality);

    return node;
}

/*****************************************************************************
 * Geography-Based Distribution - THE CORE FEATURE
 *****************************************************************************/

void distribute_resources_by_geography(AREA_DATA *area)
{
    AREA_CONTEXT *ctx;
    int geo_features;

    if (!area)
        return;

    ctx = analyze_area(area);
    if (!ctx)
        return;

    geo_features = ctx->geographic_features;

    log_string("RESOURCES: Distributing resources for %s (features: %d)",
               area->name, geo_features);

    /* MOUNTAINS - ore, gems, stone, coal */
    if (geo_features & GEO_MOUNTAINS)
    {
        create_resource_node(area, RESOURCE_ORE, number_range(60, 90));
        create_resource_node(area, RESOURCE_STONE, number_range(70, 95));
        create_resource_node(area, RESOURCE_COAL, number_range(50, 80));

        if (number_percent() < 40)
            create_resource_node(area, RESOURCE_GEMS, number_range(30, 60));
    }

    /* COAST - fish, salt, pearls */
    if (geo_features & GEO_COAST)
    {
        create_resource_node(area, RESOURCE_FISH, number_range(70, 95));
        create_resource_node(area, RESOURCE_SALT, number_range(60, 85));

        if (number_percent() < 20)
            create_resource_node(area, RESOURCE_PEARLS, number_range(20, 40));
    }

    /* FOREST - wood, herbs, game */
    if (geo_features & GEO_FOREST)
    {
        create_resource_node(area, RESOURCE_WOOD, number_range(70, 95));
        create_resource_node(area, RESOURCE_HERBS, number_range(50, 80));
        create_resource_node(area, RESOURCE_GAME, number_range(40, 70));
    }

    /* PLAINS - grain, livestock */
    if (geo_features & GEO_PLAINS)
    {
        create_resource_node(area, RESOURCE_GRAIN, number_range(75, 95));
        create_resource_node(area, RESOURCE_LIVESTOCK, number_range(60, 85));
    }

    /* SWAMP - herbs (different types) */
    if (geo_features & GEO_SWAMP)
    {
        create_resource_node(area, RESOURCE_HERBS, number_range(60, 90));
    }
}

void scan_all_areas_for_resources(void)
{
    AREA_DATA *area;
    int count = 0;

    log_string("RESOURCES: Scanning all areas for resource distribution...");

    for (area = first_area; area; area = area->next)
    {
        distribute_resources_by_geography(area);
        count++;
    }

    log_string("RESOURCES: Scanned %d areas, created %d resource nodes",
               count, total_resource_nodes);
}

/*****************************************************************************
 * Resource Extraction
 *****************************************************************************/

int harvest_resource(AREA_DATA *area, int resource_type, int amount)
{
    RESOURCE_NODE *node;
    int harvested = 0;

    /* Find resource node */
    for (node = first_resource_node; node; node = node->next)
    {
        if (node->area == area && node->resource_type == resource_type)
        {
            if (node->depleted)
                return 0;

            /* Can't harvest more than current supply */
            harvested = UMIN(amount, node->current_supply);
            node->current_supply -= harvested;

            /* Check depletion */
            if (node->current_supply <= 0)
            {
                node->depleted = TRUE;
                node->current_supply = 0;

                log_string("RESOURCES: %s depleted in %s!",
                           resource_table[resource_type].name,
                           area->name);

                /* Announce depletion */
                smart_announce(
                    "Resource Depleted",
                    "Critical resource shortage",
                    EVENT_CATEGORY_ECONOMY,
                    ANNOUNCE_PRIORITY_HIGH,
                    area->name
                );
            }

            /* Update price based on scarcity */
            update_resource_price(node);

            return harvested;
        }
    }

    return 0; /* Resource not available in this area */
}

void update_resource_price(RESOURCE_NODE *node)
{
    int scarcity;
    int base_price;

    if (!node)
        return;

    /* Calculate scarcity (0-100) */
    if (node->max_supply > 0)
        scarcity = 100 - ((node->current_supply * 100) / node->max_supply);
    else
        scarcity = 100;

    /* Price increases with scarcity */
    base_price = resource_table[node->resource_type].base_value;
    node->market_price = base_price + (base_price * scarcity / 100);

    /* Quality affects price */
    node->market_price = (node->market_price * node->quality) / 100;

    /* Demand affects price */
    node->market_price = (node->market_price * (50 + node->demand)) / 100;
}

/*****************************************************************************
 * Resource Regeneration
 *****************************************************************************/

void regenerate_resources(void)
{
    RESOURCE_NODE *node;
    int regenerated;

    for (node = first_resource_node; node; node = node->next)
    {
        if (!node->renewable)
            continue;

        if (node->depleted && node->current_supply <= 0)
        {
            /* Depleted resources take longer to recover */
            if (number_percent() < 10)
            {
                node->depleted = FALSE;
                node->current_supply = node->regeneration_rate;
                log_string("RESOURCES: %s recovering in %s",
                           resource_table[node->resource_type].name,
                           node->area->name);
            }
        }
        else
        {
            /* Normal regeneration */
            regenerated = node->regeneration_rate;
            node->current_supply += regenerated;

            /* Cap at max supply */
            if (node->current_supply > node->max_supply)
                node->current_supply = node->max_supply;
        }
    }
}

/*****************************************************************************
 * Resource Queries
 *****************************************************************************/

RESOURCE_NODE *find_resource_in_area(AREA_DATA *area, int resource_type)
{
    RESOURCE_NODE *node;

    for (node = first_resource_node; node; node = node->next)
    {
        if (node->area == area && node->resource_type == resource_type)
            return node;
    }

    return NULL;
}

bool area_has_resource(AREA_DATA *area, int resource_type)
{
    return (find_resource_in_area(area, resource_type) != NULL);
}

int get_resource_abundance(AREA_DATA *area, int resource_type)
{
    RESOURCE_NODE *node = find_resource_in_area(area, resource_type);

    if (node)
        return node->abundance;

    return 0; /* Not available */
}

AREA_DATA **find_areas_with_resource(int resource_type, int *num_areas)
{
    static AREA_DATA *areas[100];
    RESOURCE_NODE *node;
    int count = 0;

    for (node = first_resource_node; node && count < 100; node = node->next)
    {
        if (node->resource_type == resource_type && !node->depleted)
        {
            areas[count++] = node->area;
        }
    }

    *num_areas = count;
    return areas;
}

/*****************************************************************************
 * Trade Need Analysis - Creates trade opportunities
 *****************************************************************************/

void analyze_resource_needs(AREA_DATA *area)
{
    int i;
    RESOURCE_NODE *node;
    bool has_food = FALSE;
    bool has_building_materials = FALSE;

    /* Check what this area has */
    for (node = first_resource_node; node; node = node->next)
    {
        if (node->area != area)
            continue;

        /* Food resources */
        if (node->resource_type == RESOURCE_FISH ||
            node->resource_type == RESOURCE_GRAIN ||
            node->resource_type == RESOURCE_LIVESTOCK ||
            node->resource_type == RESOURCE_GAME)
        {
            has_food = TRUE;
        }

        /* Building materials */
        if (node->resource_type == RESOURCE_STONE ||
            node->resource_type == RESOURCE_WOOD)
        {
            has_building_materials = TRUE;
        }
    }

    /* Log needs */
    if (!has_food)
    {
        log_string("RESOURCES: %s needs food imports", area->name);
        /* This creates trade opportunities */
    }

    if (!has_building_materials)
    {
        log_string("RESOURCES: %s needs building materials", area->name);
    }
}

/*****************************************************************************
 * Update Loop
 *****************************************************************************/

void resource_distribution_update(void)
{
    static time_t last_regen = 0;
    time_t now = time(NULL);

    /* Regenerate monthly */
    if (difftime(now, last_regen) >= (30 * 86400 / GAME_TIME_MULTIPLIER))
    {
        last_regen = now;
        log_string("RESOURCES: Monthly regeneration...");
        regenerate_resources();
    }
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_resources(CHAR_DATA *ch, char *argument)
{
    AREA_DATA *area;
    RESOURCE_NODE *node;
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    if (IS_NPC(ch))
        return;

    area = ch->in_room ? ch->in_room->area : NULL;
    if (!area)
    {
        send_to_char("You are nowhere.\n\r", ch);
        return;
    }

    sprintf(buf, "&c=== Resources in %s ===&w\n\r\n\r", area->name);
    send_to_char(buf, ch);

    for (node = first_resource_node; node; node = node->next)
    {
        if (node->area == area)
        {
            sprintf(buf, "&Y%s&w\n\r", resource_table[node->resource_type].name);
            send_to_char(buf, ch);

            sprintf(buf, "  Abundance: %d/100, Quality: %d/100\n\r",
                    node->abundance, node->quality);
            send_to_char(buf, ch);

            sprintf(buf, "  Supply: %d/%d%s\n\r",
                    node->current_supply,
                    node->max_supply,
                    node->depleted ? " &R(DEPLETED)&w" : "");
            send_to_char(buf, ch);

            sprintf(buf, "  Price: %d gold, Demand: %d/100\n\r",
                    node->market_price,
                    node->demand);
            send_to_char(buf, ch);

            if (node->renewable)
                sprintf(buf, "  &G(Renewable: +%d/month)&w\n\r", node->regeneration_rate);
            else
                sprintf(buf, "  &R(Non-renewable)&w\n\r");
            send_to_char(buf, ch);

            send_to_char("\n\r", ch);
            count++;
        }
    }

    if (count == 0)
        send_to_char("No resources mapped in this area.\n\r", ch);
}

void do_scanresources(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    send_to_char("Scanning all areas for resources...\n\r", ch);
    scan_all_areas_for_resources();
    send_to_char("Resource scan complete!\n\r", ch);
}

void do_harvestresource(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    int resource_type;
    int harvested;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Harvest which resource?\n\r", ch);
        return;
    }

    /* Find resource type by name */
    for (resource_type = 0; resource_type < MAX_RESOURCE_TYPES; resource_type++)
    {
        if (strstr(resource_table[resource_type].name, arg))
            break;
    }

    if (resource_type >= MAX_RESOURCE_TYPES)
    {
        send_to_char("Invalid resource type.\n\r", ch);
        return;
    }

    harvested = harvest_resource(ch->in_room->area, resource_type, 100);

    if (harvested > 0)
    {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "Harvested %d units of %s.\n\r",
                harvested, resource_table[resource_type].name);
        send_to_char(buf, ch);
    }
    else
    {
        send_to_char("Resource not available or depleted.\n\r", ch);
    }
}
