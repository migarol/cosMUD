/*****************************************************************************
 * Leader AI System - Intelligent Area Leadership
 *
 * Not all areas need leaders:
 * - Darkhaven = King (major city)
 * - Small village = Mayor (town)
 * - Orcish camp = Chieftain (tribal)
 * - Ancient forest = NO LEADER (wilderness)
 * - Dungeon = NO LEADER (not a settlement)
 *
 * Leaders make strategic decisions for their domains:
 * - BUILD infrastructure
 * - TRADE with other areas
 * - RECRUIT guards/workers
 * - DECLARE WAR
 * - DIPLOMACY with other leaders
 *
 * "A king without a kingdom is just a man with a crown."
 *****************************************************************************/

#ifndef LEADER_AI_H
#define LEADER_AI_H

/* Leader types */
#define LEADER_TYPE_KING        0  /* Major city ruler */
#define LEADER_TYPE_QUEEN       1  /* Major city ruler */
#define LEADER_TYPE_MAYOR       2  /* Town/village leader */
#define LEADER_TYPE_CHIEFTAIN   3  /* Tribal leader */
#define LEADER_TYPE_ARCHMAGE    4  /* Magic academy head */
#define LEADER_TYPE_GUILDMASTER 5  /* Thieves guild, etc */
#define LEADER_TYPE_ELDER       6  /* Village elder */
#define LEADER_TYPE_WARLORD     7  /* Military dictator */

/* Area types (determines if leader is appropriate) */
#define AREA_TYPE_MAJOR_CITY    0  /* Darkhaven - needs King/Queen */
#define AREA_TYPE_TOWN          1  /* Smaller city - needs Mayor */
#define AREA_TYPE_VILLAGE       2  /* Small settlement - needs Elder/Mayor */
#define AREA_TYPE_TRIBAL        3  /* Orc camp - needs Chieftain */
#define AREA_TYPE_ACADEMY       4  /* Magic school - needs Archmage */
#define AREA_TYPE_GUILD         5  /* Thieves guild - needs Guildmaster */
#define AREA_TYPE_WILDERNESS    6  /* Forest/mountains - NO LEADER */
#define AREA_TYPE_DUNGEON       7  /* Dungeon - NO LEADER */
#define AREA_TYPE_RUINS         8  /* Abandoned - NO LEADER */

/* Personality types */
#define PERSONALITY_AGGRESSIVE  0  /* War-focused, expansionist */
#define PERSONALITY_DIPLOMATIC  1  /* Trade-focused, peaceful */
#define PERSONALITY_GREEDY      2  /* Gold-focused, mercantile */
#define PERSONALITY_WISE        3  /* Culture-focused, balanced */
#define PERSONALITY_PARANOID    4  /* Defense-focused, isolationist */
#define PERSONALITY_CHAOTIC     5  /* Unpredictable, volatile */

/* Strategic actions */
#define ACTION_BUILD            0  /* Construct building/district */
#define ACTION_TRADE            1  /* Establish trade route */
#define ACTION_RECRUIT          2  /* Hire guards/workers */
#define ACTION_DECLARE_WAR      3  /* Start conflict */
#define ACTION_DIPLOMACY        4  /* Alliance/treaty */
#define ACTION_EXPAND           5  /* Claim new territory */
#define ACTION_RESEARCH         6  /* Develop new tech/magic */
#define ACTION_NOTHING          7  /* Wait and observe */

/* Relationship levels */
#define RELATION_ALLIED         3
#define RELATION_FRIENDLY       2
#define RELATION_NEUTRAL        1
#define RELATION_UNFRIENDLY     0
#define RELATION_HOSTILE       -1
#define RELATION_AT_WAR        -2

/* Leader data */
typedef struct leader_ai_data {
    CHAR_DATA *mob;              /* The actual NPC */
    int leader_type;
    int personality;

    /* Domain */
    AREA_DATA *controlled_area;
    char *title;                 /* "King of Darkhaven" */
    char *formal_name;           /* "King Aldric the Just" */

    /* Resources */
    int treasury;                /* Gold available */
    int military_power;          /* Number of guards */
    int economic_power;          /* Trade income per day */
    int population;              /* Citizens */

    /* Capabilities */
    bool can_build;
    bool can_trade;
    bool can_declare_war;
    bool can_recruit;

    /* Relationships with other leaders */
    int num_relations;
    struct leader_relation {
        struct leader_ai_data *other_leader;
        int relationship_level;
        bool has_trade_route;
        bool at_war;
    } *relations;

    /* Current plans */
    int num_active_plans;
    struct strategic_plan {
        int action_type;
        char *action_description;
        int turns_to_complete;
        int cost;
        bool announced;
    } *active_plans;

    /* AI decision-making */
    time_t last_decision_time;
    int decision_interval;       /* How often to make decisions (seconds) */
    char *last_decision;         /* For debugging */

    /* History */
    int wars_started;
    int buildings_built;
    int trade_routes_established;
    int years_in_power;

    struct leader_ai_data *next;

} LEADER_AI_DATA;

