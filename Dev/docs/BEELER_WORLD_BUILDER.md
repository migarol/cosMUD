# Beeler - World Builder & Architect

## El Problema

**Antes tenías razón** - las casas tienen que EXISTIR como rooms reales en las áreas. No puedes simplemente asignar un vnum que no existe.

**El mob de Luskan necesita su casa EN Luskan** - no en algún lugar random.

## La Solución: Beeler Como Arquitecto

Beeler **NO solo asigna homes** - los **CONSTRUYE**.

### Flujo Completo

```
1. Mob necesita casa
   ↓
2. Beeler analiza: "¿Dónde vive este mob?"
   → Si es de Darkhaven → casa en Darkhaven
   → Si es de Luskan → casa en Luskan
   → Si es de Midgaard → casa en Midgaard
   ↓
3. Beeler busca distrito residencial apropiado en esa ciudad
   → ¿Hay distrito de apartments? ¿Hay espacio?
   → ¿Hay distrito de houses? ¿Hay espacio?
   ↓
4. Si NO hay distrito o está lleno:
   → Beeler CREA snapshot
   → Beeler GENERA nuevo distrito (10-20 rooms)
   → Beeler MODIFICA el área file
   → Beeler CONECTA el distrito a la ciudad
   → Beeler GUARDA cambios
   ↓
5. Beeler asigna home_vnum específico al mob
   ↓
6. Beeler GENERA descripción personalizada
   ↓
7. Save y listo
```

## Generación de Distritos

### Ejemplo: Darkhaven Residential Quarter

Beeler analiza Darkhaven:
- Darkhaven vnums: 10000-10999
- Rooms usados: ~800
- Disponibles: ~200

Beeler decide:
```
[BEELER] Creating Residential Quarter in Darkhaven
[BEELER] Vnums 10800-10830 available
[BEELER] Generating 30 rooms:
  - 1 entrance (10800)
  - 20 apartments (10801-10820)
  - 5 houses (10821-10825)
  - 4 manors (10826-10829)
```

### Layout Generado

```
                    [Main Square] (existing)
                          |
                      [10800] Residential Quarter Entrance
                     /    |    \
              [10801] [10802] [10803]  (Apartments - Street Level)
                 |       |       |
              [10804] [10805] [10806]

              ... more apartments ...

              [10821] Large House (2nd floor access)
              [10822] Large House
              ...
```

### Area File Modification

Beeler **modifica directamente** el archivo `darkhaven.are`:

```
#10800
Residential Quarter Entrance~
A wide street opens into a quiet residential area. Simple apartments line
the street to the north and south. Nicer homes can be seen in the distance.
The main square lies to the south.
~
100 0 1
D0
~
~
0 0 10801
D1
~
~
0 0 10802
D3
~
~
0 0 10803
D2
~
~
0 0 10000
S

#10801
A Small Apartment~
[AI-GENERATED DESCRIPTION BASED ON OCCUPANT]
~
100 0 1
D2
~
~
0 0 10800
S

... etc for all 30 rooms ...
```

## Beeler's Immortal Powers

### Command Access

Beeler tiene acceso a **TODOS** los comandos immortal:

```c
void beeler_execute_immortal_command(char *command, char *arguments)
{
    CHAR_DATA *beeler_avatar;

    /* Create virtual Beeler avatar at LEVEL_SUPREME */
    beeler_avatar = create_beeler_avatar();
    beeler_avatar->level = LEVEL_SUPREME;

    /* Log for safety */
    sprintf(log_buf, "[BEELER] Executing: %s %s", command, arguments);
    log_string(log_buf);

    /* Execute via interpreter */
    interpret(beeler_avatar, command);

    /* Cleanup */
    destroy_beeler_avatar(beeler_avatar);
}
```

### Commands Beeler Can Use

**Area Management:**
- `aset` - Modify area properties
- `rset` - Modify room properties
- `oset` - Modify object properties
- `mset` - Modify mob properties
- `redit` - Edit rooms
- `oedit` - Edit objects
- `medit` - Edit mobs

**World Control:**
- `force` - Force mobs to do things
- `goto` - Teleport anywhere
- `at` - Execute commands remotely
- `transfer` - Move mobs/players
- `restore` - Restore mobs/areas

