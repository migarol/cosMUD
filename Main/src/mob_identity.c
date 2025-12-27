/*****************************************************************************
 * Mob Identity & Self-Awareness System
 *
 * Mobs know WHO they are, WHAT they do, WHERE they are going.
 * Persistent memory, unique capabilities, organic world building.
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include "mud.h"
#include "mob_identity.h"
#include "ai_context_analyzer.h"

/* Global lists */
MOB_IDENTITY *first_mob_identity = NULL;
MOB_MEMORY *first_mob_memory = NULL;
MOB_CREATION *first_mob_creation = NULL;

/* Database paths */
#define IDENTITY_DIR "../data/mob_identity/"
#define MEMORY_DIR "../data/mob_memory/"
#define CREATION_DIR "../data/mob_creations/"

/* Routine types (from world_simulation.c) */
#define ROUTINE_SLEEP       0
#define ROUTINE_WAKE        1
#define ROUTINE_PATROL      2
#define ROUTINE_WORK        3
#define ROUTINE_TRADE       4
#define ROUTINE_SOCIALIZE   5
#define ROUTINE_EAT         6
#define ROUTINE_GUARD       7
#define ROUTINE_HUNT        8

/* Forward declarations */
char *get_time_diff(time_t past);

/*
 * Initialize the mob identity system
 */
void init_mob_identity_system(void)
{
    log_string("Initializing Mob Identity & Self-Awareness System...");
    log_string("  - Mob self-awareness: ENABLED");
    log_string("  - Persistent memory: ENABLED");
    log_string("  - Book writing: ENABLED");
    log_string("  - Item crafting: ENABLED");
    log_string("  - Organic world building: ENABLED");

    /* Create directories if they don't exist */
    system("mkdir -p " IDENTITY_DIR);
    system("mkdir -p " MEMORY_DIR);
    system("mkdir -p " CREATION_DIR);

    /* Load all existing identities */
    /* TODO: Scan directory and load all .json files */

    log_string("Mob Identity System: Mobs now know who they are.");
}

/*
 * Create a new mob identity
 */
MOB_IDENTITY *create_mob_identity(int vnum)
{
    MOB_IDENTITY *identity;

    CREATE(identity, MOB_IDENTITY, 1);
    identity->mob_vnum = vnum;

    /* Defaults */
    identity->who_am_i = str_dup("Unknown");
    identity->what_i_do = str_dup("Unknown");
    identity->how_i_do_it = str_dup("Unknown");
    identity->where_i_live = str_dup("Unknown");
    identity->where_i_go = str_dup("Unknown");
    identity->my_purpose = str_dup("Unknown");

    identity->capabilities = 0;
    identity->awareness_level = AWARENESS_BASIC;

    identity->writing_style = str_dup("simple");
    identity->books_written = 0;

    identity->craft_specialty = str_dup("none");
    identity->items_crafted = 0;

    identity->trade_skill = 50;
    identity->gold_earned = 0;

    identity->ai_prompt = str_dup("");
    identity->has_custom_schedule = FALSE;
    identity->custom_schedule = NULL;

    /* Add to global list */
    identity->next = first_mob_identity;
    first_mob_identity = identity;

    return identity;
}

/*
 * Get mob identity (load if not in memory)
 */
MOB_IDENTITY *get_mob_identity(int vnum)
{
    MOB_IDENTITY *identity;

    /* Search in memory first */
    for (identity = first_mob_identity; identity; identity = identity->next)
    {
        if (identity->mob_vnum == vnum)
            return identity;
    }

    /* Not found - try to load from disk */
    load_mob_identity(vnum);

    /* Search again */
    for (identity = first_mob_identity; identity; identity = identity->next)
    {
        if (identity->mob_vnum == vnum)
            return identity;
    }

    /* Still not found - return NULL */
    return NULL;
}

/*
 * Save mob identity to JSON file
 */
