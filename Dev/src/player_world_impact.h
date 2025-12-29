/*****************************************************************************
 * Player World Impact System
 *
 * Las acciones de los PLAYERS importan. Todo lo que hacen afecta el mundo:
 * - Matar mobs → Safety score cambia
 * - Completar quests → Economic/cultural boost
 * - Donar gold/items → Construction progress
 * - Destruir NPCs → Population decline
 * - Traer comercio → Trade routes
 * - Proteger áreas → Safety increase
 *
 * Pero los cambios son LENTOS, CONTROLADOS, BALANCEADOS.
 * Un nuevo town puede tomar ~6 MESES en formarse.
 *
 * "The world remembers. Every action echoes through time."
 *****************************************************************************/

#ifndef PLAYER_WORLD_IMPACT_H
#define PLAYER_WORLD_IMPACT_H

/* ========================================================================
 * TEMPORAL SCALING - Realistic Timeframes
 * ======================================================================== */

/* Real-time to game-time ratio (configurable) */
#define GAME_TIME_MULTIPLIER    4    /* 1 real hour = 4 game hours */

/* Construction/Evolution Timeframes (in GAME TIME) */
#define TIME_BUILD_SINGLE_ROOM      (7 * 24)     /* 1 week game-time */
#define TIME_BUILD_SMALL_HOUSE      (14 * 24)    /* 2 weeks */
#define TIME_BUILD_SHOP             (21 * 24)    /* 3 weeks */
#define TIME_BUILD_FORTRESS         (60 * 24)    /* 2 months */
#define TIME_BUILD_DISTRICT         (90 * 24)    /* 3 months */

/* Area Evolution Timeframes (in GAME TIME) */
#define TIME_OUTPOST_TO_HAMLET      (30 * 24)    /* 1 month */
#define TIME_HAMLET_TO_VILLAGE      (60 * 24)    /* 2 months */
#define TIME_VILLAGE_TO_TOWN        (180 * 24)   /* 6 months - SLOW! */
#define TIME_TOWN_TO_CITY           (360 * 24)   /* 1 year */
#define TIME_CITY_TO_METROPOLIS     (720 * 24)   /* 2 years */

/* Decline Timeframes (faster than growth - disaster) */
#define TIME_DECLINE_STABLE_TO_STRUGGLING   (60 * 24)   /* 2 months */
#define TIME_DECLINE_STRUGGLING_TO_DYING    (30 * 24)   /* 1 month */
#define TIME_DECLINE_DYING_TO_RUINS         (60 * 24)   /* 2 months */

/* Change Rate Limits */
#define MAX_ROOMS_ADDED_PER_DAY     3        /* No más de 3 rooms/día */
#define MAX_NPCS_SPAWNED_PER_DAY    5        /* No más de 5 NPCs/día */
#define MAX_VITAL_SIGN_CHANGE_PER_DAY  5     /* ±5 puntos/día en scores */

/* ========================================================================
 * PLAYER ACTION TRACKING
 * ======================================================================== */

/* Player action types that affect world */
#define PLAYER_ACTION_KILL_MOB          0
#define PLAYER_ACTION_KILL_PLAYER       1
#define PLAYER_ACTION_COMPLETE_QUEST    2
#define PLAYER_ACTION_DONATE_GOLD       3
#define PLAYER_ACTION_DONATE_ITEM       4
#define PLAYER_ACTION_BUILD             5   /* Immortal builds */
#define PLAYER_ACTION_DESTROY           6
#define PLAYER_ACTION_TRADE             7
#define PLAYER_ACTION_PROTECT           8
#define PLAYER_ACTION_HELP_NPC          9
#define PLAYER_ACTION_HARM_NPC          10

/* Player action impact */
typedef struct player_action_impact {
    CHAR_DATA *player;
    int action_type;
    time_t when;

    /* Where */
    AREA_DATA *area_affected;
    ROOM_INDEX_DATA *room;

    /* What */
    char *action_description;    /* "Killed 5 orcs near village" */
    CHAR_DATA *target_mob;       /* If killed/helped a mob */
    OBJ_DATA *target_obj;        /* If donated/destroyed object */

    /* Impact magnitude */
    int economic_impact;         /* -100 to 100 */
    int safety_impact;           /* -100 to 100 */
    int population_impact;       /* -100 to 100 */
    int cultural_impact;         /* -100 to 100 */
    int environmental_impact;    /* -100 to 100 */

    /* Cumulative tracking */
    bool significant;            /* Important enough to remember */
    bool announced;              /* Was it announced? */

    struct player_action_impact *next;
} PLAYER_ACTION_IMPACT;

