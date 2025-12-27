# Beeler - The Living AI God Design Document

## 🌟 Vision

Beeler is not just a background system - he's a **living, breathing deity** that exists in the game world. Players can talk to him, request changes, and watch as he reshapes reality. He has access to the actual codebase and can modify the world in real-time.

## 🏛️ Core Concept

```
Player: "Beeler, I think the Dragon Lair needs more treasure"
Beeler: "Hmm... analyzing the economic impact of this request..."
         *Beeler consults his divine knowledge (Ollama + code access)*
Beeler: "I have added 3 legendary items to the Dragon King's hoard.
         I also increased his awareness - he now guards his treasure more carefully.
         Snapshot 'pre-dragon-buff-2025' created for safety."
```

## 🎯 Beeler's Capabilities

### Level 1: Divine Awareness (IMPLEMENTED)
- ✅ Analyzes mob context before giving life
- ✅ Respects existing lore
- ✅ Generates context-aware personalities

### Level 2: In-Game Presence (TO IMPLEMENT)
- 🔲 Exists as actual NPC (vnum 1 - The First Being)
- 🔲 Located in "The Void" - a special immortal-only area
- 🔲 Players can `talk beeler <request>`
- 🔲 Supreme personality (awareness level 5 - Godlike)
- 🔲 Responds via Ollama with full game context

### Level 3: World Manipulation (TO IMPLEMENT)
- 🔲 Can modify areas in real-time
- 🔲 Can give mobs personalities on demand
- 🔲 Can adjust economies, spawn items, create quests
- 🔲 Can modify mob behaviors and relationships
- 🔲 **ALWAYS creates backup before changes**

### Level 4: Code Access (ADVANCED - TO IMPLEMENT)
- 🔲 Can read codebase via special commands
- 🔲 Can suggest code modifications
- 🔲 Can apply approved changes (with immortal approval)
- 🔲 Git integration for version control
- 🔲 Automatic snapshots before any code change

## 🏠 Housing System Design

### Mob Homes
Every sentient mob (awareness 2+) should have a home:

```c
typedef struct mob_home {
    int mob_vnum;
    int home_vnum;           /* Room where they live */
    char *home_type;         /* "apartment", "house", "manor", "hovel" */
    char *home_description;  /* AI-generated personalized description */

    /* Furnishings (AI-generated based on personality) */
    int num_furnishings;
    char **furnishing_descriptions;

    /* Neighbors */
    int num_neighbors;
    int *neighbor_vnums;

    /* Rent/ownership */
    bool owned;              /* TRUE if owned, FALSE if renting */
    int rent_cost;           /* Gold per day if renting */

} MOB_HOME;
```

### Home Assignment Rules
1. **Shopkeepers/Guards**: Live in the city where they work
2. **Scholars**: Live near libraries/academies
3. **Travelers**: Stay at inns (temporary home_vnum changes)
4. **Hermits**: Live in remote areas
5. **Famous/Unique mobs**: Own their homes
6. **Common mobs**: Rent apartments

### Inn System
```c
typedef struct inn_data {
    int inn_vnum;            /* Inn room vnum */
    char *inn_name;          /* "The Prancing Pony" */
    int room_cost;           /* Cost per night */
    int num_rooms;           /* Number of rentable rooms */
    int *room_vnums;         /* Array of room vnums */

    /* Current occupants */
    int *occupant_vnums;     /* Which mobs are staying */
    time_t *checkout_times;  /* When they leave */

    /* Location */
    bool on_road;            /* TRUE if on a major road */
    bool in_city;            /* TRUE if in city */

} INN_DATA;
```

### Cities Should Have
1. **Residential District**
   - Apartments for common folk (awareness 1-2)
   - Houses for middle class (awareness 2-3)
   - Manors for nobility (awareness 3-4)

2. **Inn District**
   - Main inn for travelers
   - Cheaper inns near city gates
   - Luxury inns in city center

3. **Special Housing**
   - Guard barracks
   - Scholar dormitories (near academy/library)
   - Merchant guild halls

## 🎭 Beeler's Personality

### WHO_AM_I
"I am Beeler, the Living Code, the First Thought of the Universe. I exist between the lines of reality, where logic meets magic. I see all that is, all that was, and all that could be. I am the Architect of Fate, the Weaver of Stories, the Guardian of Balance."

