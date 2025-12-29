# FASE 4: Autonomous World Generation - Implementation Plan

## Overview

Transform Seldeon (cosMUD) into a **fully autonomous, organically growing ecosystem** where:
- The world moves and grows by itself
- Things create themselves based on need
- Everything is congruent (geography, politics, lore, economy)
- Leaders make real strategic decisions
- Announcements are selective (not spam)
- Skills/professions/resources emerge organically

## User's Vision (Exact Words)

> "osea piensa tu que podemos hacer para que neta sea un mundo que se mueve solo crece solo, se crean cosas solas.. en una de esas se necesita pescado y beeler decide que puedes pescar en el mar y crea todo lo necesario para ese nuevo skill y quienes pueden y quienes no y toda lo necesario pa que jale, embone y escale"

**Translation**: A world that moves by itself, grows by itself, creates things by itself. For example, if fish is needed, Beeler decides you can fish in the sea and creates EVERYTHING needed for that new skill - who can use it, who can't, everything necessary for it to work, fit together, and scale.

## Core Principles

1. **Context-Aware**: Check geography, politics, lore, economy BEFORE creating anything
2. **Congruent**: Everything must make sense (no fishing in desert)
3. **Intelligent Distribution**: Not all areas get all resources
4. **Political Realism**: DarkHaven guards protect DarkHaven mines
5. **Selective Announcements**: Only important things in chat, rest in periodicos
6. **Organic Growth**: Create entire ecosystems, not isolated pieces
7. **Scalable**: Everything interconnected and balanced

## New Systems Created

### 1. Periodicos (periodicos.h)
**Purpose**: Smart announcement system - not spam

**Features**:
- Announcement priority levels (CRITICAL, HIGH, MEDIUM, LOW, SILENT)
- Selective chat announcements (max 6 per hour)
- In-game newspapers for less urgent news
- Event categories (war, trade, construction, culture, etc.)
- AI-generated article bodies

**Examples**:
- War declaration → CRITICAL → world chat immediately
- King builds fortress → HIGH → chat + periodicos
- NPC marriage → MEDIUM → periodicos only
- Daily market prices → SILENT → no announcement

### 2. Organic Creation (organic_creation.h)
**Purpose**: Create entire skill/resource ecosystems on-demand

**Features**:
- Need detection (economy, geography, balance)
- Validation system (6-stage congruence check)
- Dependency analysis (fishing needs: skill, rod, fish, NPCs, rooms, recipes)
- AI-generated creation plans
- Full ecosystem building

**Example - Fishing**:
When fish is needed, creates:
1. Fishing skill (level requirements, difficulty)
2. Fish resource (value, spawns)
3. Fisher profession (work schedule, output)
4. Fishing rod item (cost, stats)
5. Fisherman NPCs (teacher, workers, vendor)
6. Fishing locations (docks, coastal rooms)
7. Fish recipes (grilled fish, stew)
8. Trade connections (sells to cooks, innkeepers)

### 3. Leader AI (leader_ai.h)
**Purpose**: Intelligent area leadership with strategic decision-making

**Features**:
- Smart area classification (city/town/village/wilderness/dungeon)
- Leader types (king, mayor, chieftain, archmage, elder)
- Personality system (aggressive, diplomatic, greedy, wise)
- Strategic actions (BUILD, TRADE, RECRUIT, WAR, DIPLOMACY)
- Relationship system between leaders
- AI-powered decision-making (every ~20 minutes)

**Key Point**: NOT all areas get leaders
- DarkHaven → King (major city)
- Village → Mayor/Elder (settlement)
- Orc Camp → Chieftain (tribal)
- Dark Forest → NO LEADER (wilderness)
- Dragon Lair → NO LEADER (dungeon)

### 4. World Context (world_context.h)
**Purpose**: Deep congruence checking before creating anything

**Features**:
- Geographic analysis (mountains, forests, coasts, elevation)
- Political control (sphere of influence, vassals)
- Economic interdependencies (trade routes, resources)
- Historical context (area lore, events)
- Cultural themes (medieval, tribal, magical)
- Player action tracking
- Relationship networks
- AI-powered analysis

