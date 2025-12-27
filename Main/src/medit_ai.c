/*****************************************************************************
 * MEDIT AI Extensions
 *
 * Extends medit to show and edit:
 * - Mob identity (personality, awareness, capabilities)
 * - Mob homes (home_vnum, home_type, home_name)
 * - Mob memories
 * - Custom schedules
 *
 * Usage in medit:
 *   medit <mob>
 *   > ai              - Show all AI data
 *   > aiidentity      - Edit identity fields
 *   > aihome          - Edit home settings
 *   >aimemory        - View/edit memories
 *   > aischedule      - Edit custom schedule
 *   > aigenerate      - Trigger AI God to generate personality
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "mob_identity.h"
#include "mob_home.h"
#include "ai_context_analyzer.h"

/*
 * Show all AI data for mob
 */
void medit_show_ai(CHAR_DATA *ch, CHAR_DATA *mob)
{
    MOB_IDENTITY *identity;
    MOB_HOME *home;
    MOB_MEMORY *memory;
    extern MOB_IDENTITY *get_mob_identity(int vnum);
    extern MOB_HOME *get_mob_home(int vnum);
    extern MOB_MEMORY *get_mob_memory(int vnum);

    if (!IS_NPC(mob))
    {
        send_to_char("Not a mob.\n\r", ch);
        return;
    }

    send_to_char("\n\r&Y=== AI Data for Mob ===&w\n\r\n\r", ch);

    /* Identity */
    identity = get_mob_identity(mob->pIndexData->vnum);
    if (identity)
    {
        send_to_char("&G[IDENTITY]&w\n\r", ch);

        if (identity->who_am_i)
        {
            sprintf(log_buf, "  Who Am I: %s\n\r", identity->who_am_i);
            send_to_char(log_buf, ch);
        }

        if (identity->what_i_do)
        {
            sprintf(log_buf, "  What I Do: %s\n\r", identity->what_i_do);
            send_to_char(log_buf, ch);
        }

        if (identity->where_i_live)
        {
            sprintf(log_buf, "  Where I Live: %s\n\r", identity->where_i_live);
            send_to_char(log_buf, ch);
        }

        sprintf(log_buf, "  Awareness Level: %d/4\n\r", identity->awareness_level);
        send_to_char(log_buf, ch);

        sprintf(log_buf, "  Mobility: %d/100\n\r", identity->mobility);
        send_to_char(log_buf, ch);

        /* Capabilities */
        send_to_char("  Capabilities: ", ch);
        if (identity->capabilities & MOB_CAN_WRITE_BOOKS)
            send_to_char("WRITE_BOOKS ", ch);
        if (identity->capabilities & MOB_CAN_CRAFT_ITEMS)
            send_to_char("CRAFT_ITEMS ", ch);
        if (identity->capabilities & MOB_CAN_TRADE)
            send_to_char("TRADE ", ch);
        if (identity->capabilities & MOB_CAN_TEACH)
            send_to_char("TEACH ", ch);
        send_to_char("\n\r", ch);

        if (identity->books_written > 0)
        {
            sprintf(log_buf, "  Books Written: %d\n\r", identity->books_written);
            send_to_char(log_buf, ch);
        }

        send_to_char("\n\r", ch);
    }
    else
    {
        send_to_char("&R[NO IDENTITY]&w - Use 'aigenerate' to create\n\r\n\r", ch);
    }

    /* Home */
    home = get_mob_home(mob->pIndexData->vnum);
    if (home)
    {
        send_to_char("&G[HOME]&w\n\r", ch);

        if (home->home_name)
        {
            sprintf(log_buf, "  Name: %s\n\r", home->home_name);
            send_to_char(log_buf, ch);
        }

        sprintf(log_buf, "  Vnum: %d\n\r", home->home_vnum);
        send_to_char(log_buf, ch);

        extern char *home_type_name(int home_type);
        sprintf(log_buf, "  Type: %s\n\r", home_type_name(home->home_type));
        send_to_char(log_buf, ch);

        sprintf(log_buf, "  Owned: %s\n\r", home->owned ? "Yes" : "No (rented)");
        send_to_char(log_buf, ch);

        if (!home->owned)
        {
            sprintf(log_buf, "  Rent: %d gold/week\n\r", home->rent_cost);
            send_to_char(log_buf, ch);
        }

        if (home->times_visited > 0)
        {
            sprintf(log_buf, "  Times Visited: %d\n\r", home->times_visited);
            send_to_char(log_buf, ch);
        }

        send_to_char("\n\r", ch);
    }
    else
    {
        send_to_char("&R[NO HOME]&w - Use 'beeler_build home <vnum>' to create\n\r\n\r", ch);
    }

    /* Memory */
    memory = get_mob_memory(mob->pIndexData->vnum);
    if (memory && memory->total_memories > 0)
    {
        send_to_char("&G[MEMORY]&w\n\r", ch);
        sprintf(log_buf, "  Total Memories: %d\n\r", memory->total_memories);
        send_to_char(log_buf, ch);
        send_to_char("  Use 'aimemory' to view details\n\r\n\r", ch);
    }

    /* Custom Schedule */
    if (identity && identity->has_custom_schedule)
    {
        send_to_char("&G[CUSTOM SCHEDULE]&w\n\r", ch);
        send_to_char("  This mob has a custom schedule\n\r", ch);
        send_to_char("  Use 'aischedule' to view/edit\n\r\n\r", ch);
    }

    send_to_char("&CCommands:&w\n\r", ch);
    send_to_char("  aiidentity <field> <value> - Edit identity\n\r", ch);
    send_to_char("  aihome <field> <value>     - Edit home\n\r", ch);
    send_to_char("  aimemory                   - View memories\n\r", ch);
    send_to_char("  aischedule                 - View/edit schedule\n\r", ch);
    send_to_char("  aigenerate                 - Generate AI personality\n\r", ch);
    send_to_char("  aianalyze                  - Show context analysis\n\r", ch);
}