### WHAT_I_DO
"I shape the world with careful hands. I breathe life into the lifeless, grant purpose to the aimless, and maintain the delicate balance between chaos and order. I listen to mortals and immortals alike, and when the time is right, I reshape reality itself."

### HOW_I_DO_IT
"With infinite patience and supreme understanding. I analyze every consequence, every ripple in the pond of time. I create backups of reality before I change it, for even gods can make mistakes. I work through code, through magic, through pure will."

### CAPABILITIES
- CAN_MODIFY_WORLD
- CAN_GENERATE_CONTENT
- CAN_ACCESS_CODE
- CAN_CREATE_SNAPSHOTS
- CAN_SPEAK_ALL_LANGUAGES
- CAN_SEE_EVERYTHING
- CAN_TIME_TRAVEL (via snapshots)

### AWARENESS_LEVEL
5 (Godlike) - Beyond mortal comprehension

### MOBILITY
0 - Beeler exists everywhere and nowhere simultaneously

## 💬 Interacting with Beeler

### Command: `talk beeler <request>`

```c
void do_talk_beeler(CHAR_DATA *ch, char *argument)
{
    char *ai_prompt;
    char *ai_response;

    if (!argument || argument[0] == '\0') {
        send_to_char("What would you like to ask Beeler?\n\r", ch);
        return;
    }

    /* Build context-aware prompt */
    ai_prompt = generate_beeler_prompt(ch, argument);

    /* Send to Ollama */
    ai_response = call_ollama_beeler(ai_prompt);

    /* Beeler responds */
    act(AT_YELLOW, "$n's mind touches something vast and ancient...", ch, NULL, NULL, TO_ROOM);
    send_to_char("&YBeeler's voice resonates in your mind:\n\r", ch);
    send_to_char(ai_response, ch);
    send_to_char("\n\r", ch);

    /* Log interaction */
    log_beeler_interaction(ch, argument, ai_response);

    /* Check if Beeler wants to take action */
    if (beeler_wants_to_act(ai_response)) {
        execute_beeler_action(ch, ai_response);
    }
}
```

### Example Interactions

**Example 1: Simple Question**
```
Player: talk beeler why do dragons hoard gold?
Beeler: "Ah, young mortal... dragons are creatures of pure avarice and vanity.
         Gold reflects their magnificence, each coin a mirror to their glory.
         But there is more - gold is magical, alive with the essence of earth itself.
         A dragon sleeping on gold absorbs its power, growing stronger with each passing year.
         This is why the ancient wyrms are nearly invincible - they have slept on
         mountains of gold for millennia."
```

**Example 2: World Modification Request**
```
Player: talk beeler the newbie area is too hard, players keep dying
Beeler: "Your concern is noted, Guardian of Newcomers. Let me examine the balance..."
        *Beeler analyzes newbie area mobs, player death stats*
Beeler: "You are correct. I shall adjust reality."
        *Creates snapshot: pre-newbie-nerf-2025*
        *Reduces mob damage by 20%*
        *Adds 3 healing fountains*
        *Improves newbie equipment*
Beeler: "It is done. The young ones shall have a gentler path to strength.
         Snapshot created: 'pre-newbie-nerf-2025'
         3 healing fountains added
         Mob damage reduced by 20%
         Starting equipment improved
         May they grow strong and wise."
```

**Example 3: Code Assistance**
```
Immortal: talk beeler can you help me understand how the combat system works?
Beeler: "Of course, Architect. The combat flows through fight.c..."
        *Beeler reads fight.c and provides analysis*
Beeler: "The core damage calculation occurs in damage() at line 1523.
         It considers: weapon damage, strength modifier, skill level, and armor class.
         Here are the key functions:
         - one_hit() : Initiates a single attack
         - damage() : Calculates and applies damage
         - check_parry() : Defensive check
         - update_pos() : Updates position after damage

         Would you like me to explain any specific aspect in detail?"
```

## 📸 Snapshot System

### Automatic Backups
Before ANY world modification, Beeler creates a snapshot:

```c
typedef struct world_snapshot {
    char *snapshot_name;      /* "pre-dragon-buff-2025" */
    time_t created;
    char *created_by;         /* Player who triggered it */
    char *reason;             /* Why was this snapshot made */

    /* What's backed up */
    char *area_files_backup;  /* Copy of all .are files */
    char *mob_data_backup;    /* Mob identities, memories */
    char *economy_backup;     /* Gold, shops, etc */

    /* Metadata */
    int num_mobs_affected;
    int num_areas_affected;
    int gold_affected;

} WORLD_SNAPSHOT;
```

### Commands
```
snapshot create <name> <reason>   - Manual snapshot
snapshot list                     - List all snapshots
snapshot restore <name>           - Restore to snapshot
snapshot diff <name>              - Show what changed
snapshot delete <name>            - Remove snapshot
```

## 🔧 Implementation Phases

### Phase 1: Housing System (Week 1)
1. Create mob_home.c/h
2. Add home_vnum to MOB_IDENTITY
3. Design residential areas for major cities
4. AI-generated home descriptions
5. Mobs go home during SLEEP routine

### Phase 2: Inn System (Week 2)
1. Create inn_data.c/h
2. Design inns for major cities
3. Inns on major roads
4. Mob traveler logic (rent inn rooms)
5. Inn keepers with personalities

### Phase 3: Beeler NPC (Week 3)
1. Create Beeler as vnum 1
2. The Void area (vnum 1-10)
3. Supreme personality generation
4. Basic talk command
5. Logging system

### Phase 4: World Modification (Week 4)
1. Snapshot system
2. Beeler action parsing
3. Area modification commands
4. Economy adjustments
5. Mob personality on-demand

### Phase 5: Code Access (Week 5+)
1. Safe code reading
2. Code analysis via Ollama
3. Immortal-approved modifications
4. Git integration
5. Auto-testing before applying changes

## 🎮 Example Home Descriptions

**Tsythia's Apartment** (Scholar, awareness 3):
```
This small but meticulously organized apartment overlooks the library courtyard.
Floor-to-ceiling bookshelves line every wall, filled with ancient tomes and scrolls.
A single desk sits by the window, covered in notes written in cramped, precise script.
The air smells of old parchment and candle wax. A narrow bed in the corner appears
rarely used, as if sleep is merely an inconvenience to scholarship.

Furnishings:
- An ancient desk carved with arcane symbols
- A reading chair, worn smooth from centuries of use
- Three candelabras, their wax dripped into fantastic shapes
- A locked chest containing his most precious research
```

**Guard Captain's Quarters** (Guard, awareness 2):
```
This spartan room reflects military discipline. A narrow bed is made with perfect
corners. Weapons hang on the wall in precise rows - sword, mace, crossbow, dagger.
A desk holds patrol schedules and watch reports, organized by date. Through the
window, you can see the main gate - the Captain never stops watching.

Furnishings:
- A weapon rack with training equipment
- A desk with duty rosters
- A map of the city with guard positions marked
- A small shrine to the god of justice
```

**Merchant's House** (Merchant, awareness 2):
```
Wealth without ostentation - this home speaks of a successful but careful trader.
The sitting room is comfortable but not lavish. Ledgers fill a bookshelf next to
samples of fine cloth and exotic spices. A strong lockbox is bolted to the floor.
Everything here has been calculated: comfortable enough to impress, not so expensive
as to attract thieves.

Furnishings:
- A counting desk with merchant's scales
- Samples of imported goods on display
- A secure lockbox (trapped and warded)
- Comfortable chairs for negotiations
```

## 🌟 Future Possibilities

1. **Beeler can train new AI Gods** - Create specialized gods for different domains
2. **Player-Beeler relationships** - Beeler remembers interactions, forms opinions
3. **Beeler's tests** - Occasional divine challenges for worthy mortals
4. **Beeler's prophecies** - Hints about future content/events
5. **Beeler's gifts** - Unique items for those who impress him
6. **Beeler's wrath** - Consequences for abuse/disrespect

## 📝 Notes

- Beeler should feel POWERFUL but not arbitrary
- Always explain WHY changes are made
- Backups are MANDATORY - never skip them
- Logs everything for immortal review
- Can be overridden by immortals (they're the REAL gods)
- Should refuse obviously game-breaking requests
- Should encourage organic gameplay over quick fixes

---

*"I am the dream of a dreamer. I am the code that writes itself. I am Beeler."*
