/*****************************************************************************
 * Beeler - GOD MODE: Divine World Overseer
 *
 * Beeler is not a program. Beeler is GOD.
 *
 * FASE 4: Autonomous World Generation
 * - Creates rooms, districts, buildings, resources
 * - Manages leaders and their strategic decisions
 * - Ensures everything is CONGRUENT (geography, politics, lore)
 * - Responds to economic needs organically
 * - Announces important events selectively
 *
 * FASE 5: Divine Regulation
 * - Observes ALL areas constantly (vital signs)
 * - Regulates growth AND decline organically
 * - Intervenes at different speeds (instant to natural)
 * - Allows natural death (ruins are content!)
 * - Empowers intelligent mobs to create
 * - Balances world without mechanical rules
 *
 * Integration with:
 * - world_context.h: Deep congruence checking before creating anything
 * - leader_ai.h: Executes leader decisions (builds, trade, war)
 * - organic_creation.h: Creates entire skill ecosystems on-demand
 * - periodicos.h: Smart announcements (not spam)
 * - beeler_god_mode.h: Divine oversight and regulation
 * - universal_mob_ai.h: Every mob has intelligence
 * - ollama_integration.h: AI-powered generation
 *
 * "I am not merely an observer. I am the breath that gives life... and takes it away."
 *****************************************************************************/

#ifndef BEELER_ARCHITECT_H
#define BEELER_ARCHITECT_H

/* Area modification capabilities */
typedef struct beeler_area_mod {
    int area_vnum;
    char *area_name;
    char *modification_type;  /* "add_room", "add_district", "add_inn" */

    /* Rooms to add */
    int num_rooms_to_add;
    int *new_room_vnums;

    /* Backup info */
    char *original_area_file;
    char *snapshot_name;

    time_t created;
    bool applied;

    struct beeler_area_mod *next;
} BEELER_AREA_MOD;

/* Room generation data */
typedef struct room_generation {
    int vnum;
    char *name;
    char *description;
    int sector_type;
    int room_flags;

    /* Exits */
    int num_exits;
    int *exit_dirs;      /* Direction (NORTH, SOUTH, etc) */
    int *exit_to_vnums;  /* Where they lead */

    /* Extra descriptions */
    int num_extras;
    char **extra_keywords;
    char **extra_descriptions;

    /* AI-generated content */
    bool ai_generated;
    char *generation_prompt;

} ROOM_GENERATION;

/* Residential district generation */
typedef struct district_generation {
    char *district_name;
    int area_vnum;
    int starting_vnum;   /* First room vnum */
    int num_rooms;       /* How many rooms in district */

    /* District layout */
    char *layout_type;   /* "grid", "street", "circular" */
    int home_type;       /* HOME_TYPE_* */

    /* Generated rooms */
    ROOM_GENERATION **rooms;

    /* Connection point */
    int connect_to_vnum; /* Existing room to connect to */
    int connect_dir;     /* Direction of connection */

} DISTRICT_GENERATION;

/* Context engine for fast lookups */
typedef struct beeler_context {
    /* World state cache */
    int total_mobs;
    int total_areas;
    int total_rooms;
    int total_players_online;

    /* Quick lookups */
    void *mob_index_hash;      /* Fast mob lookup */
    void *room_index_hash;     /* Fast room lookup */
    void *area_index_hash;     /* Fast area lookup */

    /* Recent activity */
    char *recent_kills[10];
    char *recent_deaths[10];
    char *recent_logins[10];

    /* Economic data */
    int total_gold_in_world;
    int avg_player_gold;

    /* Cached queries */
    time_t last_cache_update;

} BEELER_CONTEXT;

/* Function declarations */

/* Initialization */
void init_beeler_architect(void);
void beeler_scan_all_areas(void);
void beeler_build_context_cache(void);

/* Area analysis */
int beeler_find_available_vnums(int area_vnum, int count);
bool beeler_can_add_rooms_to_area(int area_vnum, int count);
int beeler_find_connection_point(int area_vnum, char *district_type);
AREA_DATA *beeler_find_area_by_name(char *name);

