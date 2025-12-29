/*****************************************************************************
 * Universal Mob AI - Every Mob Has Intelligence
 *
 * NO MÁS mobs tontos que solo wanderean y attackean.
 * TODOS tienen personalidad, memoria, metas, relaciones.
 *
 * Niveles de inteligencia:
 * - Beeler: GOD-TIER (crea mundos)
 * - Leaders: STRATEGIC (guerra, comercio, construcción)
 * - NPCs importantes: TACTICAL (guild masters, merchants)
 * - NPCs comunes: SOCIAL (farmers, guards, cooks)
 * - Animales: INSTINCTUAL (hunt, flee, protect)
 * - Monsters: AGGRESSIVE (pero inteligente)
 *
 * "Even the smallest rat has a story to tell."
 *****************************************************************************/

#ifndef UNIVERSAL_MOB_AI_H
#define UNIVERSAL_MOB_AI_H

/* ========================================================================
 * INTELLIGENCE TIERS
 * ======================================================================== */

#define INTELLIGENCE_GOD         10  /* Beeler */
#define INTELLIGENCE_GENIUS       9  /* Archmages, ancient dragons */
#define INTELLIGENCE_STRATEGIC    8  /* Kings, leaders */
#define INTELLIGENCE_TACTICAL     7  /* Guild masters, generals */
#define INTELLIGENCE_EDUCATED     6  /* Merchants, teachers */
#define INTELLIGENCE_SOCIAL       5  /* Commoners, workers */
#define INTELLIGENCE_SIMPLE       4  /* Simple folk */
#define INTELLIGENCE_ANIMAL       3  /* Smart animals */
#define INTELLIGENCE_INSTINCT     2  /* Predators */
#define INTELLIGENCE_MINDLESS     1  /* Zombies, golems */
#define INTELLIGENCE_NONE         0  /* Objects */

/* ========================================================================
 * UNIVERSAL AI DATA - ALL MOBS HAVE THIS
 * ======================================================================== */

typedef struct universal_mob_ai {
    CHAR_DATA *mob;
    int intelligence_tier;

    /* PERSONALITY */
    char *personality_type;      /* "friendly", "grumpy", "greedy", "brave" */
    int personality_traits[10];  /* Multiple traits */
    char *ai_personality_desc;   /* Ollama-generated full personality */

    /* MEMORY */
    int num_memories;
    struct mob_memory {
        char *event_description;  /* "Player John killed my friend" */
        time_t when;
        int emotional_impact;     /* -100 to 100 */
        char *location;
    } *memories[50];             /* Last 50 memories */

    /* RELATIONSHIPS */
    int num_relationships;
    struct mob_relationship {
        CHAR_DATA *other;
        int relationship_value;   /* -100 (hate) to 100 (love) */
        char *relationship_type;  /* "friend", "enemy", "family", "boss" */
        char *how_they_met;
    } *relationships[20];

    /* GOALS & MOTIVATIONS */
    int num_goals;
    struct mob_goal {
        char *goal_description;   /* "Save enough gold to buy a home" */
        int priority;             /* 0-10 */
        int progress;             /* 0-100% complete */
        time_t deadline;          /* When to achieve by */
        bool achieved;
    } *goals[5];                 /* Up to 5 active goals */

    /* CURRENT STATE */
    int mood;                    /* -100 (miserable) to 100 (ecstatic) */
    int energy;                  /* 0-100 (tired to energized) */
    int hunger;                  /* 0-100 (starving to full) */
    int fear;                    /* 0-100 (calm to terrified) */
    int curiosity;               /* 0-100 (bored to fascinated) */

    /* DECISION-MAKING */
    char *current_thought;       /* What mob is thinking NOW */
    char *next_planned_action;   /* What mob plans to do */
    int decision_cooldown;       /* Ticks until next AI decision */

    /* CONVERSATION */
    char **conversation_topics;  /* What mob can talk about */
    int num_topics;
    char *last_conversation;     /* What was last said to/by mob */
    CHAR_DATA *talking_to;       /* Currently in conversation with */

    /* LEARNING */
    int num_skills_known;
    char **skills_known;
    int experience_level;        /* How much mob has "lived" */
    bool can_learn_new_things;

    /* AGENCY */
    bool can_create_things;      /* Can this mob build/spawn? */
    bool can_make_decisions;     /* Or just follows orders? */
    bool can_initiate_quests;    /* Can give players quests dynamically? */
    int power_level;             /* How much can mob change world */

} UNIVERSAL_MOB_AI;

