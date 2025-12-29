/*****************************************************************************
 * Beeler - GOD MODE: Organic World Regulation
 *
 * Beeler NO es un sistema automático. Es un DIOS que observa, piensa, decide:
 * - Regula crecimiento Y declive
 * - Crea rápido cuando necesario
 * - Ajusta balance constantemente
 * - Expande cuando hace sentido
 * - Cambia el mundo orgánicamente
 *
 * NO TODO CRECE. Depende de:
 * - Economía (prosperidad vs pobreza)
 * - Recursos (abundancia vs escasez)
 * - Política (paz vs guerra)
 * - Eventos (plagas, desastres, bonanzas)
 * - Player actions (ayuda vs destrucción)
 * - Historia (momentum vs estancamiento)
 *
 * "I am not a program. I am the breath that gives life... and takes it away."
 *****************************************************************************/

#ifndef BEELER_GOD_MODE_H
#define BEELER_GOD_MODE_H

/* ========================================================================
 * ÁREA VITAL SIGNS - Health of settlements
 * ======================================================================== */

/* Area health status */
#define AREA_STATUS_THRIVING      5  /* Growing rapidly */
#define AREA_STATUS_PROSPEROUS    4  /* Healthy growth */
#define AREA_STATUS_STABLE        3  /* Equilibrium */
#define AREA_STATUS_DECLINING     2  /* Losing population/resources */
#define AREA_STATUS_STRUGGLING    1  /* Near collapse */
#define AREA_STATUS_DYING         0  /* Abandonment imminent */
#define AREA_STATUS_RUINS        -1  /* Dead/abandoned */

/* Factors affecting area health */
typedef struct area_vital_signs {
    AREA_DATA *area;

    /* ECONOMIC HEALTH */
    int economic_score;          /* -100 to 100 */
    int trade_volume;            /* Gold per day */
    int employment_rate;         /* % of NPCs with professions */
    int resource_availability;   /* 0-100 */

    /* POPULATION */
    int population;              /* Total NPCs */
    int population_trend;        /* Growing(+) or shrinking(-) */
    int births_per_month;
    int deaths_per_month;
    int immigration;             /* NPCs moving IN */
    int emigration;              /* NPCs moving OUT */

    /* SECURITY */
    int safety_score;            /* 0-100 */
    int num_guards;
    int crime_rate;              /* Thefts, murders per month */
    int monster_attacks;         /* Raids per month */
    bool at_war;

    /* RESOURCES */
    int food_supply;             /* Days of food available */
    int water_access;            /* 0-100 */
    int housing_available;       /* % of population with homes */
    int infrastructure_quality;  /* 0-100 (roads, buildings) */

    /* CULTURAL */
    int happiness;               /* 0-100 (morale) */
    int cultural_activity;       /* Events, festivals, life */
    int education_level;         /* 0-100 */

    /* ENVIRONMENTAL */
    int natural_disasters;       /* Floods, fires, etc */
    int disease_outbreaks;       /* Plagues */
    int crop_yields;             /* % of normal (affected by weather) */

    /* OVERALL */
    int overall_health;          /* Calculated from all above */
    int status;                  /* AREA_STATUS_* */
    int growth_potential;        /* -100 to 100 */

    /* TRAJECTORY */
    bool is_growing;
    bool is_declining;
    bool is_stable;
    char *prognosis;             /* AI-generated future prediction */

} AREA_VITAL_SIGNS;

/* ========================================================================
 * BEELER'S DIVINE OVERSIGHT
 * ======================================================================== */

/* Beeler's world view */
typedef struct beeler_divine_oversight {
    /* Global observation */
    WORLD_CONTEXT *world_context;

    /* Health monitoring */
    int num_areas_monitored;
    AREA_VITAL_SIGNS **vital_signs;  /* All areas */

    /* Intervention triggers */
    int num_critical_situations;
    char **critical_alerts;           /* "Village X starving!", "War in Y" */

    /* Pending decisions */
    int num_pending_decisions;
    struct divine_decision {
        char *situation;              /* What Beeler observed */
        char *options[5];             /* Possible interventions */
        int chosen_option;            /* -1 = not decided yet */
        time_t decision_time;
    } *pending_decisions;

    /* Recent interventions */
    int interventions_last_day;
    int interventions_last_week;
    char **recent_actions;            /* Log of what Beeler did */

    /* Philosophy */
    int intervention_threshold;       /* How bad before Beeler acts */
    bool allow_natural_death;         /* Let areas die naturally? */
    bool prevent_all_wars;            /* Or allow conflict? */

} BEELER_DIVINE_OVERSIGHT;

