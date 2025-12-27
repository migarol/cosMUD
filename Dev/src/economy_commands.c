/***************************************************************************
 * Economy System Commands for cosMUD
 * Immortal commands for managing and viewing the economic system
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "economy.h"

/*
 * View economy status
 * Syntax: ecoview [area|region] [name]
 */
void do_ecoview( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AREA_ECONOMY *aecon;
    REGIONAL_ECONOMY *recon;
    AREA_DATA *area;
    int i;

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        /* Show current area */
        area = ch->in_room->area;
        aecon = get_area_economy( area );

        if ( !aecon )
        {
            send_to_char( "This area has no economy data.\n\r", ch );
            return;
        }

        ch_printf( ch, "\n\r&Y-=[ Economy: %s ]=-&w\n\r", area->name );
        ch_printf( ch, "&WRegion:&w %s\n\r\n\r", aecon->region_name );

        /* Resources */
        send_to_char( "&G--- Resources ---&w\n\r", ch );
        send_to_char( "Resource      Supply  Demand  Prod/Day  Cons/Day  Price\n\r", ch );
        send_to_char( "--------------------------------------------------------\n\r", ch );

        for ( i = 0; i < MAX_RESOURCE; i++ )
        {
            ch_printf( ch, "%-12s  %6d  %6d  %8d  %8d  %5d\n\r",
                       resource_name( i ),
                       aecon->supply[i],
                       aecon->demand[i],
                       aecon->production[i],
                       aecon->consumption[i],
                       aecon->current_price[i] );
        }

        /* Professions */
        send_to_char( "\n\r&G--- Professions ---&w\n\r", ch );
        send_to_char( "Profession      Workers  Productivity\n\r", ch );
        send_to_char( "--------------------------------------\n\r", ch );

        for ( i = 1; i < MAX_PROFESSION; i++ )
        {
            if ( aecon->workers[i] > 0 )
            {
                ch_printf( ch, "%-14s  %7d  %12d%%\n\r",
                           profession_name( i ),
                           aecon->workers[i],
                           aecon->productivity[i] );
            }
        }

        /* Events */
        if ( aecon->active_event != EVENT_NONE )
        {
            ch_printf( ch, "\n\r&R--- Active Event ---&w\n\r" );
            ch_printf( ch, "Event: %s\n\r", event_name( aecon->active_event ) );
            ch_printf( ch, "Duration: %d days remaining\n\r", aecon->event_duration );
        }

        /* Stats */
        ch_printf( ch, "\n\r&C--- Statistics ---&w\n\r" );
        ch_printf( ch, "Auto-balance interventions: %d\n\r", aecon->interventions );
        ch_printf( ch, "Last update: %s", ctime( &aecon->last_update ) );

        return;
    }

    if ( !str_cmp( arg1, "region" ) )
    {
        if ( arg2[0] == '\0' )
        {
            /* List all regions */
            send_to_char( "\n\r&Y-=[ Regions ]=-&w\n\r\n\r", ch );

            for ( recon = first_regional_economy; recon; recon = recon->next )
            {
                ch_printf( ch, "&W%s&w\n\r", recon->name );
                ch_printf( ch, "  Areas: %d  Workers: %d  Population: %d\n\r",
                           recon->num_areas, recon->total_workers, recon->total_population );
            }
            return;
        }

        /* Show specific region */
        recon = get_regional_economy( arg2 );
        if ( !recon )
        {
            send_to_char( "Region not found.\n\r", ch );
            return;
        }

        ch_printf( ch, "\n\r&Y-=[ Regional Economy: %s ]=-&w\n\r\n\r", recon->name );

        send_to_char( "&G--- Resources ---&w\n\r", ch );
        send_to_char( "Resource      Supply  Demand  Avg Price\n\r", ch );
        send_to_char( "----------------------------------------\n\r", ch );

        for ( i = 0; i < MAX_RESOURCE; i++ )
        {
            ch_printf( ch, "%-12s  %6d  %6d  %9d\n\r",
                       resource_name( i ),
                       recon->total_supply[i],
                       recon->total_demand[i],
                       recon->avg_price[i] );
        }

        ch_printf( ch, "\n\r&C--- Regional Stats ---&w\n\r" );
        ch_printf( ch, "Total Areas: %d\n\r", recon->num_areas );
        ch_printf( ch, "Total Workers: %d\n\r", recon->total_workers );
        ch_printf( ch, "Total Population: %d\n\r", recon->total_population );

        return;
    }

    if ( !str_cmp( arg1, "area" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_char( "View which area?\n\r", ch );
            return;
        }

        area = get_area( arg2 );
        if ( !area )
        {
            send_to_char( "Area not found.\n\r", ch );
            return;
        }

        aecon = get_area_economy( area );
        if ( !aecon )
        {
            send_to_char( "That area has no economy.\n\r", ch );
            return;
        }

        /* Recursively call with area context */
        ch->in_room->area = area;
        do_ecoview( ch, "" );
        return;
    }

    send_to_char( "Syntax: ecoview [area|region] [name]\n\r", ch );
}