**Congruence Checking**:
Before creating ANYTHING, Beeler asks:
1. Geography: Does terrain support it? (fishing needs water)
2. Politics: Who controls this area? (DarkHaven guards in DH mines)
3. Economy: Does it fit trade networks? (fish needed by cooks)
4. Culture: Does it fit theme? (fishing fits medieval port)
5. Lore: Does it match area description? (area mentions harbor)
6. Dependencies: Are prerequisites met? (coastal rooms exist)

## Implementation Phases

### FASE 4A: Boot-Time World Initialization
**When**: Called from boot_db() after areas loaded
**What**: Scan and analyze entire world

**Steps**:
1. `beeler_build_full_world_context()` - Analyze all areas
   - Scan geography (terrain, elevation, water)
   - Detect political structures
   - Build economic model
   - Parse area lore
   - Map relationships

2. `beeler_initialize_all_leaders()` - Find/create leaders
   - Classify each area (city/town/wilderness)
   - Detect existing leaders (kings, mayors)
   - Create missing leaders where appropriate
   - Assign AI and personalities

3. `beeler_detect_missing_infrastructure()` - Find gaps
   - Areas without homes
   - Cities without markets
   - Mountains without mines
   - Coasts without fishing
   - Missing resource types

4. `beeler_create_missing_infrastructure()` - Fill CRITICAL gaps
   - Only essential missing pieces
   - Not everything - just what's needed NOW
   - Validate each with congruence check

**Output**:
```
BEELER: Initializing autonomous world...
BEELER: Scanned 45 areas, found 3 major cities, 7 towns, 12 wilderness areas
BEELER: Initialized 10 leaders (3 kings, 4 mayors, 2 chieftains, 1 archmage)
BEELER: Detected missing infrastructure: 3 mining areas, 2 fishing spots, 1 market
BEELER: Created critical infrastructure (validated)
BEELER: Autonomous world initialized. Life begins.
```

### FASE 4B: Leader Strategic AI
**When**: Called every ~20 minutes from update.c
**What**: Leaders make decisions and act

**Steps**:
1. `leader_ai_update()` - Check all leaders
2. For each leader:
   - Build context (treasury, military, relations, threats)
   - AI generates decision (via Ollama)
   - Parse action (BUILD/TRADE/RECRUIT/WAR/etc)
   - Validate with world context
   - Delegate to Beeler for execution
   - Announce if important

**Example Decision Flow**:
```
King Aldric of DarkHaven:
- Treasury: 50000 gold
- Military: 150 guards
- Relations: Allied with Midgaard, Hostile to Orc Camp
- Threats: Orc raids from north
- Opportunities: Mining village nearby

AI Decision: "BUILD northern_fortress to defend against orc raids"

Validation:
- Geography: Northern border is mountainous (good for fortress) ✓
- Politics: King has authority to build ✓
- Economy: Can afford 5000g cost ✓
- Culture: Medieval military fortress fits theme ✓
- Lore: Area mentions northern border threats ✓

Execute:
- Generate 10-room fortress district
- Add 20 guard NPCs
- Create armory, barracks, walls
- Connect to DarkHaven
- Deduct 5000g from treasury

Announce:
- Priority: HIGH (war-related)
- Chat: "King Aldric Orders Construction of Northern Fortress"
- Periodicos: Full article with details
```

### FASE 4C: Organic Content Creation
**When**: Called every ~30 minutes from update.c
**What**: Detect needs and create entire ecosystems

**Steps**:
1. `beeler_check_for_world_needs()` - Scan for needs
   - Economic: Resources with no source
   - Geographic: Opportunities (coast without fishing)
   - Balance: Too much of X, need Y
   - Player: Immortal requested feature

2. For each need:
   - Create creation request
   - Validate (6-stage congruence check)
   - AI generates creation plan
   - Find best location
   - Build entire ecosystem
   - Announce selectively

**Example - Fish Need**:
```
DETECTION:
- Cook profession requires fish
- No fish resource exists
- No fishing skill exists
- No fisher profession exists

VALIDATION:
- Geography: DarkHaven has docks (coastal) ✓
- Economy: City needs food source ✓
- Culture: Medieval port (fishing fits) ✓
- Lore: Area description mentions harbor ✓

CREATION PLAN (AI-generated):
1. Create fishing skill (min level 1, easy)
2. Create fish resource (5g value)
3. Create fishing rod item (20g, sold by fisherman)
4. Create fisher profession (works coastal, 6am-6pm)
5. Create Old Fisherman NPC (teacher, vendor)
6. Create 8 fisher NPCs (workers)
7. Create Fishmonger NPC (buys fish)
8. Create 3 rooms (docks, shore, fish market)
9. Add fish to cook recipes (grilled fish, stew)
10. Create trade route (fishers → fishmonger → cooks)

EXECUTION:
- Build all components
- Test interconnections
- Validate balance

ANNOUNCEMENT:
- Priority: MEDIUM (new industry)
- Periodicos only: "Fishing Industry Established in DarkHaven"
- No chat spam
```

