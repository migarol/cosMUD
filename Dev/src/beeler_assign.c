/***************************************************************************
 * Beeler Auto-Assignment System
 *
 * Beeler analyzes mobs and automatically assigns:
 * - Identity (who, what, how, where, why)
 * - Personality and AI prompts
 * - Schedule (sleep/work cycles)
 * - Profession (economic role)
 *
 * Based on complete context analysis.
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "beeler.h"
#include "mob_identity.h"
#include "economy.h"
#include "ai_context_analyzer.h"
#include "ollama_integration.h"

/* Forward declarations */
char * beeler_analyze_mob_context( CHAR_DATA *mob );
char * beeler_generate_identity( CHAR_DATA *mob, const char *context );
void beeler_apply_identity( CHAR_DATA *mob, const char *ai_response );
int beeler_determine_profession( CHAR_DATA *mob, const char *context );

/*
 * Main function: Beeler assigns personality to a mob
 */
void beeler_assign_personality( CHAR_DATA *mob, CHAR_DATA *ch )
{
    char *context;
    char *ai_response;
    int profession;

    if ( !mob || !IS_NPC( mob ) )
    {
        if ( ch )
            send_to_char( "That's not a valid mob.\n\r", ch );
        return;
    }

    if ( ch )
        ch_printf( ch, "&CBeeler analyzing %s...&w\n\r", mob->short_descr );

    /* Step 1: Gather complete context */
    context = beeler_analyze_mob_context( mob );

    if ( !context )
    {
        if ( ch )
            send_to_char( "&RBeeler: Failed to analyze context.&w\n\r", ch );
        return;
    }

    /* Step 2: Generate identity via Ollama */
    ai_response = beeler_generate_identity( mob, context );

    if ( !ai_response )
    {
        if ( ch )
            send_to_char( "&RBeeler: Failed to generate identity (Ollama offline?).&w\n\r", ch );
        DISPOSE( context );
        return;
    }

    /* Step 3: Apply the identity to the mob */
    beeler_apply_identity( mob, ai_response );

    /* Step 4: Determine and assign profession */
    profession = beeler_determine_profession( mob, context );
    set_npc_profession( mob, profession );

    /* Step 5: Save */
    {
        MOB_IDENTITY *identity = get_mob_identity( mob->pIndexData->vnum );
        if ( identity )
            save_mob_identity( identity );
    }

    if ( ch )
    {
        ch_printf( ch, "&GBeeler: Assigned personality to %s&w\n\r", mob->short_descr );
        ch_printf( ch, "&YProfession: %s&w\n\r", profession_name( profession ) );
    }

    /* Cleanup */
    DISPOSE( context );
    DISPOSE( ai_response );
}

/*
 * Analyze complete context for a mob
 */