/* ========================================================================
 * ORGANIC GROWTH & DECLINE
 * ======================================================================== */

/* Growth trajectory */
typedef struct area_growth {
    AREA_DATA *area;

    /* Current state */
    int current_size;            /* Rooms */
    int current_population;      /* NPCs */
    int current_wealth;          /* Total gold */

    /* Growth factors */
    int natural_growth_rate;     /* % per month (can be negative) */
    int growth_momentum;         /* Accelerating or slowing */

    /* Triggers for expansion */
    bool population_pressure;    /* Too crowded */
    bool economic_boom;          /* Lots of wealth */
    bool new_resources_found;    /* Discovered mine, etc */
    bool immigration_wave;       /* People fleeing elsewhere */

    /* Triggers for decline */
    bool resource_depletion;     /* Mine ran out */
    bool economic_collapse;      /* Trade routes cut */
    bool war_devastation;        /* Military destruction */
    bool plague;                 /* Disease outbreak */
    bool natural_disaster;       /* Earthquake, flood */
    bool mass_emigration;        /* People leaving */

    /* Beeler's decision */
    bool beeler_should_expand;   /* Grow? */
    bool beeler_should_shrink;   /* Decline? */
    bool beeler_should_intervene; /* Emergency help? */
    char *beeler_reasoning;      /* Why this decision */

} AREA_GROWTH;

/* ========================================================================
 * DYNAMIC AREA EVOLUTION
 * ======================================================================== */

/* Area lifecycle stages */
#define STAGE_OUTPOST       0  /* 5-10 rooms, few NPCs */
#define STAGE_HAMLET        1  /* 10-20 rooms */
#define STAGE_VILLAGE       2  /* 20-50 rooms */
#define STAGE_TOWN          3  /* 50-100 rooms */
#define STAGE_CITY          4  /* 100-200 rooms */
#define STAGE_METROPOLIS    5  /* 200+ rooms */
#define STAGE_RUINS         6  /* Abandoned/destroyed */

/* Evolution event */
typedef struct area_evolution {
    AREA_DATA *area;
    int current_stage;
    int previous_stage;

    /* Evolution trigger */
    char *trigger_reason;        /* Why it changed */
    time_t evolution_time;

    /* What changes */
    int rooms_added;             /* Or removed (negative) */
    int npcs_added;              /* Or removed */
    int buildings_added;         /* Markets, walls, etc */
    char *infrastructure_changes; /* "Added city walls, marketplace" */

    /* Beeler's plan */
    char *evolution_plan;        /* AI-generated expansion/decline plan */
    bool approved;

} AREA_EVOLUTION;

/* ========================================================================
 * BEELER'S INTERVENTION MODES
 * ======================================================================== */

/* Intervention types */
#define INTERVENTION_EMERGENCY   0  /* FAST - Village starving */
#define INTERVENTION_STRATEGIC   1  /* PLANNED - Expand city */
#define INTERVENTION_ADJUSTMENT  2  /* TWEAK - Balance economy */
#define INTERVENTION_CREATION    3  /* NEW - Create new area */
#define INTERVENTION_DESTRUCTION 4  /* REMOVAL - Abandon area */
#define INTERVENTION_EVOLUTION   5  /* GROWTH - Village → Town */

/* Intervention speed */
#define SPEED_INSTANT       0  /* Immediate (emergency) */
#define SPEED_FAST          1  /* Within 1 game day */
#define SPEED_NORMAL        2  /* Within 1 game week */
#define SPEED_SLOW          3  /* Over 1 game month */
#define SPEED_NATURAL       4  /* Let it happen organically */