/* Area's history of player actions */
typedef struct area_player_history {
    AREA_DATA *area;

    /* Recent actions (last 30 days) */
    int num_recent_actions;
    PLAYER_ACTION_IMPACT *recent_actions[100];

    /* Aggregated impact (last 30 days) */
    int total_economic_impact;
    int total_safety_impact;
    int total_population_impact;
    int total_cultural_impact;

    /* Top contributors */
    CHAR_DATA *biggest_helper;   /* Most positive impact */
    CHAR_DATA *biggest_threat;   /* Most negative impact */
    int helper_impact_score;
    int threat_impact_score;

    /* Reputation tracking */
    int num_player_reputations;
    struct player_reputation {
        CHAR_DATA *player;
        int reputation_score;    /* -100 (hated) to 100 (hero) */
        char *reputation_title;  /* "Savior of the Village", "Destroyer" */
    } *player_reputations[50];

} AREA_PLAYER_HISTORY;

/* ========================================================================
 * GRADUAL CHANGE SYSTEM
 * ======================================================================== */

/* Represents an ongoing change happening over time */
typedef struct gradual_change {
    int change_id;
    char *change_description;

    /* What's changing */
    AREA_DATA *area;
    int change_type;             /* EVOLUTION, CONSTRUCTION, DECLINE, etc */

    /* Timeline */
    time_t start_time;
    time_t estimated_completion;
    int total_duration_hours;    /* How long total (game hours) */
    int hours_elapsed;
    int progress_percentage;     /* 0-100 */

    /* What happens when complete */
    char *completion_effect;     /* "Village becomes Town" */
    int rooms_to_add;
    int npcs_to_spawn;
    int stage_evolution;         /* New STAGE_* */

    /* Can be accelerated by players */
    bool player_can_help;
    int gold_needed;             /* Remaining gold to speed up */
    int items_needed;            /* Items that would help */
    int quests_needed;           /* Quests that advance progress */

    /* Current helpers */
    int num_helpers;
    CHAR_DATA *helpers[20];      /* Players contributing */
    int contributions[20];       /* How much each helped */

    /* Visibility */
    bool visible_to_players;     /* Can players see progress? */
    char *progress_message;      /* "The new town hall is 60% complete" */

    /* Can be disrupted */
    bool can_be_disrupted;
    int disruption_events;       /* Attacks, disasters that slow it */

    struct gradual_change *next;
} GRADUAL_CHANGE;

/* ========================================================================
 * EXAMPLE TIMELINES
 * ======================================================================== */

/* Example 1: Village → Town (6 MONTHS GAME TIME) */
void example_village_to_town_timeline(void)
{
    /*
     * MONTH 1: Recognition Phase
     * - Beeler observes: "Village thriving, needs expansion"
     * - Beeler announces (periodicos): "Village growth initiative proposed"
     * - Mayor NPC begins planning
     * - Players can donate gold to speed up
     * - Progress: 0% → 10%
     *
     * MONTH 2: Foundation Phase
     * - Initial construction begins (roads, infrastructure)
     * - 1-2 rooms added per week (slowly!)
     * - NPCs start talking about expansion
     * - Merchant NPCs order more stock
     * - Progress: 10% → 25%
     *
     * MONTH 3: Building Phase
     * - Main construction (homes, shops)
     * - 2-3 rooms added per week
     * - Immigration starts (+2-3 NPCs per week)
     * - Trade volume increases
     * - Players helping → faster progress
     * - Progress: 25% → 50%
     *
     * MONTH 4: Development Phase
     * - Major structures (town hall, walls)
     * - Existing rooms upgraded
     * - More NPCs arrive
     * - Cultural changes (festivals appear)
     * - Progress: 50% → 70%
     *
     * MONTH 5: Refinement Phase
     * - Final construction
     * - NPC professions diversify
     * - Trade routes formalize
     * - Town identity emerges
     * - Progress: 70% → 90%
     *
     * MONTH 6: Completion
     * - Final touches
     * - Official ceremony (players invited!)
     * - Mayor → promoted to proper Town Mayor
     * - STAGE_VILLAGE → STAGE_TOWN
     * - Progress: 90% → 100%
     * - Announcement: HIGH priority (celebration!)
     *
     * TOTAL: ~6 months game time (~45 days real time if 4x multiplier)
     */
}