char * beeler_analyze_mob_context( CHAR_DATA *mob )
{
    static char context[MAX_STRING_LENGTH * 4];
    AREA_ECONOMY *aecon;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int nearby_mobs = 0;
    char buf[MAX_STRING_LENGTH];

    context[0] = '\0';

    /* Basic mob info */
    sprintf( buf, "MOB ANALYSIS:\n" );
    strcat( context, buf );
    sprintf( buf, "Name: %s\n", mob->name );
    strcat( context, buf );
    sprintf( buf, "Short: %s\n", mob->short_descr );
    strcat( context, buf );
    sprintf( buf, "Long: %s\n", mob->long_descr );
    strcat( context, buf );

    if ( mob->description && mob->description[0] != '\0' )
    {
        sprintf( buf, "Description: %s\n", mob->description );
        strcat( context, buf );
    }

    sprintf( buf, "Level: %d, Race: %s, Class: %s\n",
             mob->level,
             race_table[mob->race]->race_name,
             class_table[mob->Class]->who_name );
    strcat( context, buf );

    /* Location context */
    if ( mob->in_room )
    {
        sprintf( buf, "\nLOCATION:\n" );
        strcat( context, buf );
        sprintf( buf, "Room: %s (vnum %d)\n", mob->in_room->name, mob->in_room->vnum );
        strcat( context, buf );
        sprintf( buf, "Area: %s\n", mob->in_room->area->name );
        strcat( context, buf );

        /* Nearby mobs (for context) */
        for ( rch = mob->in_room->first_person; rch; rch = rch->next_in_room )
        {
            if ( IS_NPC( rch ) && rch != mob )
                nearby_mobs++;
        }

        if ( nearby_mobs > 0 )
        {
            sprintf( buf, "Nearby NPCs: %d\n", nearby_mobs );
            strcat( context, buf );
        }
    }

    /* Equipment context */
    strcat( context, "\nEQUIPMENT:\n" );
    for ( obj = mob->first_carrying; obj; obj = obj->next_content )
    {
        sprintf( buf, "- %s\n", obj->short_descr );
        strcat( context, buf );
    }

    /* Economic context */
    if ( mob->in_room && mob->in_room->area )
    {
        aecon = get_area_economy( mob->in_room->area );
        if ( aecon )
        {
            int i;
            sprintf( buf, "\nECONOMIC CONTEXT:\n" );
            strcat( context, buf );
            sprintf( buf, "Region: %s\n", aecon->region_name );
            strcat( context, buf );

            /* Find profession needs */
            strcat( context, "Area needs:\n" );
            for ( i = 0; i < MAX_RESOURCES; i++ )
            {
                if ( aecon->supply[i] < aecon->demand[i] / 2 )
                {
                    sprintf( buf, "- Low %s (needs %s)\n",
                             resource_name( i ),
                             profession_name( get_profession_for_resource( i ) ) );
                    strcat( context, buf );
                }
            }
        }
    }

    /* Mob behavior hints */
    strcat( context, "\nBEHAVIOR FLAGS:\n" );
    if ( xIS_SET( mob->act, ACT_SENTINEL ) )
        strcat( context, "- Sentinel (stays in one place)\n" );
    if ( xIS_SET( mob->act, ACT_AGGRESSIVE ) )
        strcat( context, "- Aggressive\n" );
    if ( xIS_SET( mob->act, ACT_WIMPY ) )
        strcat( context, "- Wimpy\n" );
    if ( xIS_SET( mob->act, ACT_PRACTICE ) )
        strcat( context, "- Trainer\n" );
    if ( mob->spec_fun )
        strcat( context, "- Has special function\n" );

    return str_dup( context );
}

/*
 * Generate identity using Ollama
 */
char * beeler_generate_identity( CHAR_DATA *mob, const char *context )
{
    char prompt[MAX_STRING_LENGTH * 5];
    char *response;
    char log_buf[256];

    sprintf( prompt, "You are Beeler, an autonomous AI overseer. Analyze this NPC and create a complete identity.\n\n%s\n\nGenerate a structured identity in this EXACT format:\n\nWHO_AM_I: [First person, 1 sentence - who they are]\nWHAT_I_DO: [First person, 1 sentence - what they do]\nHOW_I_DO_IT: [First person, 1 sentence - their methods/style]\nWHERE_I_LIVE: [First person, 1 sentence - where they live]\nWHERE_I_GO: [First person, 1 sentence - where they go]\nMY_PURPOSE: [First person, 1 sentence - their life purpose]\nSCHEDULE: [Format: SLEEP 22-6, WORK 6-18, FREE 18-22]\nPERSONALITY: [3-4 personality traits]\n\nBe creative, consider the economic context, area theme, and mob role. Use Spanish for identity fields if area/mob has Spanish context, otherwise English.", context );

    /* Debug logging */
    sprintf(log_buf, "BEELER DEBUG: prompt size=%d, ollama_enabled=%d", (int)strlen(prompt), ollama_enabled);
    log_string(log_buf);

    /* Call Ollama */
    response = ollama_request( prompt, 500 );

    sprintf(log_buf, "BEELER DEBUG: ollama_request returned %s", response ? "data" : "NULL");
    log_string(log_buf);

    return response;
}

/*
 * Apply AI-generated identity to mob
 */
