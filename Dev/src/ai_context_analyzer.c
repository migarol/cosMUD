/*****************************************************************************
 * AI God Context Analyzer
 *
 * Deep context analysis to ensure generated personalities fit the world.
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "mob_identity.h"
#include "ai_context_analyzer.h"

/*
 * Main context analysis function
 */
MOB_CONTEXT *analyze_mob_context(CHAR_DATA *mob)
{
    MOB_CONTEXT *ctx;
    AREA_DATA *area;

    if (!mob || !IS_NPC(mob))
        return NULL;

    CREATE(ctx, MOB_CONTEXT, 1);

    /* Basic info */
    ctx->mob_vnum = mob->pIndexData->vnum;
    ctx->mob_name = str_dup(mob->name);
    ctx->short_desc = str_dup(mob->short_descr);
    ctx->long_desc = str_dup(mob->long_descr);

    /* Location analysis */
    if (mob->in_room)
    {
        ctx->room_vnum = mob->in_room->vnum;
        ctx->room_name = str_dup(mob->in_room->name);
        ctx->room_desc = str_dup(mob->in_room->description);

        area = mob->in_room->area;
        if (area)
        {
            ctx->area_vnum = area->low_r_vnum;  /* Use starting vnum as area ID */
            ctx->area_name = str_dup(area->name);
            ctx->area_type = infer_area_type(area);
        }
    }

    /* Analyze mob importance */
    determine_mob_importance(mob, ctx);

    /* Analyze items */
    analyze_mob_items(mob, ctx);

    /* Search for lore mentions */
    search_books_for_mob((char *)mob->name, ctx);

    /* Analyze nearby mobs */
    analyze_nearby_mobs(mob, ctx);

    /* Behavioral flags */
    ctx->is_aggressive = xIS_SET(mob->act, ACT_AGGRESSIVE);
    ctx->is_friendly = !ctx->is_aggressive;
    ctx->stays_in_place = xIS_SET(mob->act, ACT_SENTINEL);
    ctx->wanders = !ctx->stays_in_place;

    /* Calculate coherence score */
    ctx->coherence_score = 0;
    if (ctx->existing_lore && strlen(ctx->existing_lore) > 10)
        ctx->coherence_score += 30;
    if (ctx->mentioned_in_books)
        ctx->coherence_score += 40;
    if (ctx->has_unique_items)
        ctx->coherence_score += 20;
    if (ctx->nearby_mobs_count > 0)
        ctx->coherence_score += 10;

    return ctx;
}

/*
 * Determine mob importance level
 */
void determine_mob_importance(CHAR_DATA *mob, MOB_CONTEXT *ctx)
{
    int hp_ratio, level_ratio;

    /* Check if unique (only one instance) */
    CHAR_DATA *check;
    int count = 0;
    for (check = first_char; check; check = check->next)
    {
        if (IS_NPC(check) && check->pIndexData->vnum == mob->pIndexData->vnum)
            count++;
    }
    ctx->is_unique = (count == 1);

    /* Check if boss (high HP, high level) */
    hp_ratio = mob->max_hit / 100;  /* Rough estimate */
    level_ratio = mob->level;

    if (hp_ratio > 50 && level_ratio > 80)
        ctx->is_boss = TRUE;
    else
        ctx->is_boss = FALSE;

    /* Check special roles */
    ctx->is_shopkeeper = xIS_SET(mob->act, ACT_PRACTICE) ||
                         xIS_SET(mob->act, ACT_TRAIN);
    ctx->is_guard = xIS_SET(mob->act, ACT_SENTINEL);

    /* Fame level calculation */
    ctx->fame_level = 0;
    if (ctx->is_unique) ctx->fame_level += 40;
    if (ctx->is_boss) ctx->fame_level += 30;
    if (level_ratio > 90) ctx->fame_level += 20;
    if (ctx->is_shopkeeper) ctx->fame_level += 10;

    ctx->fame_level = UMIN(100, ctx->fame_level);
}

/*
 * Analyze items the mob carries
 */
void analyze_mob_items(CHAR_DATA *mob, MOB_CONTEXT *ctx)
{
    OBJ_DATA *obj;
    int count = 0;
    int i;

    /* Count items */
    for (obj = mob->first_carrying; obj; obj = obj->next_content)
        count++;

    ctx->num_items = count;

    if (count == 0)
        return;

    /* Allocate array */
    CREATE(ctx->item_names, char *, count);

    /* Store item names */
    i = 0;
    for (obj = mob->first_carrying; obj; obj = obj->next_content)
    {
        ctx->item_names[i++] = str_dup(obj->short_descr);

        /* Check if unique/legendary */
        if (obj->pIndexData->vnum < 100 ||
            strstr(obj->short_descr, "legendary") ||
            strstr(obj->short_descr, "ancient"))
        {
            ctx->has_unique_items = TRUE;
        }
    }
}

/*
 * Search books/scrolls for mentions of this mob
 */
