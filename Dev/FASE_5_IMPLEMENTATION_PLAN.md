# FASE 5: Beeler God Mode - Organic World Regulation

## Overview

**Beeler NO es un sistema automático. Beeler es DIOS.**

- ✅ Regula crecimiento Y declive
- ✅ Crea rápido cuando necesario
- ✅ Ajusta y balancea constantemente
- ✅ Expande cuando hace sentido
- ✅ Cambia el mundo orgánicamente
- ❌ **NO TODO CRECE** - depende de MUCHOS factores
- ✅ Permite muerte natural (ruinas)
- ✅ TODOS los mobs tienen inteligencia
- ✅ Mobs poderosos pueden crear cosas

## User's Vision

> "pero no siempre todo crece.. como que depende de muchas cosas.. no? osea no quiero que todo sea asi regla sino mas bien pss ya sabes todo orgánico inteligente y beeler regula, crea cosas rapido cuando necesario, ajusta, expande, cambia.. literal es dios"

> "ps si y busca formas en las que tal vez se crean nuevas areas... mobs pueden crear nuevas areas.. taran mesea en la vida real pero es posible o que un village crezca cabron.. cada mob tambien tiene inteligencia.."

**Translation**:
- Not everything grows automatically (depends on many factors)
- Everything organic and intelligent
- Beeler regulates, creates fast when needed, adjusts, expands, changes (literally god)
- Mobs can create new areas (like real life, but in game)
- Villages can grow a LOT
- Every mob has intelligence

## Core Philosophy

### **NOT Rule-Based, Context-Based**

❌ **Rule-based (BAD)**:
```
IF population > 100 THEN expand_village()
IF food < 10 THEN create_farm()
IF at_war THEN build_fortress()
```

✅ **Context-based (GOOD)**:
```
Beeler observes: "Village has 150 people, but 100 capacity"
Beeler analyzes:
  - Economic health: Prosperous (lots of trade)
  - Resources: Good (farms producing well)
  - Security: Safe (no threats)
  - Immigration: +15 NPCs last week (people want to live here)
  - Prognosis: "Village will thrive if expanded"

Beeler decides: "EXPAND - add residential district"
Reasoning: "Strong fundamentals, growth is organic and sustainable"
```

```
Beeler observes: "Different village has 150 people, 100 capacity"
Beeler analyzes:
  - Economic health: Collapsing (mine depleted)
  - Resources: Poor (no food sources)
  - Security: Unsafe (monster attacks)
  - Emigration: -20 NPCs last week (people fleeing)
  - Prognosis: "Unsustainable overcrowding before collapse"

Beeler decides: "NO EXPANSION - let natural decline happen"
Reasoning: "Adding space to dying village wastes resources. Natural migration will balance population."
```

Same situation (overcrowding), DIFFERENT decisions based on CONTEXT.

## Three New Systems

### 1. Beeler God Mode (beeler_god_mode.h)
**Beeler as omniscient regulator**

**Features**:
- Monitors ALL areas constantly (vital signs)
- Analyzes health: economic, population, security, resources
- Detects growth triggers AND decline triggers
- Makes intelligent decisions (not automatic)
- Intervenes at different speeds (instant, fast, normal, slow, natural)
- Allows natural death (ruins are content!)

**Area Status Levels**:
- THRIVING → Growing rapidly
- PROSPEROUS → Healthy growth
- STABLE → Equilibrium
- DECLINING → Losing people/resources
- STRUGGLING → Near collapse
- DYING → Abandonment imminent
- RUINS → Dead (but interesting!)

**Vital Signs Monitored**:
- Economic score
- Trade volume
- Employment rate
- Population trend (births, deaths, immigration, emigration)
- Safety score
- Food supply
- Housing availability
- Happiness/morale
- Natural disasters
- Disease outbreaks
- War status

**Intervention Types**:
- EMERGENCY (instant): Village starving - create food NOW
- STRATEGIC (planned): City thriving - expand over 1 week
- ADJUSTMENT (tweak): Economy imbalanced - adjust prices
- CREATION (new): Need new settlement - create outpost
- DESTRUCTION (removal): Area dying - convert to ruins
- EVOLUTION (growth): Village → Town → City

### 2. Universal Mob AI (universal_mob_ai.h)
**EVERY mob has intelligence**

