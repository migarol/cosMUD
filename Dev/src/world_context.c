/*****************************************************************************
 * World Context Engine - Geographic, Political, Economic Analysis
 *
 * Provides deep analysis of areas for congruence checking before creation.
 * Beeler uses this to ensure everything makes sense.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"
#include "world_context.h"
#include "ollama_integration.h"

/* Global world context */
WORLD_CONTEXT *global_world_context = NULL;

/* Area context cache */
#define MAX_AREA_CONTEXTS 100
AREA_CONTEXT *area_context_cache[MAX_AREA_CONTEXTS];
int num_cached_contexts = 0;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_world_context(void)
{
    int i;

    log_string("WORLD CONTEXT: Initializing...");

    /* Allocate global context */
    global_world_context = (WORLD_CONTEXT *)calloc(1, sizeof(WORLD_CONTEXT));
    if (!global_world_context)
    {
        bug("init_world_context: Failed to allocate global context");
        return;
    }

    /* Initialize cache */
    for (i = 0; i < MAX_AREA_CONTEXTS; i++)
        area_context_cache[i] = NULL;

    num_cached_contexts = 0;

    log_string("WORLD CONTEXT: Initialized");
}

void scan_entire_world(void)
{
    AREA_DATA *area;
    int count = 0;

    log_string("WORLD CONTEXT: Scanning entire world...");

    if (!global_world_context)
    {
        bug("scan_entire_world: Global context not initialized");
        return;
    }

    /* Scan all areas */
    for (area = first_area; area; area = area->next)
    {
        AREA_CONTEXT *ctx = analyze_area(area);
        if (ctx)
            count++;
    }

    global_world_context->num_areas = count;
    global_world_context->last_full_scan = time(NULL);

    sprintf(log_buf, "WORLD CONTEXT: Scanned %d areas", count);
    log_string(log_buf);
}

/*****************************************************************************
 * Area Analysis - Core Function
 *****************************************************************************/

AREA_CONTEXT *analyze_area(AREA_DATA *area)
{
    AREA_CONTEXT *ctx;
    int i;

    if (!area)
        return NULL;

    /* Check cache first */
    for (i = 0; i < num_cached_contexts; i++)
    {
        if (area_context_cache[i] && area_context_cache[i]->area == area)
        {
            /* Check if cache is still fresh (< 30 minutes old) */
            if (time(NULL) - area_context_cache[i]->last_updated < 1800)
                return area_context_cache[i];
        }
    }

    /* Allocate new context */
    ctx = (AREA_CONTEXT *)calloc(1, sizeof(AREA_CONTEXT));
    if (!ctx)
    {
        bug("analyze_area: Failed to allocate context");
        return NULL;
    }

    ctx->area = area;
    ctx->last_updated = time(NULL);
    ctx->needs_refresh = FALSE;

    /* GEOGRAPHY */
    ctx->elevation = detect_elevation(area);
    ctx->water_access = detect_water_access(area);
    ctx->forest_density = detect_forest_density(area);
    ctx->geographic_features = 0;

    if (area_is_mountainous(area))
        ctx->geographic_features |= GEO_MOUNTAINS;
    if (area_is_coastal(area))
        ctx->geographic_features |= GEO_COAST;
    if (ctx->forest_density > 5)
        ctx->geographic_features |= GEO_FOREST;
    if (area_is_underground(area))
        ctx->geographic_features |= GEO_UNDERGROUND;

    ctx->terrain_description = describe_terrain(ctx);

    /* POPULATION (count NPCs in area) */
    ctx->estimated_population = count_npcs_in_area(area);
    ctx->num_npcs_in_area = ctx->estimated_population;

    /* ECONOMIC */
    ctx->economic_status = calculate_basic_economic_status(area);
    ctx->trade_routes_count = 0; /* TODO: Implement trade route detection */

    /* POLITICAL */
    ctx->control_type = CONTROL_INDEPENDENT; /* Default */
    ctx->sphere_of_influence = 0;

    /* Cache it */
    if (num_cached_contexts < MAX_AREA_CONTEXTS)
    {
        area_context_cache[num_cached_contexts] = ctx;
        num_cached_contexts++;
    }

    return ctx;
}

/*****************************************************************************
 * Geographic Analysis
 *****************************************************************************/

int detect_elevation(AREA_DATA *area)
{
    ROOM_INDEX_DATA *room;
    int vnum, count = 0, elevation_sum = 0;

    if (!area)
        return 0;

    /* Sample rooms in area */
    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);
        if (room)
        {
            /* Estimate elevation from sector type */
            if (room->sector_type == SECT_MOUNTAIN)
                elevation_sum += 80;
            else if (room->sector_type == SECT_HILLS)
                elevation_sum += 50;
            else if (room->sector_type == SECT_WATER_SWIM ||
                     room->sector_type == SECT_WATER_NOSWIM)
                elevation_sum += 0;
            else if (room->sector_type == SECT_UNDERWATER)
                elevation_sum += -50;
            else
                elevation_sum += 20; /* Default moderate elevation */

            count++;
        }
    }

    if (count == 0)
        return 0;

    return elevation_sum / count;
}