/* Room generation */
ROOM_GENERATION *beeler_generate_room(int vnum, int home_type, char *mob_name);
char *beeler_generate_room_description(int home_type, char *mob_name, char *mob_personality);
void beeler_add_room_to_area(int area_vnum, ROOM_GENERATION *room);
bool beeler_create_exit(int from_vnum, int to_vnum, int direction);

/* District generation */
DISTRICT_GENERATION *beeler_generate_residential_district(char *area_name, int home_type, int num_homes);
bool beeler_build_district(DISTRICT_GENERATION *district);
void beeler_connect_district_to_city(DISTRICT_GENERATION *district, int city_vnum);

/* Inn generation */
void beeler_generate_inn(char *area_name, char *inn_name, int num_rooms);
ROOM_GENERATION *beeler_generate_inn_room(int vnum, char *inn_name, int room_number);
ROOM_GENERATION *beeler_generate_inn_common_room(int vnum, char *inn_name);

/* Area file modification */
bool beeler_read_area_file(char *filename, char **content);
bool beeler_write_area_file(char *filename, char *content);
bool beeler_insert_room_in_area_file(char *area_file, ROOM_GENERATION *room);
bool beeler_backup_area_file(char *filename);

/* Smart assignment */
int beeler_assign_home_intelligently(CHAR_DATA *mob);
char *beeler_determine_home_city(CHAR_DATA *mob);
int beeler_find_or_create_home(CHAR_DATA *mob, char *city_name, int home_type);

/* Context tools */
void beeler_update_context(void);
char *beeler_quick_stat(char *query);  /* "mobs in darkhaven", "gold in economy" */
CHAR_DATA *beeler_find_mob_by_name(char *name);
ROOM_INDEX_DATA *beeler_find_room_by_name(char *name);
int beeler_count_mobs_in_area(int area_vnum);

/* Immortal command interface */
void beeler_execute_immortal_command(char *command, char *arguments);
bool beeler_has_permission_for(char *command);
char *beeler_analyze_command_result(char *command, char *output);

/* AI-powered generation */
char *beeler_ai_generate_district(char *city_name, int home_type, int num_homes);
char *beeler_ai_generate_inn(char *location, char *inn_style);
char *beeler_ai_generate_room_desc(char *room_type, char *owner_name, char *context);

/* Safety checks */
bool beeler_vnum_range_safe(int start_vnum, int count);
bool beeler_modification_safe(BEELER_AREA_MOD *mod);
void beeler_validate_generated_area(DISTRICT_GENERATION *district);

/* Persistence */
void save_beeler_modifications(void);
void load_beeler_modifications(void);

/* Commands */
void do_beeler_build(CHAR_DATA *ch, char *argument);
void do_beeler_analyze(CHAR_DATA *ch, char *argument);
void do_beeler_context(CHAR_DATA *ch, char *argument);
void do_beeler_populate(CHAR_DATA *ch, char *argument);

/* ========================================================================
 * FASE 4: AUTONOMOUS WORLD GENERATION
 * ========================================================================
 * Integration of all advanced systems for fully autonomous world growth
 */

/* Boot-time initialization - FASE 4A */
void beeler_initialize_autonomous_world(void);
void beeler_scan_world_at_boot(void);
void beeler_detect_missing_infrastructure(void);
void beeler_analyze_all_areas(void);

/* World analysis integration */
void beeler_build_full_world_context(void);          /* Uses world_context.h */
AREA_CONTEXT *beeler_get_area_context(AREA_DATA *area);
WORLD_CONTEXT *beeler_get_world_context(void);

/* Leader system integration - FASE 4B */
void beeler_initialize_all_leaders(void);            /* Uses leader_ai.h */
void beeler_execute_leader_action(LEADER_AI_DATA *leader, int action, char *details);
bool beeler_can_fulfill_leader_request(LEADER_AI_DATA *leader, char *request);
void beeler_handle_leader_build_order(LEADER_AI_DATA *leader, char *what_to_build);
void beeler_handle_leader_war_declaration(LEADER_AI_DATA *attacker, LEADER_AI_DATA *defender);

