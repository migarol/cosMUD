# CosMUD - Autonomous AI Systems Documentation

**Last Updated:** January 7, 2026
**Status:** Production-ready with 15+ autonomous AI systems

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Architecture](#architecture)
3. [Autonomous Systems](#autonomous-systems)
4. [Recent Fixes & Improvements](#recent-fixes--improvements)
5. [Startup & Operations](#startup--operations)
6. [Troubleshooting](#troubleshooting)
7. [File Structure](#file-structure)
8. [Developer Guide](#developer-guide)

---

## System Overview

CosMUD is a SmaugFUSS 1.9.8 MUD with 15+ integrated autonomous AI systems powered by Ollama (llama3.2). The world operates autonomously with:

- **2000+ NPCs** with unique AI personalities
- **43 autonomous leaders** making strategic decisions
- **Persistent memory** and personality systems
- **Async speech** for lag-free NPC interactions
- **Housing system** - NPCs go home, sleep, wake up
- **Zero-lag startup** via personality persistence

---

## Architecture

### Core Components

```
┌─────────────────────────────────────────────────────────────┐
│                    SMAUG MUD CORE                            │
│                  (SmaugFUSS 1.9.8)                           │
└─────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
┌───────▼──────┐   ┌───────▼──────┐   ┌───────▼──────┐
│   OLLAMA     │   │   BEELER     │   │  UNIVERSAL   │
│ INTEGRATION  │   │  GOD MODE    │   │   MOB AI     │
└───────┬──────┘   └───────┬──────┘   └───────┬──────┘
        │                   │                   │
        └───────────────────┴───────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
┌───────▼──────┐   ┌───────▼──────┐   ┌───────▼──────┐
│   LEADER     │   │   HOUSING    │   │    ASYNC     │
│     AI       │   │    SYSTEM    │   │   SPEECH     │
└──────────────┘   └──────────────┘   └──────────────┘
```

### Data Flow: NPC Speech Response

```
1. Player says "hello" to NPC
   │
   ├─→ do_say() in act_comm.c detects NPC in room
   │
   ├─→ mob_queue_speech_response() creates pending response
   │   - Stores: mob pointer, speaker pointer, what was said
   │   - Shows: "$n pauses thoughtfully..."
   │   - Adds to global queue
   │
   ├─→ universal_mob_ai_update() (called every PULSE_MOBILE ~4 sec)
   │   ├─→ mob_process_pending_responses()
   │   │   - Processes ONE response per tick (non-blocking)
   │   │   - Calls ollama_request() in separate operation
   │   │   - Marks response as "processing"
   │   │
   │   └─→ mob_deliver_pending_responses()
   │       - Validates mob/speaker still exist (race condition check)
   │       - Delivers ready responses via act()
   │       - Frees memory and removes from queue
   │
   └─→ Player sees: "NPC says 'response text'"
```

---

## Autonomous Systems

### 1. Ollama Integration (`ollama_integration.c`)

**Purpose:** HTTP client for local LLM (llama3.2) via Ollama API

**Key Functions:**
- `ollama_request()` - Main API call with streaming support
- `ollama_escape_json()` - Proper JSON escaping (fixes newline bugs)
- `ollama_is_available()` - Health check

**API Endpoint:** `http://localhost:11434/api/generate`

**Recent Fixes:**
- Fixed JSON escaping for special characters (\n, \r, \t, etc)
- Changed from simple if-statement to switch-statement for escaping

**File:** `/home/user/cosMUD/Dev/src/ollama_integration.c`

---

### 2. Beeler God Mode (`beeler_god_mode.c`, `mob_identity.c`)

**Purpose:** Autonomous AI overseer that assigns personalities to NPCs

**Key Functions:**
- `do_beeler_assign()` - Admin command: `beeler assign <vnum>`
- `create_mob_identity()` - Creates MOB_IDENTITY struct
- `get_mob_identity()` - Retrieves saved identity

**What Beeler Does:**
1. Analyzes mob stats (level, race, description)
2. Calls Ollama to generate personality
3. Stores in MOB_IDENTITY structure
4. Saves to memory (linked list)

**Data Structure:**
```c
typedef struct mob_identity_data {
    int mob_vnum;
    char *who_am_i;           // "I am a merchant"
    char *what_i_do;          // "I sell weapons"
    char *ai_prompt;          // Full personality description
    int awareness_level;
    // ... more fields
} MOB_IDENTITY;
```

**Files:**
- `/home/user/cosMUD/Dev/src/beeler_god_mode.c`
- `/home/user/cosMUD/Dev/src/mob_identity.c`

---

### 3. Universal Mob AI (`universal_mob_ai.c`)

**Purpose:** EVERY NPC has intelligence, memory, personality, goals

**Key Functions:**
- `assign_mob_intelligence()` - Assigns AI to mob (with persistence!)
- `mob_ai_think()` - Decision-making loop
- `assign_personality_traits()` - Generates or loads personality
- `save_mob_ai_personality()` - **NEW** Saves to file
- `load_mob_ai_personality()` - **NEW** Loads from file

**Intelligence Tiers:**
```c
INTELLIGENCE_MINDLESS   = 0  // Slimes, oozes
INTELLIGENCE_ANIMAL     = 1  // Wolves, bears
INTELLIGENCE_INSTINCT   = 2  // Aggressive creatures
INTELLIGENCE_SIMPLE     = 3  // Basic humanoids
INTELLIGENCE_SOCIAL     = 4  // Merchants, guards
INTELLIGENCE_EDUCATED   = 5  // Scholars, mages
INTELLIGENCE_TACTICAL   = 6  // Military leaders
INTELLIGENCE_STRATEGIC  = 7  // Guild masters
INTELLIGENCE_GENIUS     = 8  // Archmages
INTELLIGENCE_GOD        = 9  // Beeler
```

**Personality Assignment Priority:**
1. **Beeler identity** (if exists) - manually assigned by god
2. **Saved file** (if exists) - loaded from disk (FAST!)
3. **Generate new** - call Ollama and save for next time

**Persistence System (NEW!):**
- Location: `/home/user/cosMUD/Dev/data/mob_personalities/`
- Format: `<vnum>.txt`
- Example:
  ```
  VNUM 1234
  PERSONALITY grumpy
  TIER 5
  POWER 2
  AI_DESC
  A grumpy old merchant who doesn't like to haggle.
  END_AI_DESC
  ```

**Files:**
- `/home/user/cosMUD/Dev/src/universal_mob_ai.c`
- `/home/user/cosMUD/Dev/src/universal_mob_ai.h`

---

### 4. Async Speech System (`universal_mob_ai.c`)

**Purpose:** NPCs respond to player speech WITHOUT LAG

**Key Functions:**
- `mob_queue_speech_response()` - Queues response for processing
- `mob_process_pending_responses()` - Processes ONE per tick
- `mob_deliver_pending_responses()` - Delivers completed responses

**Queue Structure:**
```c
struct pending_speech_response {
    CHAR_DATA *mob;           // NPC responding
    CHAR_DATA *speaker;       // Player who spoke
    char *what_was_said;      // Original text
    char *response;           // AI-generated response
    bool processing;          // Currently calling Ollama?
    struct pending_speech_response *next;
};
```

**Flow:**
1. Player says something → queued instantly
2. Shows "$n pauses thoughtfully..." immediately
3. Next tick: process ONE response (call Ollama)
4. Next tick: deliver response if ready
5. Validate mob/speaker still exist (race condition fix)

**Race Condition Fix:**
- Before delivering response, iterate through `first_char` list
- Verify both mob and speaker pointers still valid
- If mob died while thinking → discard response safely

**Integration Point:** `act_comm.c:1241-1245` in `do_say()`

**Files:**
- `/home/user/cosMUD/Dev/src/universal_mob_ai.c` (lines 562-748)
- `/home/user/cosMUD/Dev/src/act_comm.c` (do_say function)

---

### 5. Leader AI (`leader_ai.c`)

**Purpose:** 43 autonomous faction leaders make strategic decisions

**Key Functions:**
- `init_leader_ai()` - Load leaders from config
- `leader_ai_update()` - Decision loop (called every PULSE_MOBILE)
- `leader_make_decision()` - Strategic AI decisions

**Leader Types:**
- Military commanders
- Guild masters
- Faction leaders
- Economic controllers

**Decisions:**
- Declare war/peace
- Trade agreements
- Resource allocation
- NPC hiring/firing

**Files:**
- `/home/user/cosMUD/Dev/src/leader_ai.c`
- `/home/user/cosMUD/Dev/src/leader_ai.h`

---

### 6. Housing System (`mob_home.c`)

**Purpose:** NPCs have homes, go to sleep at night, wake in morning

**Key Functions:**
- `init_housing_system()` - Load homes from files
- `housing_system_update()` - **NEW** Time-based behavior loop
- `mob_go_home()` - Pathfinding to home
- `is_mob_at_home()` - Check if at home location

**Schedule:**
- **20:00 (8 PM):** Bedtime - all NPCs go home, set ACT_SENTINEL
- **06:00 (6 AM):** Wake time - remove ACT_SENTINEL, start day
- **Night (20:00-06:00):** 10% chance per hour to go home if out

**Data Structure:**
```c
typedef struct mob_home_data {
    int mob_vnum;
    int home_vnum;          // Room where they live
    char *home_name;        // "a cozy cottage"
    MOB_HOME_TYPE type;     // HOUSE, SHOP, GUILD, etc
    // ... more fields
} MOB_HOME;
```

**Files:**
- `/home/user/cosMUD/Dev/src/mob_home.c` (housing_system_update: lines 1411-1491)

---

### 7. World History (`world_history.c`)

**Purpose:** Tracks major events, player actions, wars, discoveries

**Key Functions:**
- `record_event()` - Log world-changing events
- `get_recent_history()` - Query recent events
- `world_history_summary()` - AI-generated summaries

**Event Types:**
- Player achievements
- Wars declared/ended
- Major discoveries
- Economic crashes
- Natural disasters

**Files:**
- `/home/user/cosMUD/Dev/src/world_history.c`

---

### 8. Organic Creation (`organic_creation.c`)

**Purpose:** Beeler-driven world building

**Key Functions:**
- `beeler_create_area()` - Generate new areas
- `beeler_create_mob()` - Generate NPCs organically
- `beeler_create_object()` - Generate items

**Files:**
- `/home/user/cosMUD/Dev/src/organic_creation.c`

---

## Recent Fixes & Improvements

### Session Summary (Jan 6-7, 2026)

#### Fix 1: JSON Escaping Bug
**Problem:** `beeler assign` failing with HTTP 400 - "invalid character '\n' in string literal"

**Root Cause:** `ollama_escape_json()` wasn't escaping newlines

**Fix:** Changed to switch-statement, added proper escapes:
```c
case '\n': *dst++ = '\\'; *dst++ = 'n'; break;
case '\r': *dst++ = '\\'; *dst++ = 'r'; break;
case '\t': *dst++ = '\\'; *dst++ = 't'; break;
```

**File:** `ollama_integration.c:509-565`

---

#### Fix 2: Beeler → Universal Mob AI Connection
**Problem:** NPCs not using Beeler-assigned personalities

**Fix:** Modified `assign_mob_intelligence()` to check for MOB_IDENTITY first

**File:** `universal_mob_ai.c:49-91`

---

#### Fix 3: Async Speech Not Processing
**Problem:** Mobs show "pauses thoughtfully..." but never respond

**Root Cause:** `universal_mob_ai_update()` not in game loop

**Fix:** Added to `update.c:2054` in PULSE_MOBILE section

**File:** `update.c:2051-2058`

---

#### Fix 4: Missing Initialization
**Problem:** QA audit found systems not initialized

**Fixes:**
1. Added `init_universal_mob_ai()` to `db.c:810`
2. Added `leader_ai_update()` to `update.c:2056`

**Files:**
- `db.c:803-813`
- `update.c:2051-2058`

---

#### Fix 5: Race Condition in Speech Delivery
**Problem:** Use-after-free if mob dies while response pending

**Fix:** Validate mob/speaker in `first_char` list before delivery

**Code:**
```c
bool mob_valid = FALSE;
bool speaker_valid = FALSE;
for (ch = first_char; ch; ch = ch->next) {
    if (ch == pending->mob) mob_valid = TRUE;
    if (ch == pending->speaker) speaker_valid = TRUE;
}
if (mob_valid && speaker_valid) {
    // Safe to deliver
}
```

**File:** `universal_mob_ai.c:685-716`

---

#### Fix 6: Housing System Activation
**Problem:** Mob homes existed but not running

**Fix:** Created `housing_system_update()` function
- Time-based behavior (bedtime/wake)
- Added to game loop
- Added initialization

**Files:**
- `mob_home.c:1411-1491` (new function)
- `db.c:812` (init call)
- `update.c:2057` (game loop)

---

#### Fix 7: **CRITICAL - Startup Lag (10+ minutes)**
**Problem:** MUD taking 10+ minutes to boot - calling Ollama for EVERY mob synchronously

**Root Cause:**
- `assign_mob_intelligence()` called at startup for 500+ mobs
- Each mob without saved personality called Ollama (1-2 seconds)
- 500 mobs × 1-2 sec = 10+ minutes BLOCKED

**Solution: Personality Persistence System**

Created save/load system:
1. `save_mob_ai_personality()` - Save after Ollama generation
2. `load_mob_ai_personality()` - Load from file (INSTANT)
3. Modified assignment priority:
   - Beeler identity (highest)
   - **Saved file (NEW!)** ← FAST
   - Generate new + save

**Results:**
- ✅ Startup: ~64 seconds (was 10+ minutes)
- ✅ 1,733+ personalities saved
- ✅ 0 Ollama calls at startup
- ✅ Only NEW mobs call Ollama

**Files:**
- `universal_mob_ai.c:60-160` (persistence functions)
- `universal_mob_ai.c:186-227` (modified assignment)

---

## Startup & Operations

### Directory Structure

```
/home/user/cosMUD/Dev/
├── src/                    # Source code
│   ├── smaug              # Main executable
│   ├── ollama_integration.c
│   ├── universal_mob_ai.c
│   ├── beeler_god_mode.c
│   ├── leader_ai.c
│   ├── mob_home.c
│   └── ... (more source files)
├── area/                   # Area files (.are)
│   ├── area.lst
│   └── *.are              # World data
├── system/                 # Runtime files
│   ├── time.dat           # Game time
│   ├── weathermap.dat     # Weather state
│   ├── boot.txt           # Boot log
│   └── shutdown.txt
└── data/                   # Persistent data
    ├── mob_personalities/ # **NEW** - Saved AI personalities
    ├── mob_homes/         # Home assignments
    ├── beeler/            # Beeler snapshots
    └── districts/         # District data
```

### Startup Sequence

**From `/home/user/cosMUD/Dev/area/`:**

```bash
./startup.sh   # or: nohup ../src/smaug 4000 &
```

**Boot Process:**

1. **Load Core Data** (2 seconds)
   - Socials, skills, classes, races
   - Areas and world files

2. **Initialize AI Systems** (1 second)
   ```c
   init_ollama();              // Connect to Ollama
   init_beeler();              // Beeler god mode
   init_world_history();       // Event tracking
   init_leader_ai();           // 43 leaders
   init_universal_mob_ai();    // Create personality dir
   init_housing_system();      // Load homes
   ```

3. **Load Mobs** (60 seconds with persistence)
   - For each mob:
     - Check Beeler identity → use if exists
     - **Check saved file** → load if exists (FAST!)
     - Generate new → call Ollama + save
   - **With 1733+ saved:** 0 Ollama calls, instant load

4. **Start Game Loop**
   - PULSE_MOBILE (~4 seconds):
     - `mobile_update()` - Standard mob AI
     - `universal_mob_ai_update()` - Process async speech
     - `leader_ai_update()` - Leader decisions
     - `housing_system_update()` - Home/sleep behavior

**Expected Startup Time:** ~64 seconds total

---

### Game Loop Integration

**File:** `/home/user/cosMUD/Dev/src/update.c:2051-2058`

```c
if (--pulse_mobile <= 0)
{
    pulse_mobile = PULSE_MOBILE;
    mobile_update();                  // Standard mob AI
    universal_mob_ai_update();        // Process async speech
    leader_ai_update();               // Leader decisions
    housing_system_update();          // Home/sleep behavior
}
```

**Frequency:** Every PULSE_MOBILE (~4 seconds)

---

### Admin Commands

**Beeler Commands:**
```
beeler status           - Show Beeler system status
beeler assign <vnum>    - Assign personality to mob
beeler test             - Test Ollama connection
```

**Ollama Commands:**
```
ollama test             - Test Ollama API
ollama generate <text>  - Test generation
```

**Housing Commands:**
```
housing list            - List all mob homes
housing assign <vnum> <room>  - Assign home to mob
```

---

## Troubleshooting

### Problem: MUD Laggy at Startup

**Symptoms:**
- Takes 5-10+ minutes to boot
- Logs show many "OLLAMA DEBUG" messages
- Can't login during startup

**Causes:**
1. **Missing personality files** - Fresh install without saved personalities
2. **Wrong working directory** - Not running from `/area/` directory
3. **Ollama slow/down** - Local LLM server issues

**Solutions:**

1. **Check personality files exist:**
   ```bash
   ls /home/user/cosMUD/Dev/data/mob_personalities/*.txt | wc -l
   ```
   Should show ~1700+ files. If 0 or few:
   - First startup will be slow (generating all)
   - Subsequent startups will be fast

2. **Verify working directory:**
   ```bash
   cd /home/user/cosMUD/Dev/area
   nohup ../src/smaug 4000 &
   ```
   Path to `data/mob_personalities/` is relative (`../data/`)

3. **Check Ollama status:**
   ```bash
   curl http://localhost:11434/api/generate -d '{"model":"llama3.2","prompt":"test"}'
   ```

**Quick Fix - Skip Ollama at Startup:**
- Temporarily disable AI generation in `assign_personality_traits()`
- All mobs get random personalities (instant)
- Can assign proper ones later with `beeler assign`

---

### Problem: NPCs Not Responding to Speech

**Symptoms:**
- Say "hello" to NPC
- NPC shows "pauses thoughtfully..."
- Never responds

**Causes:**
1. `universal_mob_ai_update()` not in game loop
2. Ollama API down/slow
3. Response queue stuck

**Debug Steps:**

1. **Check game loop:**
   ```bash
   grep "universal_mob_ai_update" /home/user/cosMUD/Dev/src/update.c
   ```
   Should be at line ~2054

2. **Check Ollama:**
   In-game: `ollama test`

3. **Check queue:**
   Add debug to `mob_process_pending_responses()`:
   ```c
   sprintf(log_buf, "QUEUE: %d pending responses", queue_size);
   log_string(log_buf);
   ```

---

### Problem: Mobs Not Going Home at Night

**Symptoms:**
- Time is 20:00 (8 PM)
- NPCs still wandering around

**Causes:**
1. `housing_system_update()` not in game loop
2. Mob has no home assigned
3. Mob is in combat/aggressive

**Debug Steps:**

1. **Check game loop:**
   ```bash
   grep "housing_system_update" /home/user/cosMUD/Dev/src/update.c
   ```

2. **Check mob has home:**
   In-game: `housing list` (admin command)

3. **Check time:**
   Look at game time display

---

### Problem: "OLLAMA ERROR" in Logs

**Common Errors:**

**Error:** `invalid character '\n' in string literal`
- **Cause:** JSON escaping bug (FIXED)
- **Fix:** Update to latest code with switch-statement escaping

**Error:** `Connection refused`
- **Cause:** Ollama not running
- **Fix:** `ollama serve` or check systemd service

**Error:** `model not found`
- **Cause:** llama3.2 not downloaded
- **Fix:** `ollama pull llama3.2`

**Error:** HTTP 500
- **Cause:** Ollama server crash
- **Fix:** Restart Ollama service

---

### Problem: Memory Leak / High RAM Usage

**Symptoms:**
- RAM usage grows over time
- MUD eventually crashes

**Known Causes:**
1. Personality strings not freed
2. Speech response queue not cleared
3. Memory not freed when mob dies

**Checks:**

1. **Verify mob AI cleanup:**
   - When mob dies, should free `ai->personality_type`
   - When mob dies, should free `ai->ai_personality_desc`

2. **Verify speech queue cleanup:**
   - Completed responses freed in `mob_deliver_pending_responses()`
   - Orphaned responses (mob died) should be freed

3. **Use valgrind for leak detection:**
   ```bash
   valgrind --leak-check=full ./smaug 4000
   ```

---

## File Structure

### Critical Files

| File | Purpose | Lines of Interest |
|------|---------|-------------------|
| `ollama_integration.c` | Ollama API client | 509-565 (JSON escaping) |
| `universal_mob_ai.c` | Core AI system | 60-160 (persistence), 562-748 (async speech) |
| `beeler_god_mode.c` | Personality generator | Full file |
| `mob_identity.c` | Identity data structures | Full file |
| `leader_ai.c` | Faction leaders | Full file |
| `mob_home.c` | Housing system | 1411-1491 (update function) |
| `act_comm.c` | Player communication | 1241-1245 (do_say integration) |
| `update.c` | Game loop | 2051-2058 (AI updates) |
| `db.c` | Database initialization | 803-813 (init calls) |

### Data Files

| Directory | Contents | Format |
|-----------|----------|--------|
| `data/mob_personalities/` | **NEW** Saved AI personalities | `<vnum>.txt` |
| `data/beeler/` | Beeler snapshots | JSON |
| `data/mob_homes/` | Home assignments | Custom format |
| `area/*.are` | World data | SMAUG area format |
| `system/time.dat` | Game time state | Key-value |
| `system/weathermap.dat` | Weather state | Key-value |

---

## Developer Guide

### Adding a New Autonomous System

**Example: Adding a "Reputation System"**

1. **Create source files:**
   ```bash
   touch src/reputation_system.c src/reputation_system.h
   ```

2. **Add to Makefile:**
   Edit `src/Makefile`, add to `O_FILES`:
   ```make
   o/reputation_system.o
   ```

3. **Create init function:**
   ```c
   // reputation_system.c
   void init_reputation_system(void)
   {
       log_string("REPUTATION: Initializing...");
       // Load saved data
       load_reputation_data();
       log_string("REPUTATION: Initialized");
   }
   ```

4. **Create update function:**
   ```c
   void reputation_system_update(void)
   {
       // Called every PULSE_MOBILE (~4 seconds)
       // Update reputation scores
       // Process faction standings
   }
   ```

5. **Add to initialization sequence:**
   Edit `db.c`, add extern and call:
   ```c
   extern void init_reputation_system(void);

   // In boot_db():
   init_reputation_system();
   ```

6. **Add to game loop:**
   Edit `update.c`:
   ```c
   #include "reputation_system.h"

   // In update_handler():
   if (--pulse_mobile <= 0) {
       // ... existing updates ...
       reputation_system_update();
   }
   ```

7. **Compile and test:**
   ```bash
   cd src
   make
   cd ../area
   ./startup.sh
   ```

---

### Modifying Personality Generation

**Location:** `universal_mob_ai.c:187-235`

**To change personality prompts:**

```c
char *ai_generate_mob_personality(CHAR_DATA *mob)
{
    char prompt[MAX_STRING_LENGTH];

    // Customize this prompt:
    sprintf(prompt,
        "Generate a personality for:\n"
        "Name: %s\n"
        "Level: %d\n"
        "Race: %s\n"
        "Add more context here...",
        mob->short_descr,
        mob->level,
        race_table[mob->race]->race_name
    );

    response = ollama_request(prompt, 100);
    return response;
}
```

**To add new personality types:**

Edit the `personality_types` array:
```c
const char *personality_types[] = {
    "friendly", "grumpy", "greedy",
    "mysterious",    // NEW
    "paranoid",      // NEW
    "theatrical",    // NEW
    NULL
};
```

---

### Adding Admin Commands

**Example: `reputation show <player>`**

1. **Add command function:**
   ```c
   // reputation_system.c
   void do_reputation(CHAR_DATA *ch, char *argument)
   {
       char arg[MAX_INPUT_LENGTH];

       argument = one_argument(argument, arg);

       if (!str_cmp(arg, "show")) {
           // Show reputation
       }

       send_to_char("Syntax: reputation show <name>\n", ch);
   }
   ```

2. **Declare in header:**
   ```c
   // reputation_system.h
   void do_reputation(CHAR_DATA *ch, char *argument);
   ```

3. **Register command:**
   Edit command table (location varies by codebase):
   ```c
   {"reputation", do_reputation, LEVEL_IMMORTAL, LOG_NORMAL}
   ```

4. **Recompile and test**

---

### Debugging Tips

**Enable verbose logging:**

Add to your AI functions:
```c
#define DEBUG_AI 1

#ifdef DEBUG_AI
    sprintf(log_buf, "AI_DEBUG: mob=%s, state=%d",
            mob->short_descr, ai->state);
    log_string(log_buf);
#endif
```

**Track function calls:**
```c
void my_function(CHAR_DATA *ch)
{
    log_string("TRACE: my_function() called");
    // ... your code ...
    log_string("TRACE: my_function() completed");
}
```

**Monitor queue sizes:**
```c
sprintf(log_buf, "QUEUE SIZE: %d pending speech responses",
        count_pending_responses());
log_string(log_buf);
```

---

### Performance Considerations

**Do NOT do in game loop:**
- ❌ File I/O on every tick
- ❌ Long string operations
- ❌ Ollama requests (use async queue)
- ❌ Complex pathfinding every tick

**DO optimize:**
- ✅ Cache frequently accessed data
- ✅ Use cooldowns for expensive operations
- ✅ Process work in batches
- ✅ Use async for external API calls

**Example - Cooldown Pattern:**
```c
void expensive_ai_function(UNIVERSAL_MOB_AI *ai)
{
    if (ai->decision_cooldown > 0) {
        ai->decision_cooldown--;
        return;  // Skip this tick
    }

    // Do expensive work
    make_complex_decision(ai);

    // Set cooldown (don't run again for 15 ticks = 60 seconds)
    ai->decision_cooldown = 15;
}
```

---

## Git Repository Structure

**Branch:** `claude/review-latest-commit-RYDx2`

**Recent Commits:**
```
0a86de4 - Stop tracking runtime system files
975cbb2 - Remove compiled binaries from git tracking
ba8d2d5 - Update .gitignore - exclude runtime files
471cb85 - Fix startup lag - Add personality persistence system
f79f618 - Update runtime system files - MUD running with housing system
```

**Ignored Files (`.gitignore`):**
```
Dev/src/*.o
Dev/src/o/*.o
Dev/src/smaug
Dev/system/time.dat
Dev/system/weathermap.dat
Dev/data/mob_personalities/
```

---

## Testing Checklist

### Before Deployment

- [ ] Compile without errors: `cd src && make`
- [ ] Check Ollama API: `ollama test` (in-game)
- [ ] Verify personality files exist: `ls data/mob_personalities/*.txt | wc -l`
- [ ] Test startup time: Should be <90 seconds
- [ ] Test NPC speech: Say hello, get response
- [ ] Test housing: Wait until 20:00, NPCs go home
- [ ] Check logs: No "ERROR" messages in boot.txt
- [ ] Memory check: Run 24 hours, check RAM usage
- [ ] Player count: Test with 10+ concurrent players

---

## Performance Metrics

**Expected Performance (with 1733+ saved personalities):**

| Metric | Expected | Problem If |
|--------|----------|------------|
| Startup Time | 60-90 seconds | >180 seconds |
| Ollama Calls (startup) | 0-10 | >50 |
| NPC Response Time | 1-3 seconds | >5 seconds |
| RAM Usage (idle) | 50-100 MB | >500 MB |
| RAM Growth Rate | <1 MB/hour | >10 MB/hour |
| Login Lag | <1 second | >3 seconds |

---

## Future Enhancements

### Planned Systems
- [ ] Family lineage system (headers exist)
- [ ] Mining/farming/crafting (code exists, not integrated)
- [ ] Wars and sieges (not implemented)
- [ ] Auto-generation of content (partially implemented)
- [ ] Economy simulation (basic tracking exists)
- [ ] Weather affecting NPC behavior
- [ ] Persistent mob memory across restarts

### Optimization Opportunities
- [ ] Parallel personality loading at startup
- [ ] Redis caching for frequent queries
- [ ] Compressed personality storage
- [ ] Incremental saves (not full dumps)

---

## Contact & Support

**Current Developer:** Claude Code AI Agent
**Repository:** Private (migarol/cosMUD)
**Documentation Date:** January 7, 2026

**For Issues:**
1. Check logs: `tail -100 system/boot.txt`
2. Review this document's Troubleshooting section
3. Check recent commits for related fixes

---

## Appendix: Key Code Snippets

### Async Speech Example

```c
// In act_comm.c do_say():
if (IS_NPC(vch) && !IS_NPC(ch))
{
    mob_queue_speech_response(vch, ch, (char*)sbuf);
}

// In universal_mob_ai.c:
void mob_queue_speech_response(CHAR_DATA *mob, CHAR_DATA *speaker, char *what_said)
{
    pending_speech_response *pending = CREATE(pending_speech_response, 1);
    pending->mob = mob;
    pending->speaker = speaker;
    pending->what_was_said = str_dup(what_said);
    pending->processing = FALSE;

    LINK(pending, first_pending_response, last_pending_response, next);

    act(AT_ACTION, "$n pauses thoughtfully...", mob, NULL, speaker, TO_ROOM);
}
```

### Personality Persistence Example

```c
// Save after generation:
void assign_personality_traits(UNIVERSAL_MOB_AI *ai)
{
    // Generate personality
    ai->personality_type = strdup(personality_types[random]);
    ai->ai_personality_desc = ai_generate_mob_personality(ai->mob);

    // SAVE for next time
    save_mob_ai_personality(
        ai->mob->pIndexData->vnum,
        ai->personality_type,
        ai->ai_personality_desc,
        ai->intelligence_tier,
        ai->power_level
    );
}

// Load at startup:
if (load_mob_ai_personality(vnum, &personality, &ai_desc, &tier, &power))
{
    // Use saved (FAST!)
    ai->personality_type = personality;
    ai->ai_personality_desc = ai_desc;
}
else
{
    // Generate new and save
    assign_personality_traits(ai);
}
```

---

**END OF DOCUMENTATION**