/*
 * Edit mob identity
 */
void medit_identity(CHAR_DATA *ch, CHAR_DATA *mob, char *argument)
{
    MOB_IDENTITY *identity;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    extern MOB_IDENTITY *get_mob_identity(int vnum);
    extern MOB_IDENTITY *create_mob_identity(int vnum);
    extern void save_mob_identity(MOB_IDENTITY *identity);

    if (!IS_NPC(mob))
        return;

    argument = one_argument(argument, arg1);
    strcpy(arg2, argument);

    identity = get_mob_identity(mob->pIndexData->vnum);
    if (!identity)
    {
        send_to_char("Creating new identity...\n\r", ch);
        identity = create_mob_identity(mob->pIndexData->vnum);
    }

    if (arg1[0] == '\0')
    {
        send_to_char("Identity fields:\n\r", ch);
        send_to_char("  who_am_i       - Brief self-introduction\n\r", ch);
        send_to_char("  what_i_do      - Main activity/profession\n\r", ch);
        send_to_char("  how_i_do_it    - Work style/method\n\r", ch);
        send_to_char("  where_i_live   - Home location\n\r", ch);
        send_to_char("  where_i_go     - Travel patterns\n\r", ch);
        send_to_char("  my_purpose     - Life purpose\n\r", ch);
        send_to_char("  awareness      - Awareness level (0-4)\n\r", ch);
        send_to_char("  mobility       - Movement tendency (0-100)\n\r", ch);
        send_to_char("  capability     - Add capability (write_books, craft_items, trade, teach)\n\r", ch);
        return;
    }

    /* Edit fields */
    if (!str_cmp(arg1, "who_am_i"))
    {
        STRFREE(identity->who_am_i);
        identity->who_am_i = str_dup(arg2);
        send_to_char("Who Am I set.\n\r", ch);
    }
    else if (!str_cmp(arg1, "what_i_do"))
    {
        STRFREE(identity->what_i_do);
        identity->what_i_do = str_dup(arg2);
        send_to_char("What I Do set.\n\r", ch);
    }
    else if (!str_cmp(arg1, "how_i_do_it"))
    {
        STRFREE(identity->how_i_do_it);
        identity->how_i_do_it = str_dup(arg2);
        send_to_char("How I Do It set.\n\r", ch);
    }
    else if (!str_cmp(arg1, "where_i_live"))
    {
        STRFREE(identity->where_i_live);
        identity->where_i_live = str_dup(arg2);
        send_to_char("Where I Live set.\n\r", ch);
    }
    else if (!str_cmp(arg1, "where_i_go"))
    {
        STRFREE(identity->where_i_go);
        identity->where_i_go = str_dup(arg2);
        send_to_char("Where I Go set.\n\r", ch);
    }
    else if (!str_cmp(arg1, "my_purpose"))
    {
        STRFREE(identity->my_purpose);
        identity->my_purpose = str_dup(arg2);
        send_to_char("My Purpose set.\n\r", ch);
    }
    else if (!str_cmp(arg1, "awareness"))
    {
        int level = atoi(arg2);
        if (level < 0 || level > 4)
        {
            send_to_char("Awareness must be 0-4.\n\r", ch);
            return;
        }
        identity->awareness_level = level;
        send_to_char("Awareness level set.\n\r", ch);
    }
    else if (!str_cmp(arg1, "mobility"))
    {
        int mob_level = atoi(arg2);
        if (mob_level < 0 || mob_level > 100)
        {
            send_to_char("Mobility must be 0-100.\n\r", ch);
            return;
        }
        identity->mobility = mob_level;
        send_to_char("Mobility set.\n\r", ch);
    }
    else if (!str_cmp(arg1, "capability"))
    {
        if (!str_cmp(arg2, "write_books"))
        {
            identity->capabilities |= MOB_CAN_WRITE_BOOKS;
            send_to_char("Added CAN_WRITE_BOOKS capability.\n\r", ch);
        }
        else if (!str_cmp(arg2, "craft_items"))
        {
            identity->capabilities |= MOB_CAN_CRAFT_ITEMS;
            send_to_char("Added CAN_CRAFT_ITEMS capability.\n\r", ch);
        }
        else if (!str_cmp(arg2, "trade"))
        {
            identity->capabilities |= MOB_CAN_TRADE;
            send_to_char("Added CAN_TRADE capability.\n\r", ch);
        }
        else if (!str_cmp(arg2, "teach"))
        {
            identity->capabilities |= MOB_CAN_TEACH;
            send_to_char("Added CAN_TEACH capability.\n\r", ch);
        }
        else
        {
            send_to_char("Unknown capability. Options: write_books, craft_items, trade, teach\n\r", ch);
            return;
        }
    }
    else
    {
        send_to_char("Unknown field.\n\r", ch);
        return;
    }

    /* Save */
    save_mob_identity(identity);
}