/* ========================================================================
 * MOB CREATION & EXPANSION
 * ======================================================================== */

/* Mobs with high power can create things */
typedef struct mob_creation_ability {
    CHAR_DATA *mob;
    int creation_power;          /* 0-10 */

    /* What can this mob create? */
    bool can_create_rooms;
    bool can_create_npcs;
    bool can_create_items;
    bool can_create_quests;
    bool can_create_areas;       /* Only very powerful mobs */

    /* Limitations */
    int max_rooms_per_day;
    int max_npcs_per_day;
    int gold_cost_to_create;

    /* Examples:
     * - King (power 7): Can build 10 rooms/day, hire NPCs
     * - Archmage (power 8): Can create magical rooms, summon beings
     * - Beeler (power 10): Unlimited creation
     * - Farmer (power 1): Can plant crops (creates "crop" objects)
     * - Merchant (power 2): Can order new inventory (spawns items)
     */

} MOB_CREATION_ABILITY;

/* ========================================================================
 * DYNAMIC CONVERSATION - OLLAMA-POWERED
 * ======================================================================== */

/* Every mob can talk intelligently */
typedef struct dynamic_conversation {
    CHAR_DATA *speaker;
    CHAR_DATA *listener;

    /* Context for AI */
    char *speaker_personality;
    char *speaker_current_mood;
    char *speaker_relationship_to_listener;
    char *conversation_history;   /* Last 5 exchanges */
    char *location_context;       /* Where they are */
    char *recent_events;          /* What happened recently */

    /* AI-generated response */
    char *response;
    int response_tone;            /* Friendly, hostile, neutral, etc */

} DYNAMIC_CONVERSATION;

/* ========================================================================
 * EXAMPLES - INTELLIGENT MOBS
 * ======================================================================== */

#if 0  /* Example code - not compiled */

/* Example 1: Farmer with goals and agency */
void example_intelligent_farmer(void)
{
    /*
     * FARMER TOM
     * Intelligence: SOCIAL (5)
     * Personality: "hardworking", "family-oriented", "cautious"
     *
     * GOALS:
     * 1. Save 500 gold to buy bigger farm (Progress: 320/500g)
     * 2. Marry Sarah the baker (Progress: courting)
     * 3. Plant new wheat field (Progress: 80% - field ready)
     *
     * MEMORIES:
     * - "Orc raid destroyed my crops" (2 weeks ago, impact: -80)
     * - "Player Jane helped me rebuild" (1 week ago, impact: +60)
     * - "Good harvest this season" (yesterday, impact: +40)
     *
     * RELATIONSHIPS:
     * - Sarah (baker): +75 (romantic interest)
     * - Player Jane: +60 (grateful friend)
     * - Mayor: +40 (respects authority)
     * - Orcs: -90 (destroyed his farm)
     *
     * CURRENT STATE:
     * - Mood: +60 (optimistic about harvest)
     * - Energy: 70 (working hard)
     * - Hunger: 40 (will eat soon)
     *
     * AI DECISION:
     * "I'm close to affording that bigger farm. If I sell this harvest
     *  for good price, I can propose to Sarah. But I need to expand my
     *  wheat field first. I'll ask Player Jane if she can help clear
     *  the north field. She's been good to me."
     *
     * ACTIONS:
     * - Seeks out Player Jane
     * - Offers quest: "Help me clear the north field"
     * - Reward: 50 gold + friendship + future benefits
     * - If successful: Tom can EXPAND his farm (creates 2 new rooms!)
     */
}