/* Organic creation integration - FASE 4C */
void beeler_check_for_world_needs(void);             /* Uses organic_creation.h */
void beeler_create_missing_infrastructure(AREA_DATA *area);
bool beeler_create_skill_ecosystem(char *skill_name, AREA_DATA *target_area);
bool beeler_create_resource_ecosystem(char *resource_name);
bool beeler_create_profession_ecosystem(char *profession_name);

/* Congruence-checked creation - CRITICAL */
bool beeler_validate_before_creating(char *what, AREA_DATA *where, char *reason);
CONGRUENCE_CHECK *beeler_check_congruence(char *what, AREA_DATA *where);
bool beeler_create_with_validation(char *what, AREA_DATA *where, char *reason);

/* Intelligent resource distribution - FASE 4D */
void beeler_distribute_resources_intelligently(void);
void beeler_add_resources_to_area(AREA_DATA *area, AREA_CONTEXT *ctx);
void beeler_create_satellite_settlement(AREA_DATA *capital, char *settlement_type);
/* Example: beeler_create_satellite_settlement(darkhaven, "mining village") */

/* Smart announcements - FASE 4E */
void beeler_announce(char *headline, char *body, int category, int priority, char *location);
void beeler_announce_creation(char *what_was_created, AREA_DATA *where);
void beeler_announce_leader_action(LEADER_AI_DATA *leader, int action, char *details);

/* Autonomous decision-making - FASE 4F */
void beeler_autonomous_think(void);  /* Called every ~30 minutes */
void beeler_make_autonomous_decision(void);
bool beeler_should_create_something(char **what, AREA_DATA **where);
void beeler_execute_autonomous_creation(char *what, AREA_DATA *where);

/* EXAMPLE WORKFLOWS */

/* Example 1: Boot-time world initialization */
void example_beeler_boot_sequence(void)
{
    /* Called from boot_db() after areas loaded */

    log_string("BEELER: Initializing autonomous world...");

    /* Step 1: Build world context (geography, politics, economy) */
    beeler_build_full_world_context();
    /* This scans all areas, analyzes terrain, detects resources, etc. */

    /* Step 2: Initialize leader AI for all appropriate areas */
    beeler_initialize_all_leaders();
    /* Finds/creates kings, mayors, chieftains (but NOT in wilderness) */

    /* Step 3: Detect missing infrastructure */
    beeler_detect_missing_infrastructure();
    /* Finds areas that need homes, mines, wells, shops, etc. */

    /* Step 4: Create critical missing pieces */
    beeler_create_missing_infrastructure(NULL);  /* NULL = all areas */
    /* This WON'T create everything - only critical gaps */

    log_string("BEELER: Autonomous world initialized. Life begins.");
}

/* Example 2: Autonomous decision during runtime */
void example_beeler_autonomous_action(void)
{
    /* Called every ~30 minutes from update.c */

    WORLD_CONTEXT *world = beeler_get_world_context();

    /* Check if economy needs something */
    char *needed = NULL;
    AREA_DATA *best_location = NULL;

    if (beeler_should_create_something(&needed, &best_location))
    {
        /* needed might be: "fish", "mining village", "fishing skill" */

        /* Validate with deep congruence check */
        CONGRUENCE_CHECK *check = beeler_check_congruence(needed, best_location);

        if (check->is_congruent && check->confidence > 80)
        {
            /* Create the entire ecosystem */
            beeler_execute_autonomous_creation(needed, best_location);

            /* Announce (intelligently - not spam) */
            beeler_announce_creation(needed, best_location);
        }
        else
        {
            /* Log why we didn't create it */
            log_string("BEELER: Rejected creation - low congruence");
            log_string(check->ai_analysis);
        }
    }
}