/* Example 2: Player Impact on Construction */
void example_player_accelerates_construction(void)
{
    /*
     * SCENARIO: Town building new marketplace
     *
     * BASE TIMELINE: 3 months (90 days game time)
     *
     * PLAYER ACTIONS:
     *
     * Week 1:
     * - Player John donates 500 gold
     * - Impact: Hire more workers
     * - Progress: +5% (now at 10%)
     * - Timeline: -5 days (85 days remaining)
     *
     * Week 3:
     * - Player Sarah completes quest "Gather 100 wood"
     * - Impact: Materials provided
     * - Progress: +10% (now at 25%)
     * - Timeline: -10 days (75 days remaining)
     *
     * Week 5:
     * - Player Mike kills bandits attacking construction site
     * - Impact: Workers feel safe, work faster
     * - Progress: +3% (now at 35%)
     * - Timeline: -3 days (72 days remaining)
     *
     * Week 8:
     * - Player Jane brings NPC carpenter from another town
     * - Impact: Expert help
     * - Progress: +8% (now at 50%)
     * - Timeline: -7 days (65 days remaining)
     *
     * RESULT: What would take 90 days now takes 65 days
     * Players' actions MATTERED - they see tangible impact!
     *
     * NPC dialogue updates:
     * - "Thanks to the adventurers' help, we're ahead of schedule!"
     * - "John's generous donation really sped things up"
     * - "Sarah's timber delivery was a godsend"
     */
}

/* Example 3: Player Impact on Safety Score (Immediate) */
void example_player_kills_monsters(void)
{
    /*
     * SCENARIO: Player clears orc camp near village
     *
     * BEFORE:
     * - Safety score: 40/100 (unsafe)
     * - Monster attacks: 5 per week
     * - Emigration: -3 NPCs per week (fleeing danger)
     * - Status: DECLINING
     *
     * PLAYER ACTION:
     * - Player kills 20 orcs in nearby camp
     * - Clears orc leader
     *
     * IMMEDIATE IMPACT:
     * - Safety score: 40 → 65 (+25)
     * - Monster attacks: 5/week → 1/week
     * - NPC dialogue: "The orcs have been driven off!"
     *
     * WEEK 1 AFTER:
     * - Emigration stops (people feel safer)
     * - Immigration: +2 NPCs (word spreads)
     *
     * MONTH 1 AFTER:
     * - Safety score: 65 → 75 (stabilized)
     * - Economic score: +10 (merchants return)
     * - Status: DECLINING → STABLE
     *
     * MONTH 3 AFTER:
     * - Village no longer threatened
     * - Population growing again
     * - Status: STABLE → PROSPEROUS
     *
     * LONG-TERM:
     * - Village names plaza after player: "Hero's Square"
     * - Mayor gives title: "Protector of the Village"
     * - Players see LASTING impact of their actions
     */
}

/* Example 4: Player Destruction (Negative Impact) */
void example_player_destroys_village(void)
{
    /*
     * SCENARIO: Evil player kills many NPCs
     *
     * BEFORE:
     * - Village stable, 60 NPCs
     * - Economic score: 70/100
     * - Safety score: 80/100
     *
     * PLAYER ACTION:
     * - Evil player kills 15 NPCs over 2 weeks
     * - Destroys shops, steals goods
     *
     * IMMEDIATE IMPACT:
     * - Population: 60 → 45 (-15)
     * - Safety score: 80 → 40 (-40)
     * - Economic score: 70 → 45 (-25)
     * - Player reputation: -80 (HATED)
     *
     * WEEK 1 AFTER:
     * - Emigration: -5 NPCs (fleeing)
     * - Guards double (if mayor can afford)
     * - Trade stops (too dangerous)
     *
     * MONTH 1 AFTER:
     * - Status: STABLE → STRUGGLING
     * - Population: 45 → 35
     * - Beeler observes: "Village in crisis"
     *
     * BEELER'S DECISION:
     * Option 1: Emergency intervention (create guards, food)
     * Option 2: Let natural consequences happen
     *
     * If Option 2:
     * - MONTH 3: Status → DYING
     * - MONTH 6: Status → RUINS
     * - Result: Player's actions DESTROYED entire village
     *
     * CONSEQUENCES FOR PLAYER:
     * - Reputation: "The Destroyer"
     * - Bounty on player's head
     * - Guards in ALL villages attack on sight
     * - Some NPCs remember and seek revenge
     * - Lasting world impact!
     */
}