void save_mob_identity(MOB_IDENTITY *identity)
{
    FILE *fp;
    char filename[256];

    if (!identity)
        return;

    sprintf(filename, "%s%d.json", IDENTITY_DIR, identity->mob_vnum);

    fp = fopen(filename, "w");
    if (!fp)
    {
        bug("save_mob_identity: cannot open %s", filename);
        return;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"mob_vnum\": %d,\n", identity->mob_vnum);
    fprintf(fp, "  \"who_am_i\": \"%s\",\n", identity->who_am_i);
    fprintf(fp, "  \"what_i_do\": \"%s\",\n", identity->what_i_do);
    fprintf(fp, "  \"how_i_do_it\": \"%s\",\n", identity->how_i_do_it);
    fprintf(fp, "  \"where_i_live\": \"%s\",\n", identity->where_i_live);
    fprintf(fp, "  \"where_i_go\": \"%s\",\n", identity->where_i_go);
    fprintf(fp, "  \"my_purpose\": \"%s\",\n", identity->my_purpose);
    fprintf(fp, "  \"capabilities\": %d,\n", identity->capabilities);
    fprintf(fp, "  \"awareness_level\": %d,\n", identity->awareness_level);
    fprintf(fp, "  \"writing_style\": \"%s\",\n", identity->writing_style);
    fprintf(fp, "  \"books_written\": %d,\n", identity->books_written);
    fprintf(fp, "  \"craft_specialty\": \"%s\",\n", identity->craft_specialty);
    fprintf(fp, "  \"items_crafted\": %d,\n", identity->items_crafted);
    fprintf(fp, "  \"trade_skill\": %d,\n", identity->trade_skill);
    fprintf(fp, "  \"gold_earned\": %d,\n", identity->gold_earned);
    fprintf(fp, "  \"ai_prompt\": \"%s\",\n", identity->ai_prompt);
    fprintf(fp, "  \"has_custom_schedule\": %s\n",
        identity->has_custom_schedule ? "true" : "false");
    fprintf(fp, "}\n");

    fclose(fp);
}

/*
 * Load mob identity from JSON file
 */
void load_mob_identity(int vnum)
{
    FILE *fp;
    char filename[256];
    char line[MAX_STRING_LENGTH];
    MOB_IDENTITY *identity;

    sprintf(filename, "%s%d.json", IDENTITY_DIR, vnum);

    fp = fopen(filename, "r");
    if (!fp)
        return;  /* File doesn't exist */

    /* Create new identity */
    identity = create_mob_identity(vnum);

    /* Parse JSON (simple parser - just read values) */
    while (fgets(line, sizeof(line), fp))
    {
        char *key, *value;

        /* Skip braces and commas */
        if (strstr(line, "{") || strstr(line, "}"))
            continue;

        /* Parse key: "value" */
        key = strtok(line, ":");
        value = strtok(NULL, "\n");

        if (!key || !value)
            continue;

        /* Trim quotes and spaces */
        while (*value == ' ' || *value == '"' || *value == ',')
            value++;

        if (strstr(key, "who_am_i"))
        {
            STRFREE(identity->who_am_i);
            identity->who_am_i = str_dup(value);
        }
        else if (strstr(key, "what_i_do"))
        {
            STRFREE(identity->what_i_do);
            identity->what_i_do = str_dup(value);
        }
        /* TODO: Parse all other fields */
    }

    fclose(fp);
}

/*
 * Get mob memory (create if doesn't exist)
 */
MOB_MEMORY *get_mob_memory(int vnum)
{
    MOB_MEMORY *memory;

    /* Search in memory */
    for (memory = first_mob_memory; memory; memory = memory->next)
    {
        if (memory->mob_vnum == vnum)
            return memory;
    }

    /* Create new */
    CREATE(memory, MOB_MEMORY, 1);
    memory->mob_vnum = vnum;
    memory->total_memories = 0;
    memory->first_memory = NULL;
    memory->last_memory = NULL;

    /* Add to list */
    memory->next = first_mob_memory;
    first_mob_memory = memory;

    return memory;
}

/*
 * Add a memory for a mob
 */
void add_mob_memory(int vnum, int type, char *target, char *summary, int emotional_impact)
{
    MOB_MEMORY *memory;
    MEMORY_ENTRY *entry;

    memory = get_mob_memory(vnum);

    CREATE(entry, MEMORY_ENTRY, 1);
    entry->memory_type = type;
    entry->timestamp = time(NULL);
    entry->target_name = str_dup(target);
    entry->summary = str_dup(summary);
    entry->emotional_impact = emotional_impact;
    entry->relationship_change = emotional_impact / 5;  /* -20 to +20 */
    entry->is_promise = FALSE;

    /* Add to list */
    LINK(entry, memory->first_memory, memory->last_memory, next, prev);
    memory->total_memories++;

    /* Save to disk periodically */
    if (memory->total_memories % 10 == 0)
        save_mob_memory(vnum);
}

/*
 * Find a memory about specific target
 */