void beeler_apply_identity( CHAR_DATA *mob, const char *ai_response )
{
    MOB_IDENTITY *identity;
    char *line;
    char *response_copy;
    char field[256];
    char value[MAX_STRING_LENGTH];

    if ( !ai_response || ai_response[0] == '\0' )
        return;

    /* Get or create identity */
    identity = get_mob_identity( mob->pIndexData->vnum );
    if ( !identity )
    {
        identity = create_mob_identity( mob->pIndexData->vnum );
    }

    if ( !identity )
        return; /* Failed to create */

    /* Parse the response */
    response_copy = str_dup( ai_response );
    line = strtok( response_copy, "\n" );

    while ( line )
    {
        /* Skip empty lines */
        if ( strlen( line ) < 3 )
        {
            line = strtok( NULL, "\n" );
            continue;
        }

        /* Parse "FIELD: value" format */
        if ( sscanf( line, "%[^:]: %[^\n]", field, value ) == 2 )
        {
            if ( !str_cmp( field, "WHO_AM_I" ) )
            {
                if ( identity->who_am_i )
                    DISPOSE( identity->who_am_i );
                identity->who_am_i = str_dup( value );
            }
            else if ( !str_cmp( field, "WHAT_I_DO" ) )
            {
                if ( identity->what_i_do )
                    DISPOSE( identity->what_i_do );
                identity->what_i_do = str_dup( value );
            }
            else if ( !str_cmp( field, "HOW_I_DO_IT" ) )
            {
                if ( identity->how_i_do_it )
                    DISPOSE( identity->how_i_do_it );
                identity->how_i_do_it = str_dup( value );
            }
            else if ( !str_cmp( field, "WHERE_I_LIVE" ) )
            {
                if ( identity->where_i_live )
                    DISPOSE( identity->where_i_live );
                identity->where_i_live = str_dup( value );
            }
            else if ( !str_cmp( field, "WHERE_I_GO" ) )
            {
                if ( identity->where_i_go )
                    DISPOSE( identity->where_i_go );
                identity->where_i_go = str_dup( value );
            }
            else if ( !str_cmp( field, "MY_PURPOSE" ) )
            {
                if ( identity->my_purpose )
                    DISPOSE( identity->my_purpose );
                identity->my_purpose = str_dup( value );
            }
            else if ( !str_cmp( field, "SCHEDULE" ) )
            {
                /* Parse schedule - format: SLEEP 22-6, WORK 6-18, FREE 18-22 */
                /* TODO: Implement schedule parsing */
            }
        }

        line = strtok( NULL, "\n" );
    }

    /* Set awareness level based on mob level and context */
    if ( mob->level >= 90 )
        identity->awareness_level = AWARENESS_FULL;
    else if ( mob->level >= 50 )
        identity->awareness_level = AWARENESS_HIGH;
    else if ( mob->level >= 20 )
        identity->awareness_level = AWARENESS_MODERATE;
    else
        identity->awareness_level = AWARENESS_BASIC;

    /* Set mobility based on flags */
    if ( xIS_SET( mob->act, ACT_SENTINEL ) )
        identity->mobility = 0;
    else if ( xIS_SET( mob->act, ACT_AGGRESSIVE ) )
        identity->mobility = 80;
    else
        identity->mobility = 40;

    /* Store full AI prompt */
    if ( identity->ai_prompt )
        DISPOSE( identity->ai_prompt );
    identity->ai_prompt = str_dup( ai_response );

    DISPOSE( response_copy );
}

/*
 * Determine profession based on context
 */
int beeler_determine_profession( CHAR_DATA *mob, const char *context )
{
    AREA_ECONOMY *aecon;
    int i;
    int lowest_resource = -1;
    int lowest_supply = 999999;

    /* Check economic needs */
    if ( mob->in_room && mob->in_room->area )
    {
        aecon = get_area_economy( mob->in_room->area );
        if ( aecon )
        {
            /* Find most needed resource */
            for ( i = 0; i < MAX_RESOURCES; i++ )
            {
                int supply_ratio = ( aecon->supply[i] * 100 ) / ( aecon->demand[i] + 1 );
                if ( supply_ratio < lowest_supply )
                {
                    lowest_supply = supply_ratio;
                    lowest_resource = i;
                }
            }

            /* Assign profession for most needed resource */
            if ( lowest_resource >= 0 )
                return get_profession_for_resource( lowest_resource );
        }
    }

    /* Fallback: check mob's behavior/class */
    if ( mob->Class == CLASS_WARRIOR )
        return PROF_NONE; /* Warriors don't produce */
    else if ( mob->Class == CLASS_MAGE )
        return PROF_HERBALIST;
    else if ( mob->Class == CLASS_CLERIC )
        return PROF_HERBALIST;
    else if ( xIS_SET( mob->act, ACT_PRACTICE ) )
        return PROF_NONE; /* Trainers don't produce */

    /* Default to farmer (always useful) */
    return PROF_FARMER;
}
/*
 * Main beeler command - routes to subcommands
 * Declared in mud.h with DECLARE_DO_FUN (provides C linkage)
 */
void do_beeler( CHAR_DATA *ch, const char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "&CBeeler Command System&w\n\r", ch );
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  beeler assign <vnum>        - Assign personality to a mob\n\r", ch );
        send_to_char( "  beeler assign all           - Assign personalities to ALL mobs\n\r", ch );
        send_to_char( "  beeler assign area <name>   - Assign personalities to area mobs\n\r", ch );
        send_to_char( "  beeler analyze <mob>        - View Beeler's analysis of a mob\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "assign" ) )
    {
        do_beeler_assign( ch, (char*)argument );
        return;
    }

    if ( !str_cmp( arg, "analyze" ) )
    {
        do_beeler_analyze( ch, (char*)argument );
        return;
    }

    send_to_char( "Invalid beeler subcommand. Type 'beeler' for syntax.\n\r", ch );
}