/* ========================================================================
 * FUNCTION DECLARATIONS
 * ======================================================================== */

/* Initialization */
void init_player_world_impact(void);
void load_player_impact_history(void);
void save_player_impact_history(void);

/* Action tracking */
void record_player_action(CHAR_DATA *player, int action_type, void *target, AREA_DATA *area);
PLAYER_ACTION_IMPACT *create_action_impact(CHAR_DATA *player, int action_type);
void apply_action_to_vital_signs(PLAYER_ACTION_IMPACT *action, AREA_VITAL_SIGNS *vitals);

/* Specific action handlers */
void player_killed_mob(CHAR_DATA *player, CHAR_DATA *mob);
void player_completed_quest(CHAR_DATA *player, QUEST_DATA *quest);
void player_donated_gold(CHAR_DATA *player, AREA_DATA *area, int amount);
void player_donated_item(CHAR_DATA *player, AREA_DATA *area, OBJ_DATA *item);
void player_helped_npc(CHAR_DATA *player, CHAR_DATA *npc, char *how);
void player_harmed_npc(CHAR_DATA *player, CHAR_DATA *npc);

/* Reputation system */
void update_player_reputation(CHAR_DATA *player, AREA_DATA *area, int change);
int get_player_reputation(CHAR_DATA *player, AREA_DATA *area);
char *get_reputation_title(CHAR_DATA *player, AREA_DATA *area);
bool is_player_hero(CHAR_DATA *player, AREA_DATA *area);
bool is_player_villain(CHAR_DATA *player, AREA_DATA *area);

/* Gradual change system */
GRADUAL_CHANGE *create_gradual_change(AREA_DATA *area, int change_type, int duration_hours);
void update_gradual_changes(void);  /* Called every game hour */
void advance_gradual_change(GRADUAL_CHANGE *change, int hours);
void complete_gradual_change(GRADUAL_CHANGE *change);

/* Player contribution to gradual changes */
void player_contribute_gold(CHAR_DATA *player, GRADUAL_CHANGE *change, int amount);
void player_contribute_quest(CHAR_DATA *player, GRADUAL_CHANGE *change, QUEST_DATA *quest);
void player_contribute_item(CHAR_DATA *player, GRADUAL_CHANGE *change, OBJ_DATA *item);
int calculate_acceleration(GRADUAL_CHANGE *change);  /* How much faster due to players */

/* Disruption */
void disrupt_gradual_change(GRADUAL_CHANGE *change, char *reason, int setback_hours);

/* Temporal scaling */
int convert_real_to_game_time(int real_hours);
int convert_game_to_real_time(int game_hours);
char *format_time_remaining(int game_hours);  /* "3 weeks, 2 days" */

/* Change rate limiting */
bool can_add_room_today(AREA_DATA *area);
bool can_spawn_npc_today(AREA_DATA *area);
bool can_change_vital_sign(AREA_VITAL_SIGNS *vitals, int change_amount);
void record_room_added(AREA_DATA *area);
void record_npc_spawned(AREA_DATA *area);

/* Integration with Beeler */
void beeler_consider_player_actions(AREA_DATA *area);
bool beeler_should_reward_players(AREA_DATA *area);
bool beeler_should_punish_players(AREA_DATA *area);
void beeler_announce_player_impact(CHAR_DATA *player, AREA_DATA *area, char *impact);

/* Commands for players */
void do_areainfo(CHAR_DATA *ch, char *argument);      /* See area status */
void do_progress(CHAR_DATA *ch, char *argument);      /* See ongoing changes */
void do_contribute(CHAR_DATA *ch, char *argument);    /* Help with construction */
void do_reputation(CHAR_DATA *ch, char *argument);    /* See your reputation */

#endif /* PLAYER_WORLD_IMPACT_H */