/*
 * Set economy values
 * Syntax: ecoset <resource|worker|productivity> <type> <value>
 */
void do_ecoset( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    AREA_ECONOMY *aecon;
    int type, value;
    int i;

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "Syntax: ecoset <supply|demand|production|consumption|workers|productivity> <type> <value>\n\r", ch );
        send_to_char( "\n\rResource types: meat, grain, vegetables, water, hide, wood, stone, ore, cloth, herbs\n\r", ch );
        send_to_char( "Profession types: farmer, rancher, hunter, miner, lumberjack, water_keeper, etc.\n\r", ch );
        return;
    }

    aecon = get_area_economy( ch->in_room->area );
    if ( !aecon )
    {
        send_to_char( "This area has no economy.\n\r", ch );
        return;
    }

    value = atoi( arg3 );

    /* Handle resource adjustments */
    if ( !str_prefix( arg1, "supply" ) || !str_prefix( arg1, "demand" ) ||
         !str_prefix( arg1, "production" ) || !str_prefix( arg1, "consumption" ) )
    {
        /* Find resource type */
        type = -1;
        for ( i = 0; i < MAX_RESOURCE; i++ )
        {
            if ( !str_prefix( arg2, resource_name( i ) ) )
            {
                type = i;
                break;
            }
        }

        if ( type == -1 )
        {
            send_to_char( "Invalid resource type.\n\r", ch );
            return;
        }

        if ( !str_prefix( arg1, "supply" ) )
        {
            aecon->supply[type] = value;
            ch_printf( ch, "Set %s supply to %d.\n\r", resource_name( type ), value );
        }
        else if ( !str_prefix( arg1, "demand" ) )
        {
            aecon->demand[type] = value;
            ch_printf( ch, "Set %s demand to %d.\n\r", resource_name( type ), value );
        }
        else if ( !str_prefix( arg1, "production" ) )
        {
            aecon->production[type] = value;
            ch_printf( ch, "Set %s production to %d.\n\r", resource_name( type ), value );
        }
        else if ( !str_prefix( arg1, "consumption" ) )
        {
            aecon->consumption[type] = value;
            ch_printf( ch, "Set %s consumption to %d.\n\r", resource_name( type ), value );
        }

        /* Update prices */
        update_prices( aecon );
        return;
    }

    /* Handle profession adjustments */
    if ( !str_prefix( arg1, "workers" ) || !str_prefix( arg1, "productivity" ) )
    {
        /* Find profession type */
        type = -1;
        for ( i = 0; i < MAX_PROFESSION; i++ )
        {
            if ( !str_prefix( arg2, profession_name( i ) ) )
            {
                type = i;
                break;
            }
        }

        if ( type == -1 )
        {
            send_to_char( "Invalid profession type.\n\r", ch );
            return;
        }

        if ( !str_prefix( arg1, "workers" ) )
        {
            aecon->workers[type] = value;
            ch_printf( ch, "Set %s workers to %d.\n\r", profession_name( type ), value );
        }
        else if ( !str_prefix( arg1, "productivity" ) )
        {
            aecon->productivity[type] = value;
            ch_printf( ch, "Set %s productivity to %d%%.\n\r", profession_name( type ), value );
        }

        return;
    }

    send_to_char( "Invalid parameter. Use: supply, demand, production, consumption, workers, or productivity.\n\r", ch );
}