/* Example 2: Merchant expands business */
void example_merchant_expansion(void)
{
    /*
     * MERCHANT GRETA
     * Intelligence: EDUCATED (6)
     * Power level: 3 (can hire NPCs, stock items)
     *
     * GOALS:
     * 1. Open second shop in New Haven (Progress: 40% - saving gold)
     * 2. Establish trade route with Midgaard (Progress: 0% - need permission)
     * 3. Train apprentice (Progress: 60% - found candidate)
     *
     * AI ANALYSIS:
     * "Business is booming. I'm making 200g/day profit. New Haven needs
     *  a general store - there's none there. If I open a branch shop,
     *  I can double my income. I need to hire an apprentice to run it."
     *
     * ACTION (over 1 week):
     * - Saves 1000g
     * - Asks Beeler/Mayor for permission to expand to New Haven
     * - If approved:
     *   - CREATES 1 new room in New Haven: "Greta's General Store"
     *   - SPAWNS 1 NPC: "Apprentice merchant" (runs the shop)
     *   - STOCKS items in new shop
     *   - Establishes trade route (items flow between both shops)
     *
     * Result: Merchant has ORGANICALLY expanded the world!
     */
}

/* Example 3: Guard captain creates militia */
void example_guard_creates_militia(void)
{
    /*
     * GUARD CAPTAIN MARCUS
     * Intelligence: TACTICAL (7)
     * Power level: 5 (can recruit guards, build defenses)
     *
     * SITUATION:
     * - Monster attacks increasing (5 attacks last week)
     * - Only has 10 guards (not enough)
     * - Mayor won't fund more guards (broke)
     *
     * AI DECISION:
     * "I can't wait for approval. Citizens are dying. I'll recruit
     *  volunteers from the local farmers and train a militia."
     *
     * ACTION (autonomous):
     * - Approaches 5 farmer NPCs
     * - Offers: "Join town guard, I'll train you, 20g/week"
     * - 3 farmers accept (need money, want to protect homes)
     * - Captain TRAINS them (profession changes: farmer → guard)
     * - Now has 13 guards instead of 10
     *
     * LONG-TERM:
     * - If attacks stop: Militia can return to farming
     * - If attacks continue: Militia becomes permanent
     * - Captain might BUILD guard tower (creates 3 rooms)
     */
}

/* Example 4: Archmage creates magical academy */
void example_archmage_creates_school(void)
{
    /*
     * ARCHMAGE ELDRIN
     * Intelligence: GENIUS (9)
     * Power level: 8 (can create areas!)
     *
     * GOALS:
     * - Train next generation of mages
     * - Preserve magical knowledge
     * - Protect kingdom from dark magic
     *
     * AI ANALYSIS:
     * "Too few mages in the kingdom. If I die, magical knowledge dies
     *  with me. I must create an academy to teach students. The old
     *  tower east of DarkHaven would be perfect."
     *
     * ACTION (BIG - over 1 month):
     * - Uses 10,000 gold + magical power
     * - CREATES NEW AREA: "Eldrin's Academy of High Magic"
     *   - 25 rooms (classrooms, library, dorms, labs)
     *   - 1 teacher NPC (himself)
     *   - 5 student NPCs (apprentices)
     *   - 3 magical guardian NPCs
     *   - Magical items (spellbooks, scrolls)
     * - Connected to DarkHaven (10 rooms east)
     * - Becomes LEARNING CENTER for players and NPCs
     *
     * ANNOUNCEMENT:
     * Priority: HIGH
     * "Archmage Eldrin Establishes Magical Academy Near DarkHaven"
     *
     * Result: ONE POWERFUL MOB CREATED AN ENTIRE AREA!
     */
}