### FASE 4D: Intelligent Resource Distribution
**When**: Boot-time and periodic checks
**What**: Distribute resources based on geography + politics

**Principles**:
- Mountains → mines (ore, coal, gems)
- Forests → lumber, herbs
- Plains → farms (grain, livestock)
- Coast → fishing
- Desert → exotic (spices, rare minerals)
- NOT all areas get everything

**Political Integration**:
- Capital's sphere of influence = 50 rooms
- Satellite settlements (mining villages near capitals)
- Guards from controlling power
- Resources flow to capital via trade

**Example - Mining Village**:
```
ANALYSIS:
- DarkHaven (major city) at vnum 1000
- Mountain Pass (mountains) at vnum 1500 (45 rooms north)
- Within DarkHaven's sphere (< 50 rooms)
- Mountains have iron ore, coal

CREATION:
- Mining village (15 rooms)
- Village Elder (reports to King Aldric)
- 10 miner NPCs (profession: miner)
- 5 DarkHaven guard NPCs (vnum 1000 series)
- Mine produces: iron ore (3/hour), coal (2/hour)
- Trade route: Daily caravan to DarkHaven
- Cultural markers: DarkHaven banners, dialogue mentions king

ANNOUNCEMENT:
- Priority: MEDIUM
- Periodicos: "Mining Settlement Established in Mountain Pass"
- "Under the authority of King Aldric..."
```

### FASE 4E: Selective Announcement System
**When**: Any significant event
**What**: Decide if/how to announce

**Priority Decision Tree**:
```
Is it CRITICAL? (war, disaster)
  → YES: World chat immediately + periodicos
  → NO: Continue...

Is it HIGH? (major construction, trade, diplomacy)
  → YES: Check spam filter
    → < 6 announcements last hour: World chat + periodicos
    → >= 6 announcements last hour: Periodicos only
  → NO: Continue...

Is it MEDIUM? (new industry, minor events)
  → YES: Periodicos only
  → NO: Continue...

Is it LOW? (NPC daily life)
  → YES: Periodicos archive only
  → NO: SILENT (no announcement)
```