/*
 * Trigger economic event
 * Syntax: ecoevent <plague|drought|bumper|collapse|wolf|discovery>
 */
void do_ecoevent( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AREA_ECONOMY *aecon;
    int event_type = EVENT_NONE;

    if ( IS_NPC( ch ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: ecoevent <plague|drought|bumper|collapse|wolf|discovery|clear>\n\r", ch );
        send_to_char( "\n\rEvents:\n\r", ch );
        send_to_char( "  plague     - Kills 30%% livestock\n\r", ch );
        send_to_char( "  drought    - -50%% water, -30%% crops\n\r", ch );
        send_to_char( "  bumper     - +50%% grain/vegetables (bumper crop)\n\r", ch );
        send_to_char( "  collapse   - -80%% ore production (mine collapse)\n\r", ch );
        send_to_char( "  wolf       - -20%% livestock (wolf attack)\n\r", ch );
        send_to_char( "  discovery  - +30%% ore/stone\n\r", ch );
        send_to_char( "  clear      - Clear current event\n\r", ch );
        return;
    }

    aecon = get_area_economy( ch->in_room->area );
    if ( !aecon )
    {
        send_to_char( "This area has no economy.\n\r", ch );
        return;
    }

    if ( !str_prefix( arg, "clear" ) )
    {
        aecon->active_event = EVENT_NONE;
        aecon->event_duration = 0;
        send_to_char( "Event cleared.\n\r", ch );
        return;
    }

    /* Parse event type */
    if ( !str_prefix( arg, "plague" ) )
        event_type = EVENT_PLAGUE;
    else if ( !str_prefix( arg, "drought" ) )
        event_type = EVENT_DROUGHT;
    else if ( !str_prefix( arg, "bumper" ) )
        event_type = EVENT_BUMPER_CROP;
    else if ( !str_prefix( arg, "collapse" ) )
        event_type = EVENT_MINE_COLLAPSE;
    else if ( !str_prefix( arg, "wolf" ) )
        event_type = EVENT_WOLF_ATTACK;
    else if ( !str_prefix( arg, "discovery" ) )
        event_type = EVENT_DISCOVERY;
    else
    {
        send_to_char( "Invalid event type.\n\r", ch );
        return;
    }

    /* Trigger event */
    trigger_specific_event( aecon, event_type );
    ch_printf( ch, "Triggered %s event in %s.\n\r",
               event_name( event_type ), aecon->area->name );
}

/*
 * View auto-balance information
 * Syntax: ecobalance
 */
void do_ecobalance( CHAR_DATA *ch, char *argument )
{
    AREA_ECONOMY *aecon;
    int i;
    int critical_count = 0;

    if ( IS_NPC( ch ) )
        return;

    send_to_char( "\n\r&Y-=[ Auto-Balance Status ]=-&w\n\r\n\r", ch );

    /* Check all areas */
    for ( aecon = first_area_economy; aecon; aecon = aecon->next )
    {
        int area_critical = 0;

        /* Check for critical resources */
        for ( i = 0; i < MAX_RESOURCE; i++ )
        {
            if ( aecon->supply[i] < aecon->demand[i] / 10 )
            {
                if ( area_critical == 0 )
                {
                    ch_printf( ch, "&R%s:&w\n\r", aecon->area->name );
                }
                ch_printf( ch, "  %s: %d/%d (%.1f%% of demand)\n\r",
                           resource_name( i ),
                           aecon->supply[i],
                           aecon->demand[i],
                           ( (float)aecon->supply[i] / (float)aecon->demand[i] ) * 100.0 );
                area_critical++;
                critical_count++;
            }
        }

        if ( area_critical > 0 )
        {
            ch_printf( ch, "  Interventions: %d\n\r\n\r", aecon->interventions );
        }
    }

    if ( critical_count == 0 )
    {
        send_to_char( "&GNo critical resource shortages detected.&w\n\r", ch );
    }
    else
    {
        ch_printf( ch, "&RTotal critical resources: %d&w\n\r", critical_count );
    }

    send_to_char( "\n\rNote: Auto-balance only intervenes when supply < 10%% of demand.\n\r", ch );
}

/*
 * View economic news
 * Syntax: econews [region]
 */
void do_econews( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AREA_ECONOMY *aecon;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* Show news for current region */
        aecon = get_area_economy( ch->in_room->area );
        if ( aecon )
            show_economic_news( ch, aecon->region_name );
        else
            show_economic_news( ch, NULL );
    }
    else
    {
        /* Show news for specific region */
        show_economic_news( ch, arg );
    }
}