/* Area classification result */
typedef struct area_classification {
    AREA_DATA *area;
    int area_type;               /* AREA_TYPE_* */
    bool needs_leader;
    int suggested_leader_type;   /* LEADER_TYPE_* */

    /* Detection reasoning */
    char *classification_reason;
    int population_estimate;
    int building_count;
    bool has_shops;
    bool has_government_buildings;

} AREA_CLASSIFICATION;

/* Function declarations */

/* Initialization */
void init_leader_ai(void);
void load_leader_data(void);
void save_leader_data(void);

/* Area analysis - Smart detection of which areas need leaders */
AREA_CLASSIFICATION *classify_area(AREA_DATA *area);
bool area_needs_leader(AREA_DATA *area);
int detect_appropriate_leader_type(AREA_DATA *area);
char *ai_analyze_area_type(AREA_DATA *area);  /* Ollama-powered classification */

/* Leader detection/creation */
LEADER_AI_DATA *find_leader_in_area(AREA_DATA *area);
LEADER_AI_DATA *create_leader_for_area(AREA_DATA *area, int leader_type);
CHAR_DATA *find_or_create_leader_mob(AREA_DATA *area, int leader_type);
void assign_leader_ai(CHAR_DATA *mob, int leader_type, int personality);

/* World scan - Find all leaders at boot */
void scan_world_for_leaders(void);
void detect_missing_leaders(void);  /* Find areas that should have leaders but don't */

/* Decision-making */
void leader_think(LEADER_AI_DATA *leader);
int leader_choose_action(LEADER_AI_DATA *leader);
char *ai_generate_leader_decision(LEADER_AI_DATA *leader);  /* Ollama-powered */
void leader_execute_action(LEADER_AI_DATA *leader, int action, char *details);

/* Specific actions */
bool leader_build(LEADER_AI_DATA *leader, char *what_to_build);
bool leader_establish_trade(LEADER_AI_DATA *leader, LEADER_AI_DATA *other);
bool leader_recruit(LEADER_AI_DATA *leader, char *recruit_type, int count);
bool leader_declare_war(LEADER_AI_DATA *leader, LEADER_AI_DATA *target);
bool leader_make_peace(LEADER_AI_DATA *leader, LEADER_AI_DATA *other);
bool leader_form_alliance(LEADER_AI_DATA *leader, LEADER_AI_DATA *other);

/* Relationships */
void update_leader_relationship(LEADER_AI_DATA *leader1, LEADER_AI_DATA *leader2, int change);
int get_relationship_level(LEADER_AI_DATA *leader1, LEADER_AI_DATA *leader2);
bool are_leaders_at_war(LEADER_AI_DATA *leader1, LEADER_AI_DATA *leader2);

/* Context for decisions */
char *build_leader_context(LEADER_AI_DATA *leader);
char *analyze_leader_neighbors(LEADER_AI_DATA *leader);
char *analyze_leader_threats(LEADER_AI_DATA *leader);
char *analyze_leader_opportunities(LEADER_AI_DATA *leader);

/* Integration with Beeler */
void beeler_execute_leader_command(LEADER_AI_DATA *leader, char *command);
bool beeler_can_fulfill_leader_request(LEADER_AI_DATA *leader, char *request);

/* Update loop - Called from update.c */
void leader_ai_update(void);  /* Called every game tick */

/* EXAMPLE IMPLEMENTATIONS */

/* Example 1: Detecting area types */
AREA_CLASSIFICATION *example_classify_darkhaven(void)
{
    AREA_DATA *area = find_area_by_name("Darkhaven");
    AREA_CLASSIFICATION *result = classify_area(area);

    /* Result:
     * area_type = AREA_TYPE_MAJOR_CITY
     * needs_leader = TRUE
     * suggested_leader_type = LEADER_TYPE_KING
     * classification_reason = "Large city with palace, shops, government buildings,
     *                          high population, capital-like infrastructure"
     */

    return result;
}

/* Example 2: Wilderness shouldn't have leaders */
AREA_CLASSIFICATION *example_classify_dark_forest(void)
{
    AREA_DATA *area = find_area_by_name("Dark Forest");
    AREA_CLASSIFICATION *result = classify_area(area);

    /* Result:
     * area_type = AREA_TYPE_WILDERNESS
     * needs_leader = FALSE
     * suggested_leader_type = -1 (none)
     * classification_reason = "Natural wilderness area, no settlements,
     *                          no buildings, only monsters and animals"
     */

    return result;
}

