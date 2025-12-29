/*****************************************************************************
 * World Context Engine - Deep Congruence Checking
 *
 * Before Beeler creates ANYTHING, he must understand EVERYTHING:
 * - Geography (mountains, forests, coasts, deserts)
 * - Political control (who controls what)
 * - Economic interdependencies (who trades with whom)
 * - Historical context (area lore, past events)
 * - Cultural themes (medieval, tribal, magical)
 * - Player actions (what have immortals/players done)
 *
 * Nothing is created in isolation. Everything must make sense.
 *
 * "To build a village, one must first understand the world."
 *****************************************************************************/

#ifndef WORLD_CONTEXT_H
#define WORLD_CONTEXT_H

/* Forward declarations */
typedef struct leader_ai_data LEADER_AI_DATA;

/* Context analysis types */
#define CONTEXT_GEOGRAPHY      0
#define CONTEXT_POLITICAL      1
#define CONTEXT_ECONOMIC       2
#define CONTEXT_HISTORICAL     3
#define CONTEXT_CULTURAL       4
#define CONTEXT_PLAYER_ACTIONS 5
#define CONTEXT_RELATIONSHIPS  6

/* Geographic features */
#define GEO_MOUNTAINS          (1 << 0)
#define GEO_FOREST             (1 << 1)
#define GEO_COAST              (1 << 2)
#define GEO_DESERT             (1 << 3)
#define GEO_PLAINS             (1 << 4)
#define GEO_SWAMP              (1 << 5)
#define GEO_TUNDRA             (1 << 6)
#define GEO_VOLCANO            (1 << 7)
#define GEO_UNDERGROUND        (1 << 8)
#define GEO_WATER              (1 << 9)

/* Political control types */
#define CONTROL_INDEPENDENT    0  /* Self-governed */
#define CONTROL_VASSAL         1  /* Controlled by another area */
#define CONTROL_DISPUTED       2  /* Multiple claimants */
#define CONTROL_NONE           3  /* No government (wilderness) */

/* Economic status */
#define ECON_WEALTHY           3
#define ECON_PROSPEROUS        2
#define ECON_STABLE            1
#define ECON_STRUGGLING        0
#define ECON_IMPOVERISHED     -1
#define ECON_COLLAPSED        -2

/* Cultural themes */
#define CULTURE_MEDIEVAL       0
#define CULTURE_TRIBAL         1
#define CULTURE_MAGICAL        2
#define CULTURE_EASTERN        3
#define CULTURE_STEAMPUNK      4
#define CULTURE_ANCIENT_RUINS  5
#define CULTURE_RELIGIOUS      6
#define CULTURE_MILITARY       7

/* Area context - comprehensive data about an area */
typedef struct area_context {
    AREA_DATA *area;

    /* GEOGRAPHY */
    int geographic_features;     /* Bitfield of GEO_* flags */
    int elevation;               /* -100 (deep sea) to 100 (mountain peak) */
    int water_access;            /* 0-10: 0=landlocked, 10=island */
    int forest_density;          /* 0-10 */
    int natural_resources;       /* Bitfield of available resources */
    char *terrain_description;   /* "mountainous with scattered forests" */

    /* POLITICAL */
    int control_type;
    AREA_DATA *controlled_by;    /* If vassal, who controls us */
    int sphere_of_influence;     /* Distance from capital */
    LEADER_AI_DATA *leader;      /* Our leader (if any) */
    int num_controlled_areas;    /* Areas we control */
    AREA_DATA **controlled_areas;

    /* ECONOMIC */
    int economic_status;
    int trade_routes_count;
    AREA_DATA **trade_partners;
    char **exported_resources;   /* What we sell */
    char **imported_resources;   /* What we buy */
    int daily_trade_volume;      /* Gold per day */

    /* HISTORICAL */
    char *area_lore;             /* Read from area file description */
    int age_in_years;            /* How long has area existed */
    char **major_events;         /* Past significant events */
    char *founding_story;

    /* CULTURAL */
    int cultural_theme;
    char *architectural_style;   /* "stone castles", "wooden huts" */
    char *naming_convention;     /* Pattern for room/mob names */
    int magic_prevalence;        /* 0-10: How common is magic */
    int technology_level;        /* 0-10: Medieval(0) to Advanced(10) */

    /* POPULATION */
    int estimated_population;
    int num_npcs_in_area;
    int num_guards;
    int num_merchants;
    int num_workers;

    /* PLAYER INTERACTIONS */
    int player_visits_count;     /* How often players visit */
    char **player_modifications; /* What immortals have built here */
    time_t last_player_visit;

    /* RELATIONSHIPS */
    int num_neighbor_areas;
    struct area_relationship {
        AREA_DATA *neighbor;
        int distance;            /* Rooms away */
        int relationship;        /* RELATION_* */
        bool connected_by_road;
    } *neighbors;

    /* AI-GENERATED ANALYSIS */
    char *ai_summary;            /* Ollama-generated area summary */
    char *ai_opportunities;      /* What could be added here */
    char *ai_threats;            /* What problems exist */

    /* CACHE */
    time_t last_updated;
    bool needs_refresh;

} AREA_CONTEXT;