/* Example 5: Animal intelligence - wolf pack */
void example_wolf_pack_intelligence(void)
{
    /*
     * ALPHA WOLF
     * Intelligence: ANIMAL (3)
     * Power level: 1 (limited agency)
     *
     * GOALS:
     * - Feed pack
     * - Protect territory
     * - Raise cubs
     *
     * MEMORY:
     * - "Deer herd migrated south" (3 days ago)
     * - "Human hunters killed Beta" (1 week ago, impact: -70)
     *
     * AI DECISION:
     * "Food scarce here. Deer gone. Cubs hungry. Must move pack south
     *  to follow deer. But humans dangerous. Will avoid human village."
     *
     * ACTION:
     * - Pack MIGRATES south (moves to different area)
     * - Avoids human settlement (intelligent pathing)
     * - Finds deer herd, hunts successfully
     * - Establishes NEW den (spawns in new location)
     *
     * Result: Even ANIMALS have agency and intelligent behavior!
     */
}

#endif  /* End of example code */

/* ========================================================================
 * FUNCTION DECLARATIONS
 * ======================================================================== */

/* Initialization */
void init_universal_mob_ai(void);
void assign_mob_intelligence(CHAR_DATA *mob);
int determine_mob_intelligence_tier(CHAR_DATA *mob);
int determine_mob_power_level(CHAR_DATA *mob);

/* Personality generation */
char *ai_generate_mob_personality(CHAR_DATA *mob);
void assign_personality_traits(UNIVERSAL_MOB_AI *ai);
void generate_mob_backstory(CHAR_DATA *mob);

/* Memory system */
void mob_remember_event(CHAR_DATA *mob, char *event, int emotional_impact);
void mob_forget_old_memories(CHAR_DATA *mob);  /* Keep only recent/important */
bool mob_remembers(CHAR_DATA *mob, CHAR_DATA *other);
char *mob_recall_memory(CHAR_DATA *mob, char *about_what);

/* Relationship system */
void mob_update_relationship(CHAR_DATA *mob, CHAR_DATA *other, int change);
int mob_get_relationship(CHAR_DATA *mob, CHAR_DATA *other);
void mob_form_relationship(CHAR_DATA *mob, CHAR_DATA *other, char *type);

/* Goal system */
void mob_set_goal(CHAR_DATA *mob, char *goal_description, int priority);
void mob_progress_goal(CHAR_DATA *mob, int goal_index, int progress_amount);
void mob_achieve_goal(CHAR_DATA *mob, int goal_index);
char *mob_get_current_goal(CHAR_DATA *mob);

/* AI access */
UNIVERSAL_MOB_AI *get_mob_ai(CHAR_DATA *mob);

/* AI decision-making */
void mob_ai_think(CHAR_DATA *mob);  /* Called every tick */
char *mob_ai_decide_action(UNIVERSAL_MOB_AI *ai);
void mob_execute_decision(CHAR_DATA *mob, char *decision);

/* Dynamic conversation */
char *mob_ai_respond_to_speech(CHAR_DATA *mob, CHAR_DATA *speaker, char *what_said);
char *mob_ai_start_conversation(CHAR_DATA *mob, CHAR_DATA *target);
void mob_remember_conversation(CHAR_DATA *mob, char *conversation_summary);

/* Mob creation ability */
bool mob_can_create(CHAR_DATA *mob);
bool mob_create_room(CHAR_DATA *mob, char *room_description);
bool mob_spawn_npc(CHAR_DATA *mob, char *npc_type);
bool mob_create_item(CHAR_DATA *mob, char *item_type);
bool mob_create_quest(CHAR_DATA *mob, CHAR_DATA *player);

/* Mob expansion */
void mob_expand_business(CHAR_DATA *merchant);
void mob_recruit_npcs(CHAR_DATA *leader);
void mob_build_structure(CHAR_DATA *builder, char *what);

/* Integration with Beeler */
bool beeler_approve_mob_creation(CHAR_DATA *mob, char *what_to_create);
void beeler_assist_mob_creation(CHAR_DATA *mob, char *what);

/* Update loop */
void universal_mob_ai_update(void);  /* Called every tick for ALL mobs */

#endif /* UNIVERSAL_MOB_AI_H */