void search_books_for_mob(char *mob_name, MOB_CONTEXT *ctx)
{
    OBJ_DATA *obj;
    int mentions = 0;
    /* char **book_list = NULL; */  /* TODO: Store which books mention this mob */

    /* Search all objects in the world */
    for (obj = first_object; obj; obj = obj->next)
    {
        /* Check if it's a book/scroll */
        if (obj->item_type != ITEM_SCROLL &&
            obj->item_type != ITEM_BOOK &&
            !strstr(obj->short_descr, "book") &&
            !strstr(obj->short_descr, "scroll") &&
            !strstr(obj->short_descr, "tome"))
            continue;

        /* Check if mob name appears in description */
        if (obj->description && strstr(obj->description, mob_name))
        {
            mentions++;
            /* TODO: Store which books mention this mob */
        }

        /* Check extra descriptions */
        EXTRA_DESCR_DATA *ed;
        for (ed = obj->first_extradesc; ed; ed = ed->next)
        {
            if (ed->description && strstr(ed->description, mob_name))
            {
                mentions++;
                break;
            }
        }
    }

    ctx->mentioned_in_books = (mentions > 0);
    ctx->num_mentions = mentions;
}

/*
 * Analyze nearby mobs for context
 */
void analyze_nearby_mobs(CHAR_DATA *mob, MOB_CONTEXT *ctx)
{
    CHAR_DATA *nearby;
    int count = 0;
    int i;

    if (!mob->in_room)
        return;

    /* Count nearby NPCs */
    for (nearby = mob->in_room->first_person; nearby; nearby = nearby->next_in_room)
    {
        if (IS_NPC(nearby) && nearby != mob)
            count++;
    }

    ctx->nearby_mobs_count = count;

    if (count == 0)
        return;

    /* Allocate array */
    CREATE(ctx->nearby_mob_names, char *, count);

    /* Store names */
    i = 0;
    for (nearby = mob->in_room->first_person; nearby; nearby = nearby->next_in_room)
    {
        if (IS_NPC(nearby) && nearby != mob)
            ctx->nearby_mob_names[i++] = str_dup(nearby->name);
    }

    /* Infer group type */
    if (count > 3)
    {
        /* Look for patterns */
        if (strstr(mob->name, "guard"))
            ctx->mob_group_type = str_dup("guard patrol");
        else if (strstr(mob->name, "merchant"))
            ctx->mob_group_type = str_dup("marketplace");
        else if (strstr(mob->name, "scholar") || strstr(mob->name, "scribe"))
            ctx->mob_group_type = str_dup("academics");
        else
            ctx->mob_group_type = str_dup("community");
    }
}

/*
 * Infer area type from name/description
 */
char *infer_area_type(AREA_DATA *area)
{
    char *name;

    if (!area || !area->name)
        return str_dup("unknown");

    name = (char *)area->name;

    if (strstr(name, "City") || strstr(name, "Town") || strstr(name, "Village"))
        return str_dup("city");
    else if (strstr(name, "Dungeon") || strstr(name, "Cave") || strstr(name, "Lair"))
        return str_dup("dungeon");
    else if (strstr(name, "Forest") || strstr(name, "Woods") || strstr(name, "Grove"))
        return str_dup("forest");
    else if (strstr(name, "Castle") || strstr(name, "Keep") || strstr(name, "Fortress"))
        return str_dup("castle");
    else if (strstr(name, "Temple") || strstr(name, "Shrine") || strstr(name, "Cathedral"))
        return str_dup("temple");
    else if (strstr(name, "Academy") || strstr(name, "School") || strstr(name, "Library"))
        return str_dup("academy");
    else
        return str_dup("wilderness");
}

/*
 * Generate AI prompt with full context
 */