/* Beeler's intervention */
typedef struct beeler_intervention {
    int intervention_type;
    int speed;

    /* What's happening */
    char *situation;             /* "Village starving - no food for 3 days" */
    char *problem_analysis;      /* AI-generated deep analysis */

    /* Beeler's response */
    char *chosen_action;         /* "Create emergency food supply + farm" */
    char *reasoning;             /* Why this action */
    int confidence;              /* 0-100: How sure Beeler is */

    /* Execution */
    bool executed;
    time_t execution_time;
    char *result;                /* What happened */

    /* Announcement */
    bool should_announce;
    int announcement_priority;

} BEELER_INTERVENTION;

/* ========================================================================
 * EXAMPLES - ORGANIC SCENARIOS
 * ======================================================================== */

/* Example 1: Village THRIVES and grows */
void example_village_prosperity(void)
{
    /*
     * SCENARIO: Farming Village discovers nearby iron mine
     *
     * VITAL SIGNS:
     * - Economic score: 60 → 85 (mine brings wealth)
     * - Trade volume: 100g/day → 500g/day
     * - Immigration: +15 NPCs (miners, merchants)
     * - Housing: Running out (too many people!)
     *
     * BEELER OBSERVES:
     * - Population pressure: TRUE (crowded)
     * - Economic boom: TRUE (mine profits)
     * - Growth momentum: ACCELERATING
     *
     * BEELER DECIDES:
     * "Village has critical growth potential. Expand to Town."
     *
     * ACTION:
     * - Add 30 new rooms (residential district)
     * - Add marketplace (merchants need space)
     * - Upgrade village elder → Mayor
     * - Create mining guild
     * - Build town walls (defense for wealth)
     * - Evolution: VILLAGE → TOWN
     *
     * ANNOUNCEMENT:
     * Priority: MEDIUM
     * Periodicos: "Farming Village Prospers Into Bustling Town"
     */
}

/* Example 2: Village DECLINES and abandons */
void example_village_collapse(void)
{
    /*
     * SCENARIO: Coastal fishing village - fish populations collapse
     *
     * VITAL SIGNS:
     * - Economic score: 70 → 20 (fishermen unemployed)
     * - Trade volume: 300g/day → 50g/day
     * - Food supply: 30 days → 5 days (starving)
     * - Emigration: +20 NPCs leaving
     * - Status: DECLINING → DYING
     *
     * BEELER OBSERVES:
     * - Resource depletion: TRUE (no more fish)
     * - Economic collapse: TRUE
     * - Mass emigration: TRUE
     * - Prognosis: "Village unsustainable without intervention"
     *
     * BEELER'S CHOICE:
     * Option 1: Emergency intervention (create new food source)
     * Option 2: Assist evacuation (help people leave)
     * Option 3: Let it die naturally (become ruins)
     *
     * BEELER DECIDES: Option 3 (natural death)
     * Reasoning: "Overfishing was players' fault. Natural consequences.
     *             Area will become interesting ruins. New story potential."
     *
     * ACTION (over 2 months):
     * - NPCs emigrate gradually
     * - Buildings decay
     * - Area status → RUINS
     * - New mobs: Ghosts of fishermen (lore!)
     * - New quest potential: "Revive the village?"
     *
     * ANNOUNCEMENT:
     * Priority: LOW
     * Periodicos: "Coastal Village Abandoned After Fishing Collapse"
     */
}

/* Example 3: City at WAR - emergency intervention */
void example_war_emergency(void)
{
    /*
     * SCENARIO: Orc horde attacks DarkHaven
     *
     * VITAL SIGNS:
     * - Safety score: 90 → 30 (under attack!)
     * - Deaths per month: 2 → 50 (casualties)
     * - Infrastructure: 95 → 60 (buildings burning)
     * - Status: STABLE → STRUGGLING
     *
     * BEELER OBSERVES:
     * - CRITICAL ALERT: "Major city under siege!"
     * - War devastation: TRUE
     * - Population at risk
     *
     * BEELER DECIDES:
     * Speed: INSTANT (emergency)
     * Action: "Reinforce defenses + create elite guard unit"
     * Reasoning: "DarkHaven is critical hub. Loss would destabilize
     *             entire region. Emergency intervention justified."
     *
     * ACTION (immediate):
     * - Create 20 elite guard NPCs (high level)
     * - Strengthen city walls magically
     * - Create emergency food/medical supplies
     * - Spawn Beeler avatar to assist defense (rare!)
     *
     * ANNOUNCEMENT:
     * Priority: CRITICAL
     * World chat: "BEELER INTERVENES: Divine reinforcements arrive at DarkHaven!"
     */
}