MEMORY_ENTRY *find_memory_about(int vnum, char *target_name)
{
    MOB_MEMORY *memory;
    MEMORY_ENTRY *entry;

    memory = get_mob_memory(vnum);

    for (entry = memory->first_memory; entry; entry = entry->next)
    {
        if (!str_cmp(entry->target_name, target_name))
            return entry;
    }

    return NULL;
}

/*
 * Save mob memory to file
 */
void save_mob_memory(int vnum)
{
    MOB_MEMORY *memory;
    MEMORY_ENTRY *entry;
    FILE *fp;
    char filename[256];

    memory = get_mob_memory(vnum);
    if (!memory || !memory->first_memory)
        return;

    sprintf(filename, "%s%d.mem", MEMORY_DIR, vnum);

    fp = fopen(filename, "w");
    if (!fp)
    {
        bug("save_mob_memory: cannot open %s", filename);
        return;
    }

    fprintf(fp, "VNUM %d\n", vnum);
    fprintf(fp, "TOTAL %d\n", memory->total_memories);

    for (entry = memory->first_memory; entry; entry = entry->next)
    {
        fprintf(fp, "MEMORY_START\n");
        fprintf(fp, "TYPE %d\n", entry->memory_type);
        fprintf(fp, "TIME %ld\n", entry->timestamp);
        fprintf(fp, "TARGET %s~\n", entry->target_name);
        fprintf(fp, "SUMMARY %s~\n", entry->summary);
        fprintf(fp, "IMPACT %d\n", entry->emotional_impact);
        fprintf(fp, "MEMORY_END\n");
    }

    fclose(fp);
}

/*
 * Load mob memory from file
 */
void load_mob_memory(int vnum)
{
    /* TODO: Implement file loading */
    /* For now, memories are created dynamically */
}

/*
 * Mob writes a book using AI
 */
void mob_write_book(CHAR_DATA *mob, char *topic)
{
    MOB_IDENTITY *identity;
    char prompt[MAX_STRING_LENGTH * 2];
    char *ai_response;
    OBJ_DATA *book;

    if (!IS_NPC(mob))
        return;

    identity = get_mob_identity(mob->pIndexData->vnum);
    if (!identity || !(identity->capabilities & MOB_CAN_WRITE_BOOKS))
    {
        act(AT_PLAIN, "$n doesn't know how to write books.", mob, NULL, NULL, TO_ROOM);
        return;
    }

    /* Generate book content with AI */
    sprintf(prompt,
        "You are %s.\n"
        "Your writing style: %s\n"
        "Your purpose: %s\n\n"
        "Write a short book (200 words) about: %s\n"
        "Write from first person, maintaining your personality.",
        identity->who_am_i,
        identity->writing_style,
        identity->my_purpose,
        topic);

    /* Call AI to generate book */
    /* ai_response = call_ollama(prompt); */
    ai_response = str_dup("[AI-generated book content here]");

    /* Create book object */
    /* book = create_object(...); */
    /* book->value[0] = (content); */

    identity->books_written++;
    track_mob_creation(mob->pIndexData->vnum, 0, topic, TRUE);

    act(AT_YELLOW, "$n finishes writing a book!", mob, NULL, NULL, TO_ROOM);

    sprintf(log_buf, "MOB_BOOK_WRITTEN: %s wrote '%s'", mob->name, topic);
    log_string(log_buf);

    save_mob_identity(identity);
}

/*
 * Mob crafts an item using AI
 */
void mob_craft_item(CHAR_DATA *mob, char *item_type)
{
    MOB_IDENTITY *identity;

    if (!IS_NPC(mob))
        return;

    identity = get_mob_identity(mob->pIndexData->vnum);
    if (!identity || !(identity->capabilities & MOB_CAN_CRAFT_ITEMS))
    {
        act(AT_PLAIN, "$n doesn't know how to craft items.", mob, NULL, NULL, TO_ROOM);
        return;
    }

    /* TODO: Generate item with AI based on craft_specialty */
    /* Create actual object */

    identity->items_crafted++;
    track_mob_creation(mob->pIndexData->vnum, 0, item_type, FALSE);

    act(AT_YELLOW, "$n crafts a new item!", mob, NULL, NULL, TO_ROOM);

    save_mob_identity(identity);
}

/*
 * Track mob creations (books/items)
 */