**Information:**
- `mstat` - Check mob stats
- `ostat` - Check object stats
- `rstat` - Check room stats
- `astat` - Check area stats
- `mfind` / `ofind` / `rfind` - Search

**Creation:**
- `mcreate` - Create mob
- `ocreate` - Create object
- `dig` - Create room with exits

## Context Engine

Beeler necesita **acceso rápido** a información:

```c
// Quick stats
beeler_quick_stat("how many mobs in darkhaven")
→ "There are 347 mobs in Darkhaven"

beeler_quick_stat("who is the strongest mob")
→ "Dragon King Zorthax (level 95, vnum 12345)"

beeler_quick_stat("total gold in economy")
→ "There is 15,847,293 gold in the world economy"

// Quick lookups
beeler_find_mob_by_name("Tsythia")
→ Returns CHAR_DATA pointer

beeler_find_room_by_name("Library")
→ Returns ROOM_INDEX_DATA pointer

beeler_count_mobs_in_area("Darkhaven")
→ Returns 347
```

### Context Cache

Beeler mantiene un cache actualizado:

```c
typedef struct beeler_context {
    /* Updated every 5 minutes */
    int total_mobs;
    int total_areas;
    int total_rooms;
    int total_players_online;

    /* Hash tables for fast lookup */
    void *mob_index_hash;      /* O(1) mob lookup */
    void *room_index_hash;     /* O(1) room lookup */
    void *area_index_hash;     /* O(1) area lookup */

    /* Recent activity (last 10) */
    char *recent_kills[10];
    char *recent_deaths[10];
    char *recent_logins[10];

    /* Economic data */
    int total_gold_in_world;
    int avg_player_gold;
    int total_shops;

    time_t last_cache_update;
} BEELER_CONTEXT;
```

## Interaction Flow

### Example: Player Requests Housing

```
Player: talk beeler can you build more housing in Darkhaven?

Beeler analyzes:
  - Checks Darkhaven for available vnums
  - Checks existing districts
  - Determines need for new district

Beeler responds:
  "Your request is wise, mortal. Darkhaven grows, and its people need shelter.
   I shall create a new residential quarter."

Beeler acts:
  [1] Creates snapshot: "pre-darkhaven-housing-expansion-2025"
  [2] Finds available vnums: 10800-10830
  [3] Generates 30 rooms (apartments, houses, manors)
  [4] Modifies darkhaven.are file
  [5] Creates connection from Main Square to new district
  [6] Executes: "dig 10800 north"  (from main square)
  [7] Saves changes
  [8] Broadcasts: "A new residential quarter has appeared in Darkhaven!"

Beeler confirms:
  "It is done. The Residential Quarter of Darkhaven now houses 30 new dwellings.
   20 apartments for common folk, 5 houses for merchants, 4 manors for nobility.
   Snapshot 'pre-darkhaven-housing-expansion-2025' created for safety."
```

### Example: Automatic Home Assignment

```
AI God gives life to mob "Guard Captain Marcus" (vnum 10543)

Beeler receives request:
  assign_home_to_mob(Marcus)

Beeler analyzes:
  - Marcus is in Darkhaven (area analysis)
  - Marcus is awareness level 2 (guard captain)
  - Marcus should have a HOUSE (not apartment)

Beeler checks:
  - Searches Darkhaven for existing house district
  - Finds "Officer's Quarter" district
  - Finds available house at vnum 10823

Beeler assigns:
  - Sets Marcus->home_vnum = 10823
  - Generates personalized description:
    "This sturdy house reflects military discipline. Weapons hang
     on the walls in precise rows. A desk holds patrol schedules.
     Through the window, you can see the main gate."

Beeler saves:
  - Saves to ../data/mob_homes/10543.json

Log:
  "[BEELER] Assigned Guard Captain Marcus a House in Darkhaven (vnum 10823)"
```

## Safety Mechanisms

### 1. Always Snapshot

```c
bool beeler_modify_area(int area_vnum, char *modification_type)
{
    WORLD_SNAPSHOT *snapshot;

    /* MANDATORY snapshot before ANY modification */
    snapshot = create_snapshot(
        "pre-area-modification",
        modification_type,
        "Beeler"
    );

    if (!snapshot) {
        log_string("[BEELER] ABORT: Could not create snapshot");
        return FALSE;
    }

    /* Now safe to modify */
    return TRUE;
}
```