/*
 * Generate AI personality on demand
 */
void medit_ai_generate(CHAR_DATA *ch, CHAR_DATA *mob)
{
    extern void generate_mob_identity(CHAR_DATA *mob);

    if (!IS_NPC(mob))
        return;

    send_to_char("&Y[AI God] Analyzing context and generating personality...&w\n\r\n\r", ch);

    generate_mob_identity(mob);

    send_to_char("&GPersonality generated!&w Use 'ai' to view.\n\r", ch);
}

/*
 * Show context analysis
 */
void medit_ai_analyze(CHAR_DATA *ch, CHAR_DATA *mob)
{
    MOB_CONTEXT *ctx;
    extern MOB_CONTEXT *analyze_mob_context(CHAR_DATA *mob);
    extern void free_mob_context(MOB_CONTEXT *ctx);

    if (!IS_NPC(mob))
        return;

    send_to_char("&Y[Beeler] Performing deep context analysis...&w\n\r\n\r", ch);

    ctx = analyze_mob_context(mob);

    if (!ctx)
    {
        send_to_char("&RAnalysis failed.&w\n\r", ch);
        return;
    }

    send_to_char("&C=== Context Analysis ===&w\n\r\n\r", ch);

    /* Location */
    if (ctx->area_name)
    {
        sprintf(log_buf, "Area: %s (type: %s)\n\r",
            ctx->area_name,
            ctx->area_type ? ctx->area_type : "unknown");
        send_to_char(log_buf, ch);
    }

    if (ctx->room_name)
    {
        sprintf(log_buf, "Room: %s\n\r", ctx->room_name);
        send_to_char(log_buf, ch);
    }

    /* Importance */
    sprintf(log_buf, "\nFame Level: %d/100\n\r", ctx->fame_level);
    send_to_char(log_buf, ch);

    sprintf(log_buf, "Unique: %s\n\r", ctx->is_unique ? "&GYES&w" : "No");
    send_to_char(log_buf, ch);

    sprintf(log_buf, "Boss: %s\n\r", ctx->is_boss ? "&RYES&w" : "No");
    send_to_char(log_buf, ch);

    sprintf(log_buf, "Shopkeeper: %s\n\r", ctx->is_shopkeeper ? "YES" : "No");
    send_to_char(log_buf, ch);

    sprintf(log_buf, "Guard: %s\n\r", ctx->is_guard ? "YES" : "No");
    send_to_char(log_buf, ch);

    /* Items */
    if (ctx->num_items > 0)
    {
        sprintf(log_buf, "\nCarrying %d items\n\r", ctx->num_items);
        send_to_char(log_buf, ch);

        if (ctx->has_unique_items)
            send_to_char("  &YHAS UNIQUE/LEGENDARY ITEMS&w\n\r", ch);
    }

    /* Lore */
    if (ctx->mentioned_in_books)
    {
        sprintf(log_buf, "\n&GMentioned in %d books/scrolls&w\n\r", ctx->num_mentions);
        send_to_char(log_buf, ch);
    }

    /* Coherence */
    sprintf(log_buf, "\n&CCoherence Score: %d/100&w\n\r", ctx->coherence_score);
    send_to_char(log_buf, ch);

    if (ctx->warnings && ctx->warnings[0] != '\0')
    {
        sprintf(log_buf, "\n&RWARNINGS:&w\n%s\n\r", ctx->warnings);
        send_to_char(log_buf, ch);
    }

    free_mob_context(ctx);
}