/* Example 3: King makes a decision */
void example_king_aldric_decides(void)
{
    LEADER_AI_DATA *aldric = find_leader_in_area(find_area_by_name("Darkhaven"));

    /* Build context for AI */
    char *context = build_leader_context(aldric);
    /* context includes:
     * - Treasury: 50000 gold
     * - Military: 150 guards
     * - Relations: Allied with Midgaard, Neutral to New Thalos
     * - Recent events: None
     * - Threats: Orc raids from north
     * - Opportunities: Mining village nearby
     */

    /* AI generates decision */
    char *decision = ai_generate_leader_decision(aldric);
    /* Example decision: "BUILD fortress at northern border to defend against orc raids" */

    /* Execute */
    leader_execute_action(aldric, ACTION_BUILD, "northern fortress");

    /* Announce (via periodicos) */
    smart_announce(
        "King Aldric Orders Construction of Northern Fortress",
        "In response to increasing orc activity, King Aldric has ordered "
        "the construction of a military fortress at Darkhaven's northern border.",
        EVENT_CATEGORY_CONSTRUCTION,
        ANNOUNCE_PRIORITY_HIGH,
        "Darkhaven"
    );

    /* Delegate to Beeler */
    beeler_execute_leader_command(aldric,
        "build fortress area=darkhaven type=military location=north rooms=10"
    );
}

/* Example 4: Chieftain declares war */
void example_chieftain_declares_war(void)
{
    LEADER_AI_DATA *grok = find_leader_in_area(find_area_by_name("Orc Camp"));

    /* Grok is aggressive personality, hates nearby village */
    LEADER_AI_DATA *village_elder = find_leader_in_area(find_area_by_name("Farming Village"));

    int relation = get_relationship_level(grok, village_elder);
    /* relation = RELATION_HOSTILE (already hate each other) */

    /* Grok decides to attack */
    leader_declare_war(grok, village_elder);

    /* Announce (CRITICAL - goes to chat) */
    smart_announce(
        "Chieftain Grok Declares War on Farming Village!",
        "Chieftain Grok of the Orc Camp has declared war on the nearby Farming Village. "
        "Raiders have been spotted massing at the border.",
        EVENT_CATEGORY_WAR,
        ANNOUNCE_PRIORITY_CRITICAL,
        "Orc Camp"
    );
}

/* Example 5: Scanning world at boot */
void example_boot_time_leader_scan(void)
{
    /* Called from boot_db() */
    scan_world_for_leaders();

    /* This will:
     * 1. Scan all areas
     * 2. Classify each (city/town/wilderness/dungeon)
     * 3. Check if area needs leader
     * 4. Look for existing leader mob (king/mayor/chieftain)
     * 5. If area needs leader but doesn't have one, create one
     * 6. Assign AI to all found leaders
     */

    /* Results:
     * - Darkhaven: Found King Aldric (vnum 1000), assigned KING AI
     * - New Thalos: Found Mayor (vnum 2500), assigned MAYOR AI
     * - Orc Camp: Found Chieftain Grok (vnum 3200), assigned CHIEFTAIN AI
     * - Dark Forest: Wilderness - no leader needed
     * - Dragon's Lair: Dungeon - no leader needed
     * - Farming Village: MISSING LEADER - created Elder (vnum auto)
     */
}

/* Example 6: Leader establishes trade */
void example_trade_route(void)
{
    LEADER_AI_DATA *aldric = find_leader_in_area(find_area_by_name("Darkhaven"));
    LEADER_AI_DATA *mayor = find_leader_in_area(find_area_by_name("Midgaard"));

    /* Check if trade makes sense */
    char *trade_analysis = analyze_leader_opportunities(aldric);
    /* trade_analysis: "Midgaard has surplus grain, Darkhaven needs food,
     *                  Darkhaven has weapons, Midgaard needs defense" */

    /* Establish trade */
    leader_establish_trade(aldric, mayor);

    /* Update economy */
    aldric->economic_power += 100;  /* +100 gold per day */
    mayor->economic_power += 100;

    /* Announce (MEDIUM - periodicos only) */
    smart_announce(
        "Trade Route Established Between Darkhaven and Midgaard",
        "King Aldric and Mayor of Midgaard have formalized a trade agreement. "
        "Merchants now travel regularly between the two cities.",
        EVENT_CATEGORY_TRADE,
        ANNOUNCE_PRIORITY_MEDIUM,
        "Darkhaven"
    );
}

/* Commands */
void do_leaders(CHAR_DATA *ch, char *argument);     /* List all leaders and their status */
void do_leaderinfo(CHAR_DATA *ch, char *argument);  /* Detailed info on a leader */

#endif /* LEADER_AI_H */