/* Example 4: Organic expansion - new area created */
void example_organic_new_area(void)
{
    /*
     * SCENARIO: Midgaard too crowded, people want to settle frontier
     *
     * VITAL SIGNS (Midgaard):
     * - Population: 200 NPCs (max capacity: 150)
     * - Housing: 75% occupied (crowded)
     * - Economic: Prosperous but cramped
     * - Immigration: Still coming (no space!)
     *
     * BEELER OBSERVES:
     * - Population pressure: TRUE
     * - No room to expand Midgaard (surrounded by existing areas)
     * - Frontier land available 80 rooms northwest
     *
     * BEELER DECIDES:
     * "Create new settlement for overflow population"
     * Speed: NORMAL (1 week organic growth)
     *
     * ACTION:
     * - Scout suitable location (grasslands, water access)
     * - Send 30 NPCs from Midgaard (pioneers)
     * - Create new area: "New Haven" (OUTPOST stage)
     *   - 8 rooms initially (camp-like)
     *   - Basic homes, well, small farm
     *   - Elder leader (reports to Midgaard mayor)
     *   - Trade route back to Midgaard
     * - Area can grow naturally if successful
     *
     * ANNOUNCEMENT:
     * Priority: MEDIUM
     * Periodicos: "Pioneers Establish New Haven Settlement"
     *
     * FUTURE: Might grow to village, or fail and be abandoned
     */
}

/* Example 5: Balanced adjustment - NOT intervention */
void example_non_intervention(void)
{
    /*
     * SCENARIO: Small village has minor food shortage
     *
     * VITAL SIGNS:
     * - Food supply: 15 days (normally 30)
     * - Economic: Stable
     * - Status: STABLE (not critical)
     *
     * BEELER OBSERVES:
     * - Problem exists but not severe
     * - Village has resources to solve it themselves
     * - Local farmer already increasing production
     *
     * BEELER DECIDES:
     * "No intervention needed. Monitor situation."
     * Reasoning: "Minor fluctuations are natural. Village can adapt.
     *             Over-intervention creates dependency."
     *
     * ACTION:
     * - Watch for 2 weeks
     * - If worsens → intervention
     * - If stabilizes → learn from it
     *
     * ANNOUNCEMENT: None (not important enough)
     */
}

/* ========================================================================
 * FUNCTION DECLARATIONS
 * ======================================================================== */

/* God-mode monitoring */
void beeler_divine_observation(void);    /* Observe ALL areas constantly */
void beeler_analyze_vital_signs(AREA_DATA *area);
AREA_VITAL_SIGNS *beeler_get_area_health(AREA_DATA *area);
int beeler_calculate_area_status(AREA_VITAL_SIGNS *vitals);

/* Decision-making */
bool beeler_should_intervene(AREA_VITAL_SIGNS *vitals);
int beeler_choose_intervention_type(AREA_VITAL_SIGNS *vitals);
int beeler_choose_intervention_speed(int intervention_type, int severity);
char *beeler_ai_analyze_situation(AREA_VITAL_SIGNS *vitals);
char *beeler_ai_generate_intervention_plan(char *situation);

/* Organic growth/decline */
void beeler_process_area_evolution(AREA_DATA *area);
bool beeler_should_area_grow(AREA_GROWTH *growth);
bool beeler_should_area_decline(AREA_GROWTH *growth);
void beeler_execute_growth(AREA_DATA *area, int rooms_to_add);
void beeler_execute_decline(AREA_DATA *area, int rooms_to_remove);
void beeler_evolve_area_stage(AREA_DATA *area, int new_stage);

/* Emergency response */
void beeler_emergency_intervention(char *crisis);
void beeler_handle_critical_alert(char *alert);

/* Natural processes */
void beeler_allow_natural_death(AREA_DATA *area);
void beeler_convert_to_ruins(AREA_DATA *area);
void beeler_create_abandoned_lore(AREA_DATA *area);

/* Balance regulation */
void beeler_adjust_world_balance(void);
void beeler_prevent_runaway_growth(void);
void beeler_prevent_total_collapse(void);

#endif /* BEELER_GOD_MODE_H */