void track_mob_creation(int creator_vnum, int object_vnum, char *name, bool is_book)
{
    MOB_CREATION *creation;

    CREATE(creation, MOB_CREATION, 1);
    creation->creator_vnum = creator_vnum;
    creation->object_vnum = object_vnum;
    creation->created_when = time(NULL);
    creation->creation_name = str_dup(name);
    creation->is_book = is_book;

    /* Add to list */
    creation->next = first_mob_creation;
    first_mob_creation = creation;
}

/*
 * Generate mob identity using AI
 */
void generate_mob_identity(CHAR_DATA *mob)
{
    MOB_IDENTITY *identity;
    MOB_CONTEXT *ctx;
    char *ai_prompt;
    /* char *ai_response; */
    extern MOB_CONTEXT *analyze_mob_context(CHAR_DATA *mob);
    extern char *generate_context_aware_prompt(MOB_CONTEXT *ctx);
    extern bool validate_personality_fits_context(MOB_CONTEXT *ctx, MOB_IDENTITY *identity);
    extern void free_mob_context(MOB_CONTEXT *ctx);

    if (!IS_NPC(mob))
        return;

    /* STEP 1: Deep context analysis */
    ctx = analyze_mob_context(mob);

    if (!ctx)
    {
        log_string("ERROR: Failed to analyze mob context");
        return;
    }

    /* Log importance level */
    if (ctx->fame_level > 80)
    {
        sprintf(log_buf, "[AI GOD] HIGH IMPORTANCE MOB: %s (fame %d, unique=%s, boss=%s)",
            mob->name, ctx->fame_level,
            ctx->is_unique ? "YES" : "No",
            ctx->is_boss ? "YES" : "No");
        log_string(log_buf);
    }

    /* Get or create identity */
    identity = get_mob_identity(mob->pIndexData->vnum);
    if (!identity)
        identity = create_mob_identity(mob->pIndexData->vnum);

    /* STEP 2: Generate context-aware prompt */
    ai_prompt = generate_context_aware_prompt(ctx);

    if (!ai_prompt)
    {
        log_string("ERROR: Failed to generate context-aware prompt");
        free_mob_context(ctx);
        return;
    }

    sprintf(log_buf, "[AI GOD] Generating context-aware identity for %s (vnum %d, coherence %d/100)",
        mob->name, mob->pIndexData->vnum, ctx->coherence_score);
    log_string(log_buf);

    /* STEP 3: Call AI to generate */
    /* ai_response = call_ollama_advanced(ai_prompt); */

    /* For now, set defaults based on context analysis */

    /* Assign capabilities based on context and mob type */
    if (strstr(mob->name, "scholar") || strstr(mob->name, "scribe") || (ctx->area_type && strstr(ctx->area_type, "academy")))
        identity->capabilities |= MOB_CAN_WRITE_BOOKS;

    if (strstr(mob->name, "smith") || strstr(mob->name, "craftsman"))
        identity->capabilities |= MOB_CAN_CRAFT_ITEMS;

    if (strstr(mob->name, "merchant") || strstr(mob->name, "trader") || ctx->is_shopkeeper)
        identity->capabilities |= MOB_CAN_TRADE;

    /* Set awareness based on importance */
    if (ctx->is_boss || ctx->is_unique)
        identity->awareness_level = 4;  /* Genius-level for bosses/unique */
    else if (ctx->fame_level > 70)
        identity->awareness_level = 3;  /* High for famous */
    else if (ctx->is_shopkeeper || ctx->is_guard)
        identity->awareness_level = 2;  /* Moderate for important roles */
    else
        identity->awareness_level = 1;  /* Basic for common mobs */

    /* Set mobility based on context */
    if (ctx->stays_in_place)
        identity->mobility = 5;   /* Very low for sentinels */
    else if (ctx->is_guard)
        identity->mobility = 20;  /* Low for guards (patrol only) */
    else if (ctx->is_shopkeeper)
        identity->mobility = 10;  /* Very low for shopkeepers */
    else
        identity->mobility = 50;  /* Moderate default */

    /* STEP 4: TODO - Parse AI response and fill identity fields */
    /* identity = parse_ai_response_to_identity(ai_response); */

    /* STEP 5: Validate it fits context */
    if (!validate_personality_fits_context(ctx, identity))
    {
        log_string("[AI GOD] WARNING: Generated personality rejected - doesn't fit context");
        /* Don't save, keep trying later */
        free_mob_context(ctx);
        return;
    }

    /* STEP 6: Save */
    sprintf(log_buf, "[AI GOD] %s awakens to self-awareness (awareness level %d)",
        mob->name, identity->awareness_level);
    log_string(log_buf);

    save_mob_identity(identity);
    free_mob_context(ctx);
}