char *generate_context_aware_prompt(MOB_CONTEXT *ctx)
{
    static char prompt[MAX_STRING_LENGTH * 8];
    char items_list[MAX_STRING_LENGTH];
    char nearby_list[MAX_STRING_LENGTH];
    int i;

    /* Build items list */
    items_list[0] = '\0';
    if (ctx->num_items > 0)
    {
        strcat(items_list, "Items carried: ");
        for (i = 0; i < ctx->num_items && i < 10; i++)
        {
            strcat(items_list, ctx->item_names[i]);
            if (i < ctx->num_items - 1)
                strcat(items_list, ", ");
        }
    }
    else
    {
        strcat(items_list, "No items");
    }

    /* Build nearby mobs list */
    nearby_list[0] = '\0';
    if (ctx->nearby_mobs_count > 0)
    {
        sprintf(nearby_list, "Nearby: %d other mobs", ctx->nearby_mobs_count);
    }

    /* Generate comprehensive prompt */
    sprintf(prompt,
        "CRITICAL: You are generating a personality for a mob in an EXISTING world.\n"
        "You MUST respect all existing lore and context. DO NOT contradict it.\n\n"

        "=== MOB INFORMATION ===\n"
        "Name: %s\n"
        "Short: %s\n"
        "Long: %s\n"
        "Vnum: %d\n\n"

        "=== LOCATION CONTEXT ===\n"
        "Area: %s (type: %s)\n"
        "Room: %s\n"
        "Room Description: %s\n\n"

        "=== IMPORTANCE ===\n"
        "Is Unique: %s\n"
        "Is Boss: %s\n"
        "Is Shopkeeper: %s\n"
        "Is Guard: %s\n"
        "Fame Level: %d/100\n\n"

        "=== ITEMS ===\n"
        "%s\n"
        "Has Legendary Items: %s\n\n"

        "=== LORE ===\n"
        "Mentioned in Books: %s (%d mentions)\n"
        "Existing Lore: %s\n\n"

        "=== CONTEXT ===\n"
        "%s\n"
        "Group Type: %s\n\n"

        "=== BEHAVIOR FLAGS ===\n"
        "Aggressive: %s\n"
        "Stays in Place: %s\n\n"

        "=== YOUR TASK ===\n"
        "Generate a UNIQUE personality that:\n"
        "1. RESPECTS all existing lore and context\n"
        "2. FITS the location and role\n"
        "3. EXPLAINS why they have these items\n"
        "4. MATCHES their importance level\n"
        "5. INTEGRATES with nearby mobs\n"
        "6. FEELS AUTHENTIC to the world\n\n"

        "Provide:\n"
        "WHO_AM_I: (20 words)\n"
        "WHAT_I_DO: (15 words)\n"
        "HOW_I_DO_IT: (15 words)\n"
        "WHERE_I_LIVE: (10 words)\n"
        "WHERE_I_GO: (15 words)\n"
        "PURPOSE: (15 words)\n"
        "WRITING_STYLE: if applicable (10 words)\n"
        "MOBILITY: 0-100\n"
        "CAPABILITIES: CAN_WRITE_BOOKS, CAN_CRAFT_ITEMS, CAN_TRADE, etc\n"
        "AWARENESS: 0-4\n"
        "WHY_THESE_ITEMS: Explain items (30 words)\n\n"

        "IMPORTANT: If this is a boss/unique/famous mob, reflect that in personality!\n"
        "IMPORTANT: If mentioned in books, acknowledge that existing fame!\n"
        "IMPORTANT: Match the area type (city scholar vs dungeon beast)!\n",

        ctx->mob_name,
        ctx->short_desc,
        ctx->long_desc,
        ctx->mob_vnum,

        ctx->area_name ? ctx->area_name : "Unknown",
        ctx->area_type ? ctx->area_type : "unknown",
        ctx->room_name ? ctx->room_name : "Unknown",
        ctx->room_desc ? ctx->room_desc : "No description",

        ctx->is_unique ? "YES - Only one in world!" : "No",
        ctx->is_boss ? "YES - Boss mob!" : "No",
        ctx->is_shopkeeper ? "YES" : "No",
        ctx->is_guard ? "YES" : "No",
        ctx->fame_level,

        items_list,
        ctx->has_unique_items ? "YES" : "No",

        ctx->mentioned_in_books ? "YES" : "No",
        ctx->num_mentions,
        ctx->existing_lore ? ctx->existing_lore : "None found",

        nearby_list,
        ctx->mob_group_type ? ctx->mob_group_type : "Solo",

        ctx->is_aggressive ? "YES" : "No",
        ctx->stays_in_place ? "YES" : "No"
    );

    return prompt;
}

/*
 * Validate that generated personality fits context
 */
bool validate_personality_fits_context(MOB_CONTEXT *ctx, MOB_IDENTITY *identity)
{
    /* Check boss mobs aren't made weak */
    if (ctx->is_boss && identity->awareness_level < 2)
    {
        log_string("WARNING: Boss mob given low awareness - rejecting");
        return FALSE;
    }

    /* Check guards aren't made mobile */
    if (ctx->is_guard && !identity->has_custom_schedule)
    {
        /* Guards should have guard schedule */
    }

    /* Check unique mobs have high awareness */
    if (ctx->is_unique && ctx->fame_level > 70 && identity->awareness_level < 3)
    {
        log_string("WARNING: Famous unique mob given low awareness - rejecting");
        return FALSE;
    }

    return TRUE;
}

/*
 * Free context structure
 */
void free_mob_context(MOB_CONTEXT *ctx)
{
    int i;

    if (!ctx)
        return;

    STRFREE(ctx->mob_name);
    STRFREE(ctx->short_desc);
    STRFREE(ctx->long_desc);
    STRFREE(ctx->area_name);
    STRFREE(ctx->area_type);
    STRFREE(ctx->room_name);
    STRFREE(ctx->room_desc);

    if (ctx->item_names)
    {
        for (i = 0; i < ctx->num_items; i++)
            STRFREE(ctx->item_names[i]);
        DISPOSE(ctx->item_names);
    }

    if (ctx->nearby_mob_names)
    {
        for (i = 0; i < ctx->nearby_mobs_count; i++)
            STRFREE(ctx->nearby_mob_names[i]);
        DISPOSE(ctx->nearby_mob_names);
    }

    DISPOSE(ctx);
}