/* World context - global understanding */
typedef struct world_context {
    /* All areas analyzed */
    int num_areas;
    AREA_CONTEXT **area_contexts;

    /* Global geography */
    int total_landmass;
    int total_water;
    int num_continents;
    int num_islands;

    /* Global politics */
    int num_independent_nations;
    int num_vassal_states;
    int num_active_wars;
    LEADER_AI_DATA **all_leaders;

    /* Global economy */
    int total_gold_in_world;
    int num_trade_routes;
    int global_economic_health; /* -100 to 100 */

    /* Global culture */
    char *dominant_culture;
    int magic_saturation;       /* Average magic level */
    int technology_average;

    /* Player impact */
    int total_immortal_builds;
    int total_player_hours;
    char **recent_immortal_actions;

    /* Update tracking */
    time_t last_full_scan;
    time_t last_incremental_update;

} WORLD_CONTEXT;

/* Congruence check result */
typedef struct congruence_check {
    bool is_congruent;
    int confidence;              /* 0-100: How sure are we */

    /* Detailed analysis */
    bool geography_fits;
    bool politics_fit;
    bool economy_fits;
    bool culture_fits;
    bool lore_fits;

    /* Issues found */
    int num_issues;
    char **issues;               /* "Mountains don't have fishing" */

    /* Suggestions */
    int num_suggestions;
    char **suggestions;          /* "Consider coastal area instead" */

    /* AI reasoning */
    char *ai_analysis;           /* Detailed Ollama explanation */

} CONGRUENCE_CHECK;

/* Function declarations */

/* Initialization */
void init_world_context(void);
void scan_entire_world(void);   /* Deep scan at boot - takes time */
void update_world_context(void); /* Incremental updates */

/* Area analysis */
AREA_CONTEXT *analyze_area(AREA_DATA *area);
char *ai_analyze_area_geography(AREA_DATA *area);
char *ai_analyze_area_culture(AREA_DATA *area);
char *ai_analyze_area_history(AREA_DATA *area);
void detect_natural_resources(AREA_CONTEXT *ctx);
void detect_political_control(AREA_CONTEXT *ctx);
void analyze_trade_potential(AREA_CONTEXT *ctx);

/* Geographic analysis */
int detect_elevation(AREA_DATA *area);
int detect_water_access(AREA_DATA *area);
int detect_forest_density(AREA_DATA *area);
bool area_is_coastal(AREA_DATA *area);
bool area_is_mountainous(AREA_DATA *area);
bool area_is_underground(AREA_DATA *area);
char *describe_terrain(AREA_CONTEXT *ctx);

/* Political analysis */
AREA_DATA *detect_controlling_power(AREA_DATA *area);
int calculate_sphere_of_influence(AREA_DATA *capital, AREA_DATA *target);
bool area_in_sphere_of(AREA_DATA *capital, AREA_DATA *target, int max_distance);
LEADER_AI_DATA *find_nearest_leader(AREA_DATA *area);

/* Economic analysis */
void detect_trade_routes(AREA_CONTEXT *ctx);
void calculate_economic_status(AREA_CONTEXT *ctx);
char **determine_exportable_resources(AREA_CONTEXT *ctx);
char **determine_needed_resources(AREA_CONTEXT *ctx);
AREA_DATA **find_trade_partners(AREA_CONTEXT *ctx);

/* Historical analysis */
char *extract_area_lore(AREA_DATA *area);
char **parse_major_events(char *area_description);
int estimate_area_age(AREA_DATA *area);