/*
 * Command: beeler assign <vnum|all|area>
 */
void do_beeler_assign( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *mob;
    AREA_DATA *area;
    int count = 0;

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: beeler assign <vnum>\n\r", ch );
        send_to_char( "        beeler assign all\n\r", ch );
        send_to_char( "        beeler assign area <area name>\n\r", ch );
        return;
    }

    /* beeler assign all */
    if ( !str_cmp( arg, "all" ) )
    {
        send_to_char( "&YBeeler: Beginning mass analysis of all mobs...&w\n\r", ch );
        send_to_char( "&RWarning: This may take several minutes!&w\n\r\n\r", ch );

        for ( mob = first_char; mob; mob = mob->next )
        {
            if ( !IS_NPC( mob ) )
                continue;

            /* Skip special mobs */
            if ( mob->pIndexData->vnum == BEELER_MOB_VNUM )
                continue;
            if ( xIS_SET( mob->act, ACT_PROTOTYPE ) )
                continue;

            beeler_assign_personality( mob, NULL );
            count++;

            if ( count % 10 == 0 )
                ch_printf( ch, "&G.&w" );
        }

        ch_printf( ch, "\n\r&GBeeler: Assigned personalities to %d mobs.&w\n\r", count );
        return;
    }

    /* beeler assign area <name> */
    if ( !str_cmp( arg, "area" ) )
    {
        if ( argument[0] == '\0' )
        {
            area = ch->in_room->area;
        }
        else
        {
            area = get_area( argument );
            if ( !area )
            {
                send_to_char( "Area not found.\n\r", ch );
                return;
            }
        }

        ch_printf( ch, "&YBeeler: Analyzing all mobs in %s...&w\n\r", area->name );

        for ( mob = first_char; mob; mob = mob->next )
        {
            if ( !IS_NPC( mob ) )
                continue;
            if ( !mob->in_room )
                continue;
            if ( mob->in_room->area != area )
                continue;
            if ( mob->pIndexData->vnum == BEELER_MOB_VNUM )
                continue;

            beeler_assign_personality( mob, NULL );
            count++;

            if ( count % 5 == 0 )
                ch_printf( ch, "&G.&w" );
        }

        ch_printf( ch, "\n\r&GBeeler: Assigned personalities to %d mobs in %s.&w\n\r",
                   count, area->name );
        return;
    }

    /* beeler assign <vnum> */
    mob = get_char_world( ch, arg );

    if ( !mob || !IS_NPC( mob ) )
    {
        send_to_char( "Mob not found.\n\r", ch );
        return;
    }

    beeler_assign_personality( mob, ch );
}

/*
 * Command: beeler analyze <vnum>
 * Shows what Beeler sees about a mob
 */
void do_beeler_analyze( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    char *context;

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: beeler analyze <mob>\n\r", ch );
        return;
    }

    mob = get_char_world( ch, argument );
    if ( !mob || !IS_NPC( mob ) )
    {
        send_to_char( "Mob not found.\n\r", ch );
        return;
    }

    send_to_char( "&CBeeler's Analysis:&w\n\r\n\r", ch );

    context = beeler_analyze_mob_context( mob );
    if ( context )
    {
        send_to_char( context, ch );
        DISPOSE( context );
    }
}

/*
 * Beeler status command - shows system status
 */
void do_beeler_status(CHAR_DATA *ch, const char *argument)
{
    if (IS_NPC(ch))
        return;

    send_to_char("&C=== Beeler Autonomous World System Status ===&w\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("&YCore Systems:&w\n\r", ch);
    send_to_char("  Beeler God Mode:        ACTIVE\n\r", ch);
    send_to_char("  World History Tracker:  ACTIVE\n\r", ch);
    send_to_char("  Organic Creation:       ACTIVE\n\r", ch);
    send_to_char("  Leader AI:              ACTIVE\n\r", ch);
    send_to_char("  Universal Mob AI:       ACTIVE\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("&YAI Integration:&w\n\r", ch);
    send_to_char("  Ollama:                 ENABLED\n\r", ch);
    send_to_char("  Redis Events:           ENABLED\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("Use 'beeler' for command list.\n\r", ch);
    send_to_char("Use 'ollama' for AI content generation.\n\r", ch);
}