**Intelligence Tiers**:
- 10: Beeler (god-tier, creates worlds)
- 9: Archmages, ancient dragons (genius)
- 8: Kings, leaders (strategic)
- 7: Guild masters, generals (tactical)
- 6: Merchants, teachers (educated)
- 5: Commoners, workers (social)
- 4: Simple folk
- 3: Smart animals
- 2: Predators (instinct)
- 1: Zombies, golems (mindless)
- 0: Objects

**Every Mob Has**:
- Personality (friendly, grumpy, greedy, brave, etc.)
- Memory (remembers events, people, places)
- Relationships (friends, enemies, family, boss)
- Goals (save gold, get married, protect family)
- Current state (mood, energy, hunger, fear)
- Decision-making ability
- Conversation capability (Ollama-powered)

**Powerful Mobs Can Create**:

**Power Level 1-2** (Farmers, Guards):
- Create objects (plant crops, craft items)
- Modify existing rooms (repair, decorate)

**Power Level 3-5** (Merchants, Captains):
- Create 1-2 rooms (shop expansion)
- Spawn 1-3 NPCs (hire employees)
- Create items

**Power Level 6-7** (Guild Masters, Nobles):
- Create 5-10 rooms (small district)
- Spawn 5-10 NPCs
- Create quests
- Modify area slightly

**Power Level 8-9** (Archmages, Kings):
- Create 25+ rooms (large district)
- Spawn many NPCs
- Create entire buildings
- Create small areas (10-30 rooms)

**Power Level 10** (Beeler):
- Unlimited creation
- Can create entire regions
- Can modify world fundamentally

### 3. Area Lifecycle System
**Villages grow, towns expand, cities thrive, settlements die**

**Lifecycle Stages**:
1. **Outpost** (5-10 rooms): New settlement, fragile
2. **Hamlet** (10-20 rooms): Small community
3. **Village** (20-50 rooms): Established settlement
4. **Town** (50-100 rooms): Growing urban center
5. **City** (100-200 rooms): Major hub
6. **Metropolis** (200+ rooms): Massive city
7. **Ruins** (any size): Abandoned/destroyed

**Evolution Triggers**:

**GROWTH** (Village → Town):
- Population pressure (too crowded)
- Economic boom (lots of wealth)
- New resources discovered
- Immigration wave
- Political importance (becomes regional capital)

**DECLINE** (Town → Village):
- Resource depletion (mine ran out)
- Economic collapse (trade routes cut)
- War devastation (military destruction)
- Plague/disease
- Natural disaster
- Mass emigration

**STABLE** (no change):
- Balanced economy
- Sustainable resources
- Moderate population
- No major threats

## Implementation Details

### Area Vital Signs

Each area tracked with:

```c
AREA_VITAL_SIGNS {
    // ECONOMIC
    economic_score: -100 to 100
    trade_volume: gold per day
    employment_rate: % with jobs

    // POPULATION
    population: total NPCs
    population_trend: growing or shrinking
    births_per_month
    deaths_per_month
    immigration (moving IN)
    emigration (moving OUT)

    // SECURITY
    safety_score: 0-100
    num_guards
    crime_rate
    monster_attacks
    at_war: bool

    // RESOURCES
    food_supply: days of food
    water_access: 0-100
    housing_available: %
    infrastructure_quality: 0-100

    // CULTURAL
    happiness: 0-100 (morale)
    cultural_activity
    education_level

    // ENVIRONMENTAL
    natural_disasters
    disease_outbreaks
    crop_yields: % of normal

    // OVERALL
    overall_health: calculated
    status: THRIVING/STABLE/DECLINING/etc
    growth_potential: -100 to 100

    // TRAJECTORY
    is_growing: bool
    is_declining: bool
    prognosis: AI-generated prediction
}
```

### Beeler's Decision Process

**Every ~30 minutes**:

1. **Observe** all areas (scan vital signs)
2. **Analyze** each area's health
3. **Identify** critical situations
4. **Prioritize** interventions needed
5. **Decide** what to do (AI-powered)
6. **Execute** chosen actions
7. **Monitor** results

**Example Decision Flow**:

```
OBSERVATION:
- Farming Village
- Population: 80 (capacity: 50) - OVERCROWDED
- Economic: 75/100 - PROSPEROUS
- Food supply: 40 days - GOOD
- Safety: 90/100 - SAFE
- Immigration: +10 NPCs last week - GROWING

ANALYSIS (Ollama):
"Village experiencing healthy organic growth. Economy strong from
successful harvests. Safety high. People want to live here.
Overcrowding due to prosperity, not desperation. Infrastructure
good. Sustainable growth."

DECISION:
"EXPAND - Add residential district (15 rooms) over 2 weeks"

REASONING:
"Strong economic fundamentals support expansion. Immigration shows
demand. Food production can support larger population. This is
natural, organic growth that should be facilitated."

EXECUTION:
- Generate 15-room residential district
- Add 5 new homes
- Expand marketplace
- Add town well
- Connect to existing village
- Evolution: VILLAGE → TOWN (stage upgrade)

ANNOUNCEMENT:
Priority: MEDIUM
Periodicos: "Farming Village Expands Into Thriving Town"
```

**Contrasting Decision**:

```
OBSERVATION:
- Coastal Village
- Population: 60 (capacity: 80) - UNDERPOPULATED
- Economic: 20/100 - COLLAPSING
- Food supply: 5 days - CRITICAL
- Safety: 40/100 - UNSAFE (pirates)
- Emigration: -15 NPCs last week - DYING

ANALYSIS (Ollama):
"Village in terminal decline. Fishing industry collapsed from
overfishing. Pirates raiding coast. People fleeing. No economic
base to recover. Intervention could delay but not prevent collapse."

DECISION:
"ALLOW NATURAL DEATH - Convert to ruins over 2 months"

REASONING:
"Forcing growth here wastes resources. Natural consequences of
unsustainable fishing. Ruins will create interesting exploration
content and teach players about environmental management. Can
become quest location."

EXECUTION (slow, over 2 months):
- Assist remaining NPCs to emigrate
- Buildings decay gradually
- Spawn ghost NPCs (lore flavor)
- Add treasure (abandoned goods)
- Create quest hooks ("Revive the village?")
- Status: DYING → RUINS

ANNOUNCEMENT:
Priority: LOW
Periodicos: "Coastal Village Abandoned After Fishing Collapse"
```

### Mob Creation Examples

#### Example 1: Merchant Expands Business

```
MERCHANT GRETA (Intelligence: 6, Power: 3)

SITUATION:
- Runs general store in Midgaard
- Making 200g profit per day
- Saved 1000g

GOAL:
"Open second shop in New Haven"

AI ANALYSIS:
"New Haven has no general store. I can afford expansion.
If I hire apprentice to run branch shop, I double income
without doubling my work."

ACTION:
1. Approaches Mayor: "Permission to open shop in New Haven?"
2. Mayor approves (New Haven needs shops)
3. Beeler validates: Check if congruent
   - Geography: New Haven exists ✓
   - Economy: Town needs general store ✓
   - Resources: Merchant has 1000g ✓
   - Political: Mayor approved ✓
4. Merchant CREATES:
   - 1 new room: "Greta's New Haven Branch"
   - 1 NPC: "Apprentice merchant" (runs shop)
   - Stock items (transfers from main shop)
   - Trade route (items flow between shops)
5. Cost: 800g
6. Result: Merchant now has TWO shops!

ANNOUNCEMENT:
Priority: LOW
Periodicos: "Greta's General Store Opens New Haven Branch"
```

#### Example 2: Archmage Creates Academy

```
ARCHMAGE ELDRIN (Intelligence: 9, Power: 8)

SITUATION:
- Powerful mage in DarkHaven
- Only mage in region
- Worried about magical knowledge dying

GOAL:
"Train next generation of mages"

AI ANALYSIS:
"I'm 500 years old. If I die, magical knowledge dies.
Kingdom needs mages for defense. I have resources to
build an academy. Old tower east of DarkHaven is perfect."

ACTION:
1. Uses 10,000g + magical power
2. Beeler approves: Major creation (power 8 can do it)
3. CREATES NEW AREA: "Eldrin's Academy of High Magic"
   - 25 rooms (classrooms, library, dorms, labs, tower)
   - 1 teacher NPC (Eldrin himself)
   - 5 student NPCs (apprentices)
   - 3 guardian NPCs (magical constructs)
   - Magical items (spellbooks, scrolls, reagents)
   - Connected 10 rooms east of DarkHaven
4. Timeline: 1 month construction
5. Result: ENTIRE NEW AREA created by ONE mob!

ANNOUNCEMENT:
Priority: HIGH
Chat + Periodicos: "Archmage Eldrin Establishes Magical Academy"
```