/*
 * Command: mobidentity - view mob's self-awareness
 */
void do_mobidentity(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    MOB_IDENTITY *identity;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: mobidentity <mob>\n\r", ch);
        return;
    }

    mob = get_char_world(ch, arg);
    if (!mob || !IS_NPC(mob))
    {
        send_to_char("That mob is not here.\n\r", ch);
        return;
    }

    identity = get_mob_identity(mob->pIndexData->vnum);
    if (!identity)
    {
        send_to_char("That mob has no identity data.\n\r", ch);
        return;
    }

    ch_printf(ch, "\n&W=== MOB IDENTITY: %s ===\n\n", mob->name);
    ch_printf(ch, "&GWHO AM I:&w %s\n", identity->who_am_i);
    ch_printf(ch, "&GWHAT I DO:&w %s\n", identity->what_i_do);
    ch_printf(ch, "&GHOW I DO IT:&w %s\n", identity->how_i_do_it);
    ch_printf(ch, "&GWHERE I LIVE:&w %s\n", identity->where_i_live);
    ch_printf(ch, "&GWHERE I GO:&w %s\n", identity->where_i_go);
    ch_printf(ch, "&GMY PURPOSE:&w %s\n\n", identity->my_purpose);

    ch_printf(ch, "&YCapabilities:&w\n");
    if (identity->capabilities & MOB_CAN_WRITE_BOOKS)
        ch_printf(ch, "  - Can write books (style: %s)\n", identity->writing_style);
    if (identity->capabilities & MOB_CAN_CRAFT_ITEMS)
        ch_printf(ch, "  - Can craft items (specialty: %s)\n", identity->craft_specialty);
    if (identity->capabilities & MOB_CAN_TRADE)
        ch_printf(ch, "  - Can trade (skill: %d)\n", identity->trade_skill);

    ch_printf(ch, "\n&CAwareness Level:&w %d\n", identity->awareness_level);
    ch_printf(ch, "&CBooks Written:&w %d\n", identity->books_written);
    ch_printf(ch, "&CItems Crafted:&w %d\n", identity->items_crafted);
}

/*
 * Command: genpersonality - generate personality for mob
 */
void do_genpersonality(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];

    if (ch->level < MAX_LEVEL)
    {
        send_to_char("Only implementors can generate personalities.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: genpersonality <mob>\n\r", ch);
        return;
    }

    mob = get_char_world(ch, arg);
    if (!mob || !IS_NPC(mob))
    {
        send_to_char("That mob is not here.\n\r", ch);
        return;
    }

    ch_printf(ch, "Generating unique personality for %s...\n\r", mob->name);
    generate_mob_identity(mob);
    ch_printf(ch, "Done! Use 'mobidentity %s' to view.\n\r", mob->name);
}

/*
 * Command: mobmemory - view mob's memories
 */
void do_mobmemory(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    MOB_MEMORY *memory;
    MEMORY_ENTRY *entry;
    char arg[MAX_INPUT_LENGTH];
    int count = 0;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: mobmemory <mob>\n\r", ch);
        return;
    }

    mob = get_char_world(ch, arg);
    if (!mob || !IS_NPC(mob))
    {
        send_to_char("That mob is not here.\n\r", ch);
        return;
    }

    memory = get_mob_memory(mob->pIndexData->vnum);
    if (!memory || !memory->first_memory)
    {
        send_to_char("That mob has no memories.\n\r", ch);
        return;
    }

    ch_printf(ch, "\n&W=== MEMORIES OF %s ===\n\n", mob->name);
    ch_printf(ch, "&YTotal memories: %d\n\n", memory->total_memories);

    for (entry = memory->first_memory; entry; entry = entry->next)
    {
        count++;
        if (count > 20)  /* Limit display */
        {
            ch_printf(ch, "... and %d more memories.\n", memory->total_memories - 20);
            break;
        }

        ch_printf(ch, "&G%d.&w %s\n", count, entry->summary);
        ch_printf(ch, "   About: %s | Impact: %+d | %s ago\n\n",
            entry->target_name,
            entry->emotional_impact,
            get_time_diff(entry->timestamp));
    }
}

/*
 * Helper: format time difference
 */