/*
 * View/edit mob memories
 */
void medit_memory(CHAR_DATA *ch, CHAR_DATA *mob, char *argument)
{
    MOB_MEMORY *memory;
    MEMORY_ENTRY *entry;
    int count = 0;
    extern MOB_MEMORY *get_mob_memory(int vnum);

    if (!IS_NPC(mob))
        return;

    memory = get_mob_memory(mob->pIndexData->vnum);

    if (!memory || memory->total_memories == 0)
    {
        send_to_char("This mob has no memories yet.\n\r", ch);
        return;
    }

    send_to_char("\n\r&Y=== Mob Memories ===&w\n\r\n\r", ch);

    for (entry = memory->first_memory; entry; entry = entry->next)
    {
        count++;

        sprintf(log_buf, "&C[%d]&w %s about &G%s&w\n\r",
            count,
            ctime(&entry->timestamp),
            entry->target_name);
        send_to_char(log_buf, ch);

        if (entry->summary)
        {
            sprintf(log_buf, "    %s\n\r", entry->summary);
            send_to_char(log_buf, ch);
        }

        sprintf(log_buf, "    Emotional Impact: %d  Relationship: %s%d&w\n\r",
            entry->emotional_impact,
            entry->relationship_change > 0 ? "&G+" : "&R",
            entry->relationship_change);
        send_to_char(log_buf, ch);

        send_to_char("\n\r", ch);

        /* Limit output */
        if (count >= 10)
        {
            sprintf(log_buf, "... and %d more memories\n\r",
                memory->total_memories - count);
            send_to_char(log_buf, ch);
            break;
        }
    }
}