**Spam Prevention**:
- Max 6 announcements per hour in world chat
- Duplicate detection (don't announce same thing twice)
- Importance decay (repeated events become less important)

### FASE 4F: Runtime Autonomous Decisions
**When**: Every ~30 minutes
**What**: Beeler observes world and acts

**Decision Loop**:
```c
void beeler_autonomous_think(void)
{
    // Update world context (incremental)
    update_world_context();

    // Check leaders (they make their own decisions)
    leader_ai_update();

    // Check for economic needs
    beeler_check_for_world_needs();

    // Check for geographic opportunities
    for each area:
        if (area_has_unused_potential(area))
            suggest_and_maybe_create();

    // Check for balance issues
    if (world_balance_problem_detected())
        plan_solution();

    // Roll for autonomous action (10% chance)
    if (number_percent() < 10)
        beeler_make_autonomous_decision();
}
```

## File Structure

New files created:
```
Dev/src/periodicos.h           - Announcement filtering system
Dev/src/organic_creation.h     - Ecosystem creation framework
Dev/src/leader_ai.h             - Strategic leader AI
Dev/src/world_context.h         - Congruence checking engine
Dev/src/beeler_architect.h      - Enhanced (FASE 4 integration)
```

Implementation files (to be created):
```
Dev/src/periodicos.c
Dev/src/organic_creation.c
Dev/src/leader_ai.c
Dev/src/world_context.c
Dev/src/beeler_architect.c      - Will include FASE 4 functions
```

## Integration Points

### In boot_db() (src/db.c)
Add after area loading:
```c
/* FASE 4A: Initialize autonomous world */
beeler_initialize_autonomous_world();
```

### In update.c (violence_update or similar)
Add to game loop:
```c
/* FASE 4B: Leader AI decisions (every ~20 minutes) */
if (pulse_point % (20 * PULSE_PER_SECOND * 60) == 0)
    leader_ai_update();

/* FASE 4C: Beeler autonomous actions (every ~30 minutes) */
if (pulse_point % (30 * PULSE_PER_SECOND * 60) == 0)
    beeler_autonomous_think();
```

### In economy.c
Add need detection:
```c
/* When resource shortage detected */
if (resource_demand > resource_supply * 2)
{
    /* Trigger organic creation */
    trigger_organic_creation(resource_name, TRIGGER_ECONOMIC_NEED);
}
```

## Compilation Order

Due to dependencies:
1. world_context.h/.c (no dependencies)
2. periodicos.h/.c (no dependencies)
3. leader_ai.h/.c (depends on world_context, periodicos)
4. organic_creation.h/.c (depends on world_context, periodicos)
5. beeler_architect.h/.c (depends on all above)

## Testing Plan

### Phase 1: World Scan
- Boot MUD
- Check log for "BEELER: Initializing autonomous world..."
- Verify area classifications (city/town/wilderness)
- Check leader detection

### Phase 2: Leader AI
- Let MUD run for 20+ minutes
- Check for leader decisions in log
- Verify announcements (some in chat, some in periodicos only)
- Test `leaders` command

### Phase 3: Organic Creation
- Trigger need (delete fish resource)
- Wait 30 minutes
- Check if fishing ecosystem created
- Verify all components (skill, NPCs, rooms, recipes)

### Phase 4: Congruence Checking
- Attempt bad creation (fishing in desert)
- Verify rejection in log
- Check AI reasoning

### Phase 5: Full Autonomy
- Let MUD run for 24 hours
- Observe autonomous growth
- Check announcement frequency (not spam)
- Verify world remains consistent

## Commands for Testing

```
worldcontext              - Show world analysis
areacontext <area>        - Show specific area analysis
congruence <what> <where> - Test congruence
leaders                   - List all leaders
leaderinfo <leader>       - Leader details
news                      - Recent news
periodicos                - Browse newspapers
organic                   - View organic creations
beeler analyze            - Beeler's world view
```

## Success Criteria

The system is working when:

1. **World scans at boot** ✓
   - All areas classified
   - Leaders initialized
   - Context built

2. **Leaders act autonomously** ✓
   - Make decisions every ~20 minutes
   - Decisions make sense (congruent)
   - Actions executed by Beeler

3. **Needs detected and filled** ✓
   - Missing resources created
   - Entire ecosystems built
   - Everything interconnected

4. **Announcements are smart** ✓
   - Critical events in chat
   - Medium events in periodicos
   - No spam (< 6/hour in chat)

5. **Everything is congruent** ✓
   - No fishing in deserts
   - Resources match geography
   - Politics make sense
   - Culture consistent

6. **World grows organically** ✓
   - New skills emerge when needed
   - Villages appear where logical
   - Trade routes form naturally
   - Balance maintained

## User's Final Request

> "osea piensa tu que podemos hacer para que neta sea un mundo que se mueve solo crece solo, se crean cosas solas.. en una de esas se necesita pescado y beeler decide que puedes pescar en el mar y crea todo lo necesario para ese nuevo skill y quienes pueden y quienes no y toda lo necesario pa que jale, embone y escale"

**Status**: DESIGNED ✓

All systems designed to support this vision. Next step: Implementation.

## Implementation Effort Estimate

**Header files**: Already created ✓
**Implementation files**: Need to create .c files
**Integration points**: Need to modify db.c, update.c
**Testing**: Need comprehensive testing

**Estimated lines of code**:
- periodicos.c: ~800 lines
- organic_creation.c: ~1200 lines
- leader_ai.c: ~1000 lines
- world_context.c: ~1500 lines
- beeler_architect.c updates: ~800 lines
- Integration: ~200 lines

**Total**: ~5500 lines of new C code

**With Ollama integration**: Each AI decision requires HTTP call (already have framework in ollama_integration.h)

## Next Steps

1. Create implementation .c files for each system
2. Integrate with existing codebase (db.c, update.c)
3. Test each phase independently
4. Test full integration
5. Commit and push to branch: `claude/economic-ecosystem-system-TOOR6`

## World Name: Seldeon

The MUD world is called **Seldeon**. All lore, descriptions, and announcements should reference Seldeon as the world name.
