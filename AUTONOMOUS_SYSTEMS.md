# CosMUD - Autonomous AI Systems Documentation

**Last Updated:** March 28, 2026
**Status:** Simplified - 4 active systems (was 14)

---

## Overview

CosMUD is a SmaugFUSS 1.9.8 MUD with autonomous AI powered by Ollama (llama3.2).
After a major cleanup, the codebase was reduced from 14 half-broken systems to **4 working ones**.

**Philosophy:** Hacer más con menos. Only ship what works.

---

## Active Systems

### 1. Ollama Integration (`ollama_integration.c`)

HTTP client for local LLM via Ollama API.

- **Endpoint:** `http://localhost:11434/api/generate`
- **Model:** llama3.2
- **Key functions:**
  - `ollama_request(prompt, max_tokens)` - Main API call
  - `ollama_escape_json(str)` - JSON escaping (fixed: \n, \r, \t)
  - `ollama_is_available()` - Health check
- **Fallback:** Template responses if Ollama is down
- **Dependency:** libcurl (`-DHAVE_CURL`)

### 2. Universal Mob AI (`universal_mob_ai.c`)

Every NPC has personality, memory, and can respond to speech.

- **Game loop:** `universal_mob_ai_update()` every PULSE_MOBILE (~4 sec)
- **Init:** `init_universal_mob_ai()` in db.c
- **Features:**
  - 15 personality types (friendly, grumpy, wise, etc.)
  - Async speech responses (no lag)
  - Personality persistence to disk
  - Intelligence tiers (0-9, mindless to god)
  - Memory system (50 memories per mob)
  - Relationship tracking

**Personality Persistence:**
- Saved to: `../data/mob_personalities/<vnum>.txt`
- Format:
  ```
  VNUM 1234
  PERSONALITY grumpy
  TIER 5
  POWER 2
  AI_DESC
  A grumpy merchant who hates haggling.
  END_AI_DESC
  ```
- First boot: generates with Ollama and saves
- Subsequent boots: loads from file (instant)

**Async Speech Flow:**
1. Player says something → `mob_queue_speech_response()`
2. Shows "pauses thoughtfully..." (instant)
3. Next tick: `mob_process_pending_responses()` calls Ollama
4. Next tick: `mob_deliver_pending_responses()` delivers response
5. Validates mob/speaker still alive (race condition fix)

### 3. Mob Housing (`mob_home.c`)

NPCs have homes and follow day/night cycles.

- **Game loop:** `housing_system_update()` every PULSE_MOBILE
- **Init:** `init_housing_system()` in db.c
- **Schedule:**
  - 20:00: Bedtime - NPCs go home, set ACT_SENTINEL
  - 06:00: Wake - remove ACT_SENTINEL
  - Night: 10% chance per hour to head home

### 4. Leader AI (`leader_ai.c`)

43 area leaders detected and making basic decisions.

- **Game loop:** `leader_ai_update()` every PULSE_MOBILE (1hr throttle)
- **Init:** `init_leader_ai()` in db.c
- **Working:** Leader detection, area classification
- **Basic:** Trade, war, building decisions

---

## Supporting Files (kept)

| File | Purpose |
|------|---------|
| `beeler_assign.c` | `beeler assign <vnum>` command - assigns personality via Ollama |
| `beeler_commands.c` | Immortal commands: `minvoke`, `beeler` |
| `mob_identity.c` | Minimal identity storage for beeler_assign |

---

## Removed Systems (10 cut)

| System | Why Removed |
|--------|-------------|
| beeler.c | 60% stubs, create_beeler_npc() TODO |
| beeler_god_mode.c | Never ran in game loop |
| beeler_architect.c | Never ran in game loop |
| world_history_tracker.c | Never ran in game loop |
| organic_creation.c | Never ran in game loop |
| player_world_impact.c | Never ran in game loop |
| book_writing_system.c | Never ran in game loop |
| ai_context_analyzer.c | Not even initialized |
| world_context.c | 70% TODOs |
| economy_stub.c | All stubs |
| mob_identity_stubs.c | Replaced with working mob_identity.c |
| redis_bridge.c | Optional, not core |
| periodicos.c | Depended on dead systems |

**Note:** .h files kept for compatibility. .c files removed from Makefile.

---

## Game Loop Integration

**File:** `update.c:2051-2058`

```c
if (--pulse_mobile <= 0)
{
    pulse_mobile = PULSE_MOBILE;
    mobile_update();              // Standard SMAUG mob AI
    universal_mob_ai_update();    // Async speech + AI behavior
    leader_ai_update();           // Leader strategic decisions
    housing_system_update();      // Night/day home cycles
}
```

## Initialization Sequence

**File:** `db.c` (boot_db function)

```c
init_ollama();
init_leader_ai();
init_universal_mob_ai();
init_housing_system();
```

---

## Startup

```bash
cd /home/user/cosMUD
./startup.sh
```

Or manually:
```bash
cd /home/user/cosMUD/Dev/area
nohup ../src/smaug 4000 &
```

**Expected boot:** ~60-90 seconds with saved personalities, zero Ollama calls.

---

## Troubleshooting

### MUD laggy at startup
- Check `data/mob_personalities/` has files
- First boot is slow (generating), subsequent are fast
- Verify Ollama is running: `curl http://localhost:11434/api/tags`

### NPCs not responding to speech
- Check `universal_mob_ai_update()` is in update.c
- Check Ollama is running
- In-game: `ollama test`

### Compilation errors after cleanup
- Ensure Makefile only has: `ollama_integration.c universal_mob_ai.c mob_home.c leader_ai.c beeler_assign.c beeler_commands.c mob_identity.c`
- Install: `apt-get install libcurl4-openssl-dev`
- Run: `make clean && make`

---

## File Map

```
Dev/src/
├── ollama_integration.c    # Ollama HTTP client (ACTIVE)
├── universal_mob_ai.c      # Core mob AI + async speech (ACTIVE)
├── mob_home.c              # Housing system (ACTIVE)
├── leader_ai.c             # Leader AI (ACTIVE)
├── beeler_assign.c         # beeler assign command (ACTIVE)
├── beeler_commands.c       # Immortal commands (ACTIVE)
├── mob_identity.c          # Minimal identity storage (ACTIVE)
├── *.h                     # Headers (kept for compat)
└── [removed .c files]      # Dead code, not compiled

Dev/data/
├── mob_personalities/      # Saved AI personalities (1700+ files)
├── mob_homes/              # Home assignments
└── beeler/                 # Beeler snapshots
```

---

## For Future Claude Code Sessions

**IMPORTANT:** This codebase was cleaned up on March 28, 2026.
- Only 7 AI source files compile (listed above)
- Dead .h files exist but their .c files are NOT in the Makefile
- Don't re-add removed systems without good reason
- All mob AI personality data persists to `data/mob_personalities/`
- The MUD's look and feel (colors, interface) is UNCHANGED - only backend AI was simplified