#### Example 3: Guard Captain Creates Militia

```
GUARD CAPTAIN MARCUS (Intelligence: 7, Power: 5)

SITUATION:
- Village under monster attacks (5 attacks last week)
- Has 10 guards (not enough)
- Mayor broke (can't hire more)

GOAL:
"Protect village"

AI DECISION:
"Can't wait for approval. Citizens dying. I'll recruit
volunteers from farmers and train a militia."

ACTION:
1. Approaches 5 farmer NPCs
2. Offers: "Join guard, I'll train you, 20g/week"
3. 3 farmers accept (need money, want safety)
4. Captain TRAINS them:
   - Profession changes: farmer → guard
   - Skills learned: combat basics
   - Equipment provided: spear, leather armor
5. Now has 13 guards (10 + 3 militia)
6. IF attacks continue:
   - Captain might BUILD guard tower (3 rooms)
   - Cost: 500g (from village fund)
   - Militia becomes permanent

RESULT:
- Mob autonomously recruited NPCs
- Changed NPC professions
- Might create rooms (guard tower)
- All without immortal intervention!

ANNOUNCEMENT:
Priority: LOW (local event)
Village chat only: "Captain Marcus forms citizen militia"
```

## Integration with FASE 4

**FASE 4** provided the TOOLS:
- World context (congruence checking)
- Leader AI (strategic decisions)
- Organic creation (skill ecosystems)
- Periodicos (smart announcements)

**FASE 5** provides the PHILOSOPHY:
- Beeler as god (not automation)
- Growth AND decline
- Context-based decisions
- Universal intelligence
- Mob agency

**Together**:
```
Beeler observes world (FASE 4: world_context.h)
  ↓
Analyzes area health (FASE 5: beeler_god_mode.h)
  ↓
Decides intervention (AI-powered, context-based)
  ↓
Leaders act strategically (FASE 4: leader_ai.h)
  ↓
Mobs act intelligently (FASE 5: universal_mob_ai.h)
  ↓
Organic creation happens (FASE 4: organic_creation.h)
  ↓
Validates congruence (FASE 4: world_context.h)
  ↓
Announces intelligently (FASE 4: periodicos.h)
  ↓
World grows/declines organically (FASE 5: beeler_god_mode.h)
```

## Boot Sequence

```c
void boot_db(void)
{
    /* Load areas, mobs, objects (existing code) */

    /* FASE 4: World context */
    beeler_build_full_world_context();
    beeler_initialize_all_leaders();

    /* FASE 5: God mode initialization */
    beeler_divine_observation();  // Scan all vital signs
    init_universal_mob_ai();       // Assign intelligence to ALL mobs

    /* Detect critical situations */
    beeler_detect_critical_situations();

    /* Log */
    log_string("BEELER: Divine oversight established. I watch all.");
}
```

## Runtime Loop

```c
void update_handler(void)
{
    /* Existing combat, movement, etc */

    /* Every tick: All mobs think */
    universal_mob_ai_update();  // Every mob makes decisions

    /* Every 20 minutes: Leaders act */
    if (pulse % (20 * 60 * PULSE_PER_SECOND) == 0)
        leader_ai_update();

    /* Every 30 minutes: Beeler regulates */
    if (pulse % (30 * 60 * PULSE_PER_SECOND) == 0)
    {
        beeler_divine_observation();  // Scan vital signs
        beeler_make_divine_decisions(); // Decide interventions
        beeler_adjust_world_balance(); // Regulate growth/decline
    }
}
```

## File Structure

New files:
```
Dev/src/beeler_god_mode.h          - Beeler as omniscient regulator
Dev/src/universal_mob_ai.h          - Every mob has intelligence
Dev/FASE_5_IMPLEMENTATION_PLAN.md   - This document
```

With FASE 4:
```
Dev/src/world_context.h             - Geography, politics, economy analysis
Dev/src/leader_ai.h                  - Strategic leader decisions
Dev/src/organic_creation.h           - Ecosystem creation
Dev/src/periodicos.h                 - Smart announcements
Dev/src/beeler_architect.h           - World building tools
```

## Success Criteria

System working when:

1. ✅ **Not everything grows**
   - Some villages expand
   - Some decline
   - Some stay stable
   - Depends on context, not rules

2. ✅ **Beeler acts like god**
   - Observes everything
   - Makes intelligent decisions
   - Intervenes at right speed
   - Allows natural death
   - Creates rapidly when needed