### 2. Validation

```c
bool beeler_validate_generated_area(DISTRICT_GENERATION *district)
{
    /* Check vnums don't conflict */
    for each room in district:
        if get_room_index(vnum) exists:
            ERROR: vnum already in use

    /* Check connections are valid */
    if district->connect_to_vnum:
        if !get_room_index(connect_to_vnum):
            ERROR: connection target doesn't exist

    /* Check area boundaries */
    if vnums outside area range:
        ERROR: vnums exceed area limits

    return all_checks_passed;
}
```

### 3. Rollback on Failure

```c
if !beeler_build_district(district):
    log_string("[BEELER] Build failed - restoring snapshot")
    restore_snapshot(last_snapshot_name)
    return FALSE
```

## Commands for Immortals

### Build Commands
```
beeler_build district <area> <type> <count>
  → Builds residential district with N homes

beeler_build inn <area> <name>
  → Creates inn with rooms

beeler_build home <mob_vnum>
  → Creates personalized home for specific mob
```

### Analysis Commands
```
beeler_analyze <area>
  → Shows available vnums, current occupancy, potential for expansion

beeler_context <query>
  → Quick stats: "mobs in darkhaven", "available vnums in midgaard"
```

### Management Commands
```
beeler_stats
  → Shows Beeler's activity, modifications made, snapshots created

snapshot list
  → Lists all snapshots

snapshot restore <name>
  → Restores world to snapshot
```

## Integration with Existing Systems

### Mob Identity System
```c
void generate_mob_identity(CHAR_DATA *mob)
{
    MOB_IDENTITY *identity;
    MOB_HOME *home;

    /* Generate personality (existing) */
    identity = create_identity_with_context(mob);

    /* NEW: Assign home via Beeler */
    int home_vnum = beeler_assign_home_intelligently(mob);

    if (home_vnum > 0) {
        home = create_mob_home(mob->vnum, home_vnum, ...);
        identity->home_vnum = home_vnum;
    }
}
```

### World Simulation
```c
void execute_mob_routine(CHAR_DATA *mob)
{
    int routine = get_mob_routine(mob);

    if (routine == ROUTINE_SLEEP) {
        /* Go home to sleep */
        mob_go_home(mob);  /* Uses pathfinding to home_vnum */
    }
}
```

## Future: AI-Generated Content

### Room Descriptions
```python
# Beeler calls Ollama
prompt = f"""
Generate a detailed room description for:
- Home type: {home_type}
- Owner: {mob_name}
- Personality: {mob_personality}
- City: {city_name}

Make it UNIQUE and PERSONAL. Include:
1. Overall atmosphere
2. Furnishings
3. Personal touches
4. Sensory details (smell, light, sound)

Maximum 4 paragraphs.
"""

description = ollama.generate(prompt)
```

### District Themes
```python
prompt = f"""
Generate a thematic residential district for {city_name}:
- District type: {home_type_name}
- Number of homes: {count}
- Architectural style: {city_style}

Provide:
1. District name
2. Entrance description
3. Street layout (grid, winding, etc)
4. Overall aesthetic
"""
```

## Status

### Implemented ✅
- Data structures for room generation
- District generation framework
- Context cache system
- Vnum finding algorithm
- Area analysis
- Home type determination
- City determination based on mob location

### To Implement 🔲
- Actual area file modification
- Room insertion into existing areas
- Exit creation between rooms
- Area file backup/restore
- Immortal command execution framework
- AI-generated descriptions
- District building pipeline
- Inn generation

### To Test 🧪
- Vnum conflict detection
- Area boundaries validation
- Snapshot/restore cycle
- Multi-district generation
- Connection to existing city

---

**La visión completa**: Un mob nace → AI God le da personalidad → Beeler analiza dónde vive → Beeler encuentra o CREA su casa EN ESA CIUDAD → El mob ahora tiene un hogar real, en un lugar real, que realmente existe.

No más homes falsos. Todo es real. Todo existe. Beeler construye el mundo, ladrillo por ladrillo. 🏗️