int detect_water_access(AREA_DATA *area)
{
    ROOM_INDEX_DATA *room;
    int vnum, water_rooms = 0, total_rooms = 0;

    if (!area)
        return 0;

    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);
        if (room)
        {
            total_rooms++;
            if (room->sector_type == SECT_WATER_SWIM ||
                room->sector_type == SECT_WATER_NOSWIM ||
                room->sector_type == SECT_UNDERWATER)
            {
                water_rooms++;
            }
        }
    }

    if (total_rooms == 0)
        return 0;

    /* Return 0-10 scale */
    return (water_rooms * 10) / total_rooms;
}

int detect_forest_density(AREA_DATA *area)
{
    ROOM_INDEX_DATA *room;
    int vnum, forest_rooms = 0, total_rooms = 0;

    if (!area)
        return 0;

    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);
        if (room)
        {
            total_rooms++;
            if (room->sector_type == SECT_FOREST ||
                room->sector_type == SECT_FOREST)
            {
                forest_rooms++;
            }
        }
    }

    if (total_rooms == 0)
        return 0;

    /* Return 0-10 scale */
    return (forest_rooms * 10) / total_rooms;
}

bool area_is_coastal(AREA_DATA *area)
{
    if (!area)
        return FALSE;

    /* Check water access - coastal if moderate water */
    int water = detect_water_access(area);
    return (water >= 3 && water <= 7);
}

bool area_is_mountainous(AREA_DATA *area)
{
    if (!area)
        return FALSE;

    int elevation = detect_elevation(area);
    return (elevation >= 60);
}

bool area_is_underground(AREA_DATA *area)
{
    ROOM_INDEX_DATA *room;
    int vnum, underground_count = 0, total = 0;

    if (!area)
        return FALSE;

    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);
        if (room)
        {
            total++;
            if (room->sector_type == SECT_UNDERGROUND ||
                room->sector_type == SECT_UNDERWATER)
            {
                underground_count++;
            }
        }
    }

    /* Underground if majority of rooms are underground */
    return (total > 0 && underground_count > (total / 2));
}

char *describe_terrain(AREA_CONTEXT *ctx)
{
    static char buf[256];
    char features[256];

    if (!ctx)
        return "unknown";

    features[0] = '\0';

    if (ctx->geographic_features & GEO_MOUNTAINS)
        strcat(features, "mountainous ");
    if (ctx->geographic_features & GEO_FOREST)
        strcat(features, "forested ");
    if (ctx->geographic_features & GEO_COAST)
        strcat(features, "coastal ");
    if (ctx->geographic_features & GEO_UNDERGROUND)
        strcat(features, "underground ");

    if (features[0] == '\0')
        strcpy(features, "plains ");

    sprintf(buf, "%sterrain", features);
    return buf;
}

/*****************************************************************************
 * Population & Economic Analysis
 *****************************************************************************/

int count_npcs_in_area(AREA_DATA *area)
{
    ROOM_INDEX_DATA *room;
    CHAR_DATA *mob;
    int vnum, count = 0;

    if (!area)
        return 0;

    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);
        if (room)
        {
            for (mob = room->first_person; mob; mob = mob->next_in_room)
            {
                if (IS_NPC(mob))
                    count++;
            }
        }
    }

    return count;
}

int calculate_basic_economic_status(AREA_DATA *area)
{
    int status = ECON_STABLE; /* Default */

    if (!area)
        return status;

    /* Use area's economic fields if available */
    if (area->high_economy > 1000)
        status = ECON_WEALTHY;
    else if (area->high_economy > 500)
        status = ECON_PROSPEROUS;
    else if (area->low_economy < 100)
        status = ECON_STRUGGLING;
    else if (area->low_economy < 50)
        status = ECON_IMPOVERISHED;

    return status;
}

/*****************************************************************************
 * Congruence Checking - THE CRITICAL FUNCTION
 *****************************************************************************/