char *get_time_diff(time_t past)
{
    static char buf[128];
    time_t diff = time(NULL) - past;
    int days = diff / 86400;
    int hours = (diff % 86400) / 3600;
    int mins = (diff % 3600) / 60;

    if (days > 0)
        sprintf(buf, "%d day%s", days, days == 1 ? "" : "s");
    else if (hours > 0)
        sprintf(buf, "%d hour%s", hours, hours == 1 ? "" : "s");
    else
        sprintf(buf, "%d minute%s", mins, mins == 1 ? "" : "s");

    return buf;
}

/*
 * Get custom routine for mob based on their schedule
 */
int get_custom_mob_routine(CHAR_DATA *mob)
{
    MOB_IDENTITY *identity;
    CUSTOM_SCHEDULE *sched;
    SCHEDULE_ENTRY *entry;
    int hour;
    extern int get_game_hour(void);  /* From world_simulation.c */

    if (!mob || !IS_NPC(mob))
        return -1;

    identity = get_mob_identity(mob->pIndexData->vnum);
    if (!identity || !identity->has_custom_schedule)
        return -1;

    sched = (CUSTOM_SCHEDULE *)identity->custom_schedule;
    if (!sched)
        return -1;

    hour = get_game_hour();

    /* Find matching schedule entry */
    entry = get_current_schedule(sched, hour);
    if (!entry)
        return -1;

    /* Map activity string to routine constant */
    if (!str_cmp(entry->activity, "SLEEP"))
        return ROUTINE_SLEEP;
    else if (!str_cmp(entry->activity, "WAKE"))
        return ROUTINE_WAKE;
    else if (!str_cmp(entry->activity, "PATROL"))
        return ROUTINE_PATROL;
    else if (!str_cmp(entry->activity, "WORK") || !str_cmp(entry->activity, "RESEARCH"))
        return ROUTINE_WORK;
    else if (!str_cmp(entry->activity, "WRITE"))
        return ROUTINE_WORK;  /* Writing is a form of work */
    else if (!str_cmp(entry->activity, "TRADE"))
        return ROUTINE_TRADE;
    else if (!str_cmp(entry->activity, "SOCIALIZE") || !str_cmp(entry->activity, "SOCIAL"))
        return ROUTINE_SOCIALIZE;
    else if (!str_cmp(entry->activity, "EAT"))
        return ROUTINE_EAT;
    else if (!str_cmp(entry->activity, "GUARD"))
        return ROUTINE_GUARD;
    else if (!str_cmp(entry->activity, "HUNT"))
        return ROUTINE_HUNT;
    else if (!str_cmp(entry->activity, "MEDITATE"))
        return ROUTINE_WORK;  /* Meditation is contemplative work */

    return -1;  /* Unknown activity */
}

/*
 * Get current schedule entry for given hour
 */
SCHEDULE_ENTRY *get_current_schedule(CUSTOM_SCHEDULE *sched, int hour)
{
    SCHEDULE_ENTRY *entry;

    if (!sched)
        return NULL;

    for (entry = sched->first_entry; entry; entry = entry->next)
    {
        /* Check if current hour falls within this entry's time range */
        if (entry->start_hour <= entry->end_hour)
        {
            /* Normal range (e.g., 8:00-17:00) */
            if (hour >= entry->start_hour && hour < entry->end_hour)
                return entry;
        }
        else
        {
            /* Wraps around midnight (e.g., 22:00-6:00) */
            if (hour >= entry->start_hour || hour < entry->end_hour)
                return entry;
        }
    }

    return NULL;
}

/*
 * Create custom schedule
 */
CUSTOM_SCHEDULE *create_custom_schedule(void)
{
    CUSTOM_SCHEDULE *sched;

    CREATE(sched, CUSTOM_SCHEDULE, 1);
    sched->first_entry = NULL;
    sched->last_entry = NULL;

    return sched;
}

/*
 * Add schedule entry
 */
void add_schedule_entry(CUSTOM_SCHEDULE *sched, int start, int end, char *activity, char *location)
{
    SCHEDULE_ENTRY *entry;

    if (!sched)
        return;

    CREATE(entry, SCHEDULE_ENTRY, 1);
    entry->start_hour = start;
    entry->end_hour = end;
    entry->activity = str_dup(activity);
    entry->location = str_dup(location);
    entry->location_vnum = 0;  /* Can be set later */
    entry->next = NULL;

    /* Add to list */
    LINK(entry, sched->first_entry, sched->last_entry, next, prev);
}