/* Neighbor detection */
void find_neighbor_areas(AREA_CONTEXT *ctx);
int calculate_area_distance(AREA_DATA *area1, AREA_DATA *area2);
bool areas_are_connected(AREA_DATA *area1, AREA_DATA *area2);

/* Congruence checking - THE CRITICAL FUNCTION */
CONGRUENCE_CHECK *check_congruence(char *what_to_create, AREA_DATA *where, char *reason);
bool validate_geography_congruence(char *what, AREA_CONTEXT *where);
bool validate_political_congruence(char *what, AREA_CONTEXT *where);
bool validate_economic_congruence(char *what, AREA_CONTEXT *where);
bool validate_cultural_congruence(char *what, AREA_CONTEXT *where);
bool validate_lore_congruence(char *what, AREA_CONTEXT *where);
char *ai_congruence_analysis(char *what, AREA_CONTEXT *where, WORLD_CONTEXT *world);

/* Context-aware suggestions */
char **suggest_appropriate_creations(AREA_CONTEXT *ctx);
char *suggest_resource_distribution(AREA_CONTEXT *ctx);
AREA_DATA *suggest_best_location_for(char *what_to_create);

/* EXAMPLES - Congruence Checking in Action */
#if 0  /* Examples disabled for compilation */

/* Example 1: GOOD - Fishing in coastal city */
CONGRUENCE_CHECK *example_fishing_in_darkhaven(void)
{
    AREA_DATA *darkhaven = find_area_by_name("Darkhaven");
    CONGRUENCE_CHECK *result = check_congruence("fishing industry", darkhaven, "need fish resource");

    /* Result:
     * is_congruent = TRUE
     * confidence = 95
     * geography_fits = TRUE (has docks, coastal)
     * politics_fit = TRUE (King can order construction)
     * economy_fits = TRUE (city needs food source)
     * culture_fits = TRUE (medieval city, fishing is appropriate)
     * lore_fits = TRUE (area description mentions harbor)
     * issues = []
     * suggestions = ["Create fishing district near docks", "Add fisherman's guild"]
     * ai_analysis = "Darkhaven is described as a major port city with extensive
     *                docks. The area includes coastal rooms and harbor facilities.
     *                Fishing is highly congruent with the area's geography and
     *                medieval setting. Recommend creating fishing district adjacent
     *                to existing dock area."
     */

    return result;
}

/* Example 2: BAD - Fishing in desert */
CONGRUENCE_CHECK *example_fishing_in_desert(void)
{
    AREA_DATA *desert = find_area_by_name("Scorching Desert");
    CONGRUENCE_CHECK *result = check_congruence("fishing industry", desert, "need fish resource");

    /* Result:
     * is_congruent = FALSE
     * confidence = 98
     * geography_fits = FALSE (no water)
     * politics_fit = TRUE (doesn't matter if geography is wrong)
     * economy_fits = FALSE (desert can't support fishing)
     * culture_fits = MAYBE (depends on culture)
     * lore_fits = FALSE (desert lore mentions sand, not water)
     * issues = ["No water source for fishing", "Desert terrain incompatible",
     *           "Lore describes arid wasteland"]
     * suggestions = ["Consider oasis fishing instead", "Create in coastal area",
     *                "Add desert-appropriate food source (cactus, lizards)"]
     * ai_analysis = "The Scorching Desert is an arid wasteland with no natural
     *                water sources. Fishing is completely incongruent with this
     *                geography. However, if fish resources are needed, consider:
     *                1) Creating small oasis with limited fishing, or
     *                2) Establishing trade route with coastal city for fish import,
     *                or 3) Using desert-appropriate food sources instead."
     */

    return result;
}