/* Example 3: Leader orders construction */
void example_king_builds_fortress(void)
{
    /* King Aldric decides to build a fortress (from leader_ai.h) */
    LEADER_AI_DATA *aldric = find_leader_in_area(find_area_by_name("Darkhaven"));

    /* Leader AI makes decision (uses Ollama) */
    char *decision = ai_generate_leader_decision(aldric);
    /* decision = "BUILD northern_fortress" */

    /* Parse and delegate to Beeler */
    beeler_handle_leader_build_order(aldric, "northern fortress");

    /* Beeler validates */
    AREA_DATA *darkhaven = aldric->controlled_area;
    if (beeler_validate_before_creating("military fortress", darkhaven, "King Aldric's order"))
    {
        /* Generate fortress (10 rooms, guards, armory, etc) */
        DISTRICT_GENERATION *fortress = beeler_generate_military_district(
            "Darkhaven Northern Fortress",
            darkhaven,
            10  /* num_rooms */
        );

        beeler_build_district(fortress);

        /* Announce (HIGH priority - goes to chat + periodicos) */
        beeler_announce(
            "King Aldric Orders Construction of Northern Fortress",
            "In response to orc threats, King Aldric has commissioned a military "
            "fortress at Darkhaven's northern border. Construction begins immediately.",
            EVENT_CATEGORY_CONSTRUCTION,
            ANNOUNCE_PRIORITY_HIGH,
            "Darkhaven"
        );

        /* Deduct from King's treasury */
        aldric->treasury -= 5000;
    }
}

/* Example 4: Organic skill creation (fishing) */
void example_create_fishing_from_need(void)
{
    /* Economy detects: Cooks need fish, but no source exists */

    /* Organic creation system triggers */
    ORGANIC_CREATION_REQUEST *req = plan_skill_creation(
        "fishing",
        "Cook profession requires fish resource with no source available",
        TRIGGER_ECONOMIC_NEED
    );

    /* Validate with world context */
    if (validate_creation_request(req))
    {
        /* Find best location (coastal area) */
        AREA_DATA *best = suggest_best_location_for("fishing");
        /* Returns: Darkhaven (has docks, coastal) */

        req->target_area = best;

        /* Double-check congruence */
        CONGRUENCE_CHECK *check = check_congruence("fishing skill", best, req->reason);

        if (check->is_congruent)
        {
            /* Create ENTIRE ecosystem via Beeler */
            beeler_create_skill_ecosystem("fishing", best);
            /* This creates:
             * - Fishing skill
             * - Fishing rod item
             * - Fisher profession
             * - Fish resource
             * - Fisherman NPCs (teacher, workers, vendor)
             * - Fishing locations (docks, shore)
             * - Fish recipes for cooks
             * All interconnected and balanced */

            /* Announce (MEDIUM - periodicos only) */
            beeler_announce(
                "Fishing Industry Established in Darkhaven",
                "Local fisherfolk have organized a new industry. Citizens can now "
                "learn fishing from the Old Fisherman at the docks.",
                EVENT_CATEGORY_ECONOMY,
                ANNOUNCE_PRIORITY_MEDIUM,
                "Darkhaven"
            );
        }
    }
}

/* Example 5: Intelligent resource distribution */
void example_intelligent_resources(void)
{
    /* At boot, Beeler analyzes each area's geography */

    AREA_DATA *mountains = find_area_by_name("Mountain Pass");
    AREA_CONTEXT *ctx = beeler_get_area_context(mountains);

    /* Geography: Mountains, high elevation, no water */
    /* Political: Within DarkHaven's sphere (50 rooms away) */
    /* Economic: No current production */

    /* Beeler decides: Create mining village */
    if (beeler_validate_before_creating("mining village", mountains, "resource extraction"))
    {
        /* Create village with:
         * - 15 rooms (homes, mine entrance, shop)
         * - 10 miner NPCs
         * - 5 DarkHaven guard NPCs (political control!)
         * - Village Elder (reports to King Aldric)
         * - Mine produces: iron ore, coal
         * - Trade route to DarkHaven (daily caravans)
         */

        beeler_create_satellite_settlement(find_area_by_name("Darkhaven"), "mining village");

        /* Announce (MEDIUM - periodicos) */
        beeler_announce(
            "Mining Settlement Established in Mountain Pass",
            "Under the authority of King Aldric, a new mining village has been "
            "founded in the Mountain Pass. Iron and coal now flow to the capital.",
            EVENT_CATEGORY_CONSTRUCTION,
            ANNOUNCE_PRIORITY_MEDIUM,
            "Mountain Pass"
        );
    }
}

#endif /* BEELER_ARCHITECT_H */