/*
 * Harvest resource from node
 * Syntax: harvest <resource>
 */
void do_harvest( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    RESOURCE_NODE *node;
    int amount;
    int i;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Harvest what resource?\n\r", ch );
        return;
    }

    /* Find resource type */
    for ( i = 0; i < MAX_RESOURCE; i++ )
    {
        if ( !str_prefix( arg, resource_name( i ) ) )
            break;
    }

    if ( i >= MAX_RESOURCE )
    {
        send_to_char( "Invalid resource type.\n\r", ch );
        return;
    }

    /* Find resource node in room */
    for ( node = first_resource_node; node; node = node->next )
    {
        if ( node->room == ch->in_room && node->resource_type == i )
            break;
    }

    if ( !node )
    {
        send_to_char( "There's no source of that resource here.\n\r", ch );
        return;
    }

    if ( node->current_amount <= 0 )
    {
        send_to_char( "That resource has been depleted.\n\r", ch );
        return;
    }

    /* Harvest */
    amount = number_range( 5, 15 );
    amount = harvest_resource( ch, node, amount );

    if ( amount > 0 )
    {
        ch_printf( ch, "You harvest %d units of %s.\n\r", amount, resource_name( i ) );
        act( AT_PLAIN, "$n harvests some $t.", ch, resource_name( i ), NULL, TO_ROOM );
    }
    else
    {
        send_to_char( "You fail to harvest anything.\n\r", ch );
    }
}

/*
 * Get water from source
 * Syntax: getwater
 */
void do_getwater( CHAR_DATA *ch, char *argument )
{
    WATER_SOURCE *source;
    int amount;

    /* Find water source in room */
    for ( source = first_water_source; source; source = source->next )
    {
        if ( source->room == ch->in_room )
            break;
    }

    if ( !source )
    {
        send_to_char( "There's no water source here.\n\r", ch );
        return;
    }

    if ( source->depleted )
    {
        send_to_char( "This water source has been depleted.\n\r", ch );
        return;
    }

    /* Get water */
    amount = number_range( 50, 100 );
    amount = get_water( ch, source, amount );

    if ( amount > 0 )
    {
        ch_printf( ch, "You gather %d units of water.\n\r", amount );
        act( AT_PLAIN, "$n gathers some water.", ch, NULL, NULL, TO_ROOM );
    }
    else
    {
        send_to_char( "You fail to get any water.\n\r", ch );
    }
}

/*
 * Set or view profession
 * Syntax: profession [mob] [profession]
 */
void do_profession( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int profession;
    int i;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: profession <mob> [profession]\n\r", ch );
        send_to_char( "\n\rProfessions: farmer, rancher, hunter, miner, lumberjack, merchant,\n\r", ch );
        send_to_char( "             water_keeper, butcher, tanner, blacksmith, weaver, herbalist\n\r", ch );
        return;
    }

    victim = get_char_room( ch, arg1 );
    if ( !victim )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) )
    {
        send_to_char( "Players don't have professions.\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        /* Show current profession */
        profession = get_npc_profession( victim );
        ch_printf( ch, "%s is a %s.\n\r", victim->short_descr, profession_name( profession ) );
        return;
    }

    /* Set profession */
    profession = -1;
    for ( i = 0; i < MAX_PROFESSION; i++ )
    {
        if ( !str_prefix( arg2, profession_name( i ) ) )
        {
            profession = i;
            break;
        }
    }

    if ( profession == -1 )
    {
        send_to_char( "Invalid profession.\n\r", ch );
        return;
    }

    set_npc_profession( victim, profession );
    ch_printf( ch, "%s is now a %s.\n\r", victim->short_descr, profession_name( profession ) );
}