CONGRUENCE_CHECK *check_congruence(char *what_to_create, AREA_DATA *where, char *reason)
{
    CONGRUENCE_CHECK *check;
    AREA_CONTEXT *ctx;

    if (!what_to_create || !where)
        return NULL;

    /* Allocate check result */
    check = (CONGRUENCE_CHECK *)calloc(1, sizeof(CONGRUENCE_CHECK));
    if (!check)
    {
        bug("check_congruence: Failed to allocate check");
        return NULL;
    }

    /* Get area context */
    ctx = analyze_area(where);
    if (!ctx)
    {
        check->is_congruent = FALSE;
        check->confidence = 0;
        return check;
    }

    /* Validate each aspect */
    check->geography_fits = validate_geography_congruence(what_to_create, ctx);
    check->politics_fit = validate_political_congruence(what_to_create, ctx);
    check->economy_fits = validate_economic_congruence(what_to_create, ctx);
    check->culture_fits = validate_cultural_congruence(what_to_create, ctx);
    check->lore_fits = validate_lore_congruence(what_to_create, ctx);

    /* Count passing validations */
    int passed = 0;
    if (check->geography_fits) passed++;
    if (check->politics_fit) passed++;
    if (check->economy_fits) passed++;
    if (check->culture_fits) passed++;
    if (check->lore_fits) passed++;

    /* Overall congruence */
    check->is_congruent = (passed >= 3); /* Need at least 3/5 */
    check->confidence = (passed * 100) / 5;

    /* AI analysis if available */
    if (ollama_is_available())
    {
        check->ai_analysis = ai_congruence_analysis(what_to_create, ctx, global_world_context);
    }
    else
    {
        check->ai_analysis = strdup("AI analysis unavailable (Ollama not connected)");
    }

    sprintf(log_buf, "CONGRUENCE: '%s' in '%s' = %s (confidence %d%%)",
            what_to_create, where->name,
            check->is_congruent ? "CONGRUENT" : "INCONGRUENT",
            check->confidence);
    log_string(log_buf);

    return check;
}

bool validate_geography_congruence(char *what, AREA_CONTEXT *where)
{
    if (!what || !where)
        return FALSE;

    /* Simple keyword matching for now */
    /* TODO: More sophisticated checking */

    /* Fishing needs water */
    if (strstr(what, "fish") && where->water_access < 3)
        return FALSE;

    /* Mining needs mountains */
    if (strstr(what, "mine") && !(where->geographic_features & GEO_MOUNTAINS))
        return FALSE;

    /* Lumber needs forest */
    if (strstr(what, "lumber") && where->forest_density < 5)
        return FALSE;

    /* Default: assume congruent */
    return TRUE;
}

bool validate_political_congruence(char *what, AREA_CONTEXT *where)
{
    /* Simple validation for now */
    /* TODO: Check sphere of influence, political control */
    return TRUE;
}

bool validate_economic_congruence(char *what, AREA_CONTEXT *where)
{
    /* Simple validation */
    /* TODO: Check if economy can support this */
    return TRUE;
}

bool validate_cultural_congruence(char *what, AREA_CONTEXT *where)
{
    /* Simple validation */
    /* TODO: Check cultural theme */
    return TRUE;
}

bool validate_lore_congruence(char *what, AREA_CONTEXT *where)
{
    /* Simple validation */
    /* TODO: Check area lore/description */
    return TRUE;
}

/*****************************************************************************
 * AI-Powered Analysis (Ollama Integration)
 *****************************************************************************/

char *ai_congruence_analysis(char *what, AREA_CONTEXT *where, WORLD_CONTEXT *world)
{
    char prompt[MAX_STRING_LENGTH * 2];
    char *response;

    if (!what || !where)
        return strdup("Invalid parameters");

    sprintf(prompt,
        "Analyze if creating '%s' in this area makes sense:\n\n"
        "AREA: %s\n"
        "Terrain: %s\n"
        "Elevation: %d (0=sea level, 100=mountain peak)\n"
        "Water access: %d/10\n"
        "Forest density: %d/10\n"
        "Population: %d NPCs\n"
        "Economic status: %d\n\n"
        "Does this make geographic, economic, and cultural sense?\n"
        "Answer in 2-3 sentences. Focus on why or why not.",
        what,
        where->area ? where->area->name : "unknown",
        where->terrain_description ? where->terrain_description : "unknown",
        where->elevation,
        where->water_access,
        where->forest_density,
        where->estimated_population,
        where->economic_status
    );

    response = ollama_request(prompt, 200);

    if (!response || response[0] == '\0')
        return strdup("AI analysis failed");

    return response;
}

/*****************************************************************************
 * Utility Functions
 *****************************************************************************/

void update_world_context(void)
{
    /* Incremental update - refresh stale caches */
    int i;
    time_t now = time(NULL);

    for (i = 0; i < num_cached_contexts; i++)
    {
        if (area_context_cache[i])
        {
            /* Refresh if > 30 minutes old */
            if (now - area_context_cache[i]->last_updated > 1800)
            {
                area_context_cache[i]->needs_refresh = TRUE;
            }
        }
    }
}

AREA_CONTEXT *get_area_context(AREA_DATA *area)
{
    if (!area)
        return NULL;

    return analyze_area(area);
}

WORLD_CONTEXT *get_world_context(void)
{
    return global_world_context;
}