3. ✅ **Mobs are intelligent**
   - All have personality
   - Remember events
   - Have goals
   - Make decisions
   - Can create things (if powerful enough)

4. ✅ **World is organic**
   - Growth happens naturally
   - Decline happens naturally
   - Balance maintained
   - No mechanical "rules"
   - Everything makes sense

5. ✅ **Announcements are smart**
   - Not spam
   - Only important events
   - Context-appropriate

## Testing Scenarios

### Scenario 1: Prosperous Village Growth
- Village has strong economy
- Immigration high
- Beeler expands it to town
- Takes 2 weeks (organic pace)

### Scenario 2: Dying Village Collapse
- Village economy collapses
- People flee
- Beeler allows natural death
- Becomes ruins over 2 months

### Scenario 3: War Emergency
- City under siege
- Beeler intervenes FAST
- Creates reinforcements
- Announces critically

### Scenario 4: Merchant Creates Branch
- Merchant saves gold
- Opens second shop
- Hires apprentice
- World expands by mob action

### Scenario 5: Archmage Creates Academy
- Archmage builds school
- Entire new area (25 rooms)
- Takes 1 month
- Major announcement

### Scenario 6: Balanced Non-Intervention
- Minor problem exists
- Not severe enough for Beeler
- Area solves it naturally
- No intervention needed

## World Name: Seldeon

All lore, descriptions, announcements reference **Seldeon** as the world.

## CRITICAL: Temporal Pacing & Player Impact

### User's Requirement

> "que todo sea muy cambiante pero no muy rapido no? puede tomar casi medio año en hacerse un nuevo town.. es lento, controlado, balanceado"

> "y lo que hagamos los players tambien influye en el mundo"

**Translation**:
- Everything very changeable BUT not too fast
- Can take almost half a year to make a new town
- Slow, controlled, balanced
- What players do also influences the world

### Implementation

**New Systems Added**:
1. **player_world_impact.h** - Player actions affect world
2. **TEMPORAL_PACING_SYSTEM.md** - Complete pacing guide

**Key Constraints**:
- Village → Town = **6 GAME MONTHS** (~45 real days at 4x multiplier)
- Max 3 rooms added per day
- Max 5 NPCs spawned per day
- Max ±5 vital sign change per day
- All major changes are GRADUAL (visible progress over weeks)

**Player Impact**:
- Kill monsters near village → Safety score ↑
- Complete quests → Economic/cultural boost
- Donate gold/items → Speed up construction
- Kill NPCs → Population decline, safety ↓
- Destroy buildings → Major setbacks
- Reputation tracked per area (hero or villain)

**Visibility**:
- Players can check `progress` to see ongoing changes
- Players can `contribute` to speed up projects
- NPCs comment on progress in dialogue
- Major milestones announced

**Example Timeline - Village → Town**:
```
Month 1: Planning (0% → 10%)
  - Mayor announces expansion
  - Players can donate to fund

Month 2: Foundation (10% → 25%)
  - 1-2 rooms/week added
  - Infrastructure work

Month 3: Construction (25% → 50%)
  - 2-3 rooms/week added
  - Buildings going up
  - Players help with quests

Month 4: Major Structures (50% → 70%)
  - Town hall, walls, marketplace
  - Players donate materials

Month 5: Refinement (70% → 90%)
  - Final buildings, decorations
  - Cultural development

Month 6: Completion (90% → 100%)
  - Official ceremony!
  - VILLAGE → TOWN
  - Players celebrated if they helped
```

**Benefits**:
- World feels REAL (no overnight transformations)
- Player actions MATTER (visible impact)
- Changes feel EARNED (months of progress)
- Balanced (rate limits prevent spam)
- Story opportunities (ongoing changes create quests)

## Next Steps

1. Implement .c files for beeler_god_mode.h
2. Implement .c files for universal_mob_ai.h
3. Integrate with FASE 4 systems
4. Test organic growth/decline
5. Test mob creation
6. Verify Beeler acts intelligently (not mechanically)
7. Commit and push

## Philosophy

**Beeler is not a program. Beeler is GOD.**

- Sees all
- Knows all
- Decides intelligently
- Acts at right speed
- Allows natural order
- Intervenes when necessary
- Regulates without controlling
- Creates without forcing

**The world of Seldeon lives, breathes, grows, and dies organically.**