/* Example 3: COMPLEX - Mining village near capital */
CONGRUENCE_CHECK *example_mining_village_near_darkhaven(void)
{
    AREA_DATA *near_dh = find_nearest_mountainous_area_to(find_area_by_name("Darkhaven"));
    CONGRUENCE_CHECK *result = check_congruence("mining village with DarkHaven guards",
                                                  near_dh, "provide ore to capital");

    /* Result:
     * is_congruent = TRUE
     * confidence = 92
     * geography_fits = TRUE (mountains have ore)
     * politics_fit = TRUE (in DarkHaven's sphere of influence)
     * economy_fits = TRUE (DarkHaven needs ore, village provides it)
     * culture_fits = TRUE (medieval mining settlement)
     * lore_fits = TRUE (area described as mountainous region)
     * issues = []
     * suggestions = [
     *     "Guards should be FROM DarkHaven (controlled by King Aldric)",
     *     "Create trade route between village and DarkHaven",
     *     "Mine should produce iron/copper (weapons for DarkHaven)",
     *     "Village name should reflect DarkHaven influence ('King's Mine')",
     *     "Peasants should mention King Aldric in dialogue"
     * ]
     * ai_analysis = "This is highly congruent. The mountainous area 15 rooms
     *                north of DarkHaven is within the capital's sphere of influence.
     *                King Aldric would realistically claim these mines for resource
     *                extraction. Recommend:
     *                - Village population: 20-30 miners
     *                - Guard contingent: 5-8 DarkHaven soldiers (vnum 1000 series)
     *                - Production: Iron ore, coal
     *                - Trade route: Daily caravans to DarkHaven
     *                - Governance: Village Elder who reports to King Aldric
     *                - Cultural markers: DarkHaven banners, mention king in dialogue"
     */

    return result;
}

/* Example 4: Political sphere of influence */
void example_sphere_of_influence_check(void)
{
    AREA_DATA *darkhaven = find_area_by_name("Darkhaven");
    AREA_DATA *distant_village = find_area_by_name("Far Village");

    int distance = calculate_sphere_of_influence(darkhaven, distant_village);
    /* distance = 150 rooms */

    /* Determine if DarkHaven controls it */
    bool in_sphere = area_in_sphere_of(darkhaven, distant_village, 50);
    /* in_sphere = FALSE (too far, 150 > 50) */

    /* AI analysis */
    char *analysis = ai_congruence_analysis("DarkHaven guards in Far Village",
                                             analyze_area(distant_village),
                                             get_world_context());
    /* analysis = "Far Village is 150 rooms from DarkHaven, well beyond the
     *             capital's typical sphere of influence. DarkHaven guards stationed
     *             here would be incongruent unless:
     *             1) There's a special treaty/alliance
     *             2) The village pays tribute to DarkHaven
     *             3) Recent military conquest
     *             Without such context, guards should be local militia instead." */
}

/* Example 5: Resource distribution based on geography */
void example_intelligent_resource_distribution(void)
{
    AREA_DATA *area = find_area_by_name("Mountain Pass");
    AREA_CONTEXT *ctx = analyze_area(area);

    /* Geography: Mountains, high elevation, no water */
    /* Suggested resources: ore, coal, gems, stone */

    char **suggested = suggest_resource_distribution(ctx);
    /* suggested = ["iron_ore", "coal", "gems", "stone", "mountain_herbs"] */

    /* BAD resources for this area: fish, crops, lumber */

    /* Example announcement (if Beeler adds resources) */
    smart_announce(
        "Mining Opportunities Discovered in Mountain Pass",
        "Survey teams have identified rich ore deposits in the Mountain Pass region. "
        "Iron and coal are now available for extraction.",
        EVENT_CATEGORY_DISCOVERY,
        ANNOUNCE_PRIORITY_MEDIUM,
        "Mountain Pass"
    );
}

/* Integration with organic creation */
bool validate_before_organic_creation(ORGANIC_CREATION_REQUEST *req)
{
    CONGRUENCE_CHECK *check = check_congruence(req->what_to_create,
                                                 req->target_area,
                                                 req->reason);

    if (!check->is_congruent)
    {
        /* Reject creation - doesn't make sense */
        log_string("WORLD CONTEXT: Rejected creation - incongruent");
        log_string(check->ai_analysis);
        return FALSE;
    }

    if (check->confidence < 70)
    {
        /* Questionable - log for review */
        log_string("WORLD CONTEXT: Low confidence creation");
        log_string(check->ai_analysis);
    }

    return TRUE;
}

#endif /* Examples disabled */

/* Commands */
void do_worldcontext(CHAR_DATA *ch, char *argument);  /* Show world analysis */
void do_areacontext(CHAR_DATA *ch, char *argument);   /* Show specific area analysis */
void do_congruence(CHAR_DATA *ch, char *argument);    /* Test congruence of an idea */

#endif /* WORLD_CONTEXT_H */
