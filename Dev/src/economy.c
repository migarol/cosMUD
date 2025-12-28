/***************************************************************************
 * Economy System for cosMUD - Core Implementation
 * Handles dual-layer economic simulation with resource management
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "mud.h"
#include "economy.h"

/* Global economy lists */
AREA_ECONOMY *      first_area_economy      = NULL;
AREA_ECONOMY *      last_area_economy       = NULL;
REGIONAL_ECONOMY *  first_regional_economy  = NULL;
REGIONAL_ECONOMY *  last_regional_economy   = NULL;
RESOURCE_NODE *     first_resource_node     = NULL;
RESOURCE_NODE *     last_resource_node      = NULL;
WATER_SOURCE *      first_water_source      = NULL;
WATER_SOURCE *      last_water_source       = NULL;
ECON_NEWS *         first_econ_news         = NULL;
ECON_NEWS *         last_econ_news          = NULL;

/* Base prices (in copper coins) */
const int base_resource_prices[MAX_RESOURCE] =
{
    50,     /* RES_MEAT */
    20,     /* RES_GRAIN */
    15,     /* RES_VEGETABLES */
    5,      /* RES_WATER */
    40,     /* RES_HIDE */
    30,     /* RES_WOOD */
    25,     /* RES_STONE */
    100,    /* RES_ORE */
    60,     /* RES_CLOTH */
    80      /* RES_HERBS */
};

/* Resource name table */
const char * resource_names[MAX_RESOURCE] =
{
    "meat", "grain", "vegetables", "water", "hide",
    "wood", "stone", "ore", "cloth", "herbs"
};

/* Profession name table */
const char * profession_names[MAX_PROFESSION] =
{
    "none", "farmer", "rancher", "hunter", "miner", "lumberjack",
    "merchant", "water keeper", "butcher", "tanner", "blacksmith",
    "weaver", "herbalist"
};

/* Event name table */
const char * event_names[MAX_EVENT] =
{
    "none", "plague", "drought", "bumper crop", "mine collapse",
    "wolf attack", "discovery"
};

/*
 * Utility function to get resource name
 */
const char * resource_name( int type )
{
    if ( type < 0 || type >= MAX_RESOURCE )
        return "unknown";
    return resource_names[type];
}

/*
 * Utility function to get profession name
 */
const char * profession_name( int type )
{
    if ( type < 0 || type >= MAX_PROFESSION )
        return "unknown";
    return profession_names[type];
}

/*
 * Utility function to get event name
 */
const char * event_name( int type )
{
    if ( type < 0 || type >= MAX_EVENT )
        return "unknown";
    return event_names[type];
}

/*
 * Get resource type by name (case-insensitive)
 */
int get_resource_by_name( const char *name )
{
    int i;

    if ( !name || !str_cmp( name, "" ) )
        return -1;

    for ( i = 0; i < MAX_RESOURCE; i++ )
    {
        if ( !str_cmp( name, resource_names[i] ) )
            return i;
    }

    return -1;
}

/*
 * Get profession type by name (case-insensitive)
 */
int get_profession_by_name( const char *name )
{
    int i;

    if ( !name || !str_cmp( name, "" ) )
        return PROF_NONE;

    for ( i = 0; i < MAX_PROFESSION; i++ )
    {
        if ( !str_cmp( name, profession_names[i] ) )
            return i;
        /* Also check partial matches */
        if ( !str_prefix( name, profession_names[i] ) )
            return i;
    }

    return PROF_NONE;
}

/*
 * Get profession that produces a resource
 */
int get_profession_for_resource( int resource )
{
    switch( resource )
    {
        case RES_MEAT:       return PROF_RANCHER;
        case RES_GRAIN:      return PROF_FARMER;
        case RES_VEGETABLES: return PROF_FARMER;
        case RES_WATER:      return PROF_WATER_KEEPER;
        case RES_HIDE:       return PROF_HUNTER;
        case RES_WOOD:       return PROF_LUMBERJACK;
        case RES_STONE:      return PROF_MINER;
        case RES_ORE:        return PROF_MINER;
        case RES_CLOTH:      return PROF_WEAVER;
        case RES_HERBS:      return PROF_HERBALIST;
        default:             return PROF_NONE;
    }
}

/*
 * Initialize economy system
 */
void init_economy_system( void )
{
    AREA_DATA *area;

    log_string( "Initializing economy system..." );

    /* Try to load existing economy data */
    load_economy_data();

    /* Create area economies for areas that don't have one */
    for ( area = first_area; area; area = area->next )
    {
        if ( !get_area_economy( area ) )
        {
            /* Default to generic region based on area name */
            create_area_economy( area, "Unknown Region" );
        }
    }

    log_string( "Economy system initialized." );
}

/*
 * Create area economy
 */
AREA_ECONOMY * create_area_economy( AREA_DATA *area, const char *region )
{
    AREA_ECONOMY *aecon;
    int i;

    CREATE( aecon, AREA_ECONOMY, 1 );
    aecon->area = area;
    aecon->region_name = str_dup( region );

    /* Initialize supply/demand with defaults */
    for ( i = 0; i < MAX_RESOURCE; i++ )
    {
        aecon->supply[i] = 1000;        /* Start with moderate supply */
        aecon->demand[i] = 800;         /* Moderate demand */
        aecon->production[i] = 100;     /* Base production */
        aecon->consumption[i] = 80;     /* Base consumption */
        aecon->base_price[i] = base_resource_prices[i];
        aecon->current_price[i] = base_resource_prices[i];
    }

    /* Water is abundant */
    aecon->supply[RES_WATER] = 10000;
    aecon->production[RES_WATER] = 500;

    /* Initialize professions */
    for ( i = 0; i < MAX_PROFESSION; i++ )
    {
        aecon->workers[i] = 0;
        aecon->productivity[i] = 100;   /* 100% productivity */
    }

    /* Events */
    aecon->active_event = EVENT_NONE;
    aecon->event_duration = 0;
    aecon->last_event = current_time;

    /* Stats */
    aecon->interventions = 0;
    aecon->last_update = current_time;

    /* Link into list */
    LINK( aecon, first_area_economy, last_area_economy, next, prev );

    return aecon;
}

/*
 * Destroy area economy
 */
void destroy_area_economy( AREA_ECONOMY *aecon )
{
    if ( !aecon )
        return;

    UNLINK( aecon, first_area_economy, last_area_economy, next, prev );
    DISPOSE( aecon->region_name );
    DISPOSE( aecon );
}

/*
 * Get area economy by area pointer
 */
AREA_ECONOMY * get_area_economy( AREA_DATA *area )
{
    AREA_ECONOMY *aecon;

    for ( aecon = first_area_economy; aecon; aecon = aecon->next )
    {
        if ( aecon->area == area )
            return aecon;
    }

    return NULL;
}

/*
 * Create regional economy
 */
REGIONAL_ECONOMY * create_regional_economy( const char *name )
{
    REGIONAL_ECONOMY *recon;
    int i;

    CREATE( recon, REGIONAL_ECONOMY, 1 );
    recon->name = str_dup( name );

    /* Initialize aggregated data */
    for ( i = 0; i < MAX_RESOURCE; i++ )
    {
        recon->total_supply[i] = 0;
        recon->total_demand[i] = 0;
        recon->avg_price[i] = base_resource_prices[i];
    }

    recon->total_population = 0;
    recon->total_workers = 0;
    recon->num_areas = 0;

    /* Link into list */
    LINK( recon, first_regional_economy, last_regional_economy, next, prev );

    return recon;
}

/*
 * Destroy regional economy
 */
void destroy_regional_economy( REGIONAL_ECONOMY *recon )
{
    if ( !recon )
        return;

    UNLINK( recon, first_regional_economy, last_regional_economy, next, prev );
    DISPOSE( recon->name );
    DISPOSE( recon );
}

/*
 * Get regional economy by name
 */
REGIONAL_ECONOMY * get_regional_economy( const char *name )
{
    REGIONAL_ECONOMY *recon;

    for ( recon = first_regional_economy; recon; recon = recon->next )
    {
        if ( !str_cmp( recon->name, name ) )
            return recon;
    }

    return NULL;
}

/*
 * Update regional economy from area data
 */
void update_regional_economy( REGIONAL_ECONOMY *recon )
{
    AREA_ECONOMY *aecon;
    int i;
    int count = 0;

    if ( !recon )
        return;

    /* Reset totals */
    for ( i = 0; i < MAX_RESOURCE; i++ )
    {
        recon->total_supply[i] = 0;
        recon->total_demand[i] = 0;
        recon->avg_price[i] = 0;
    }
    recon->total_population = 0;
    recon->total_workers = 0;
    recon->num_areas = 0;

    /* Aggregate from all areas in this region */
    for ( aecon = first_area_economy; aecon; aecon = aecon->next )
    {
        if ( !str_cmp( aecon->region_name, recon->name ) )
        {
            for ( i = 0; i < MAX_RESOURCE; i++ )
            {
                recon->total_supply[i] += aecon->supply[i];
                recon->total_demand[i] += aecon->demand[i];
                recon->avg_price[i] += aecon->current_price[i];
            }

            for ( i = 0; i < MAX_PROFESSION; i++ )
                recon->total_workers += aecon->workers[i];

            recon->num_areas++;
            count++;
        }
    }

    /* Calculate averages */
    if ( count > 0 )
    {
        for ( i = 0; i < MAX_RESOURCE; i++ )
            recon->avg_price[i] /= count;
    }
}

/*
 * Create resource node
 */
RESOURCE_NODE * create_resource_node( ROOM_INDEX_DATA *room, int type, int amount )
{
    RESOURCE_NODE *node;

    if ( !room || type < 0 || type >= MAX_RESOURCE )
        return NULL;

    CREATE( node, RESOURCE_NODE, 1 );
    node->room = room;
    node->resource_type = type;
    node->current_amount = amount;
    node->max_amount = amount;
    node->regen_rate = amount / 10;  /* 10% regen per day */
    node->last_harvest = current_time;

    /* Link into global list */
    LINK( node, first_resource_node, last_resource_node, next, prev );

    return node;
}

/*
 * Destroy resource node
 */
void destroy_resource_node( RESOURCE_NODE *node )
{
    if ( !node )
        return;

    UNLINK( node, first_resource_node, last_resource_node, next, prev );
    DISPOSE( node );
}

/*
 * Harvest resource from node
 */
int harvest_resource( CHAR_DATA *ch, RESOURCE_NODE *node, int amount )
{
    int harvested;

    if ( !ch || !node )
        return 0;

    /* Can't harvest more than available */
    harvested = UMIN( amount, node->current_amount );

    if ( harvested <= 0 )
        return 0;

    /* Reduce node supply */
    node->current_amount -= harvested;
    node->last_harvest = current_time;

    return harvested;
}

/*
 * Regenerate all resource nodes
 */
void regenerate_resources( void )
{
    RESOURCE_NODE *node;
    int regen;

    for ( node = first_resource_node; node; node = node->next )
    {
        /* Slow regeneration */
        regen = node->regen_rate;

        /* Don't exceed max */
        if ( node->current_amount + regen > node->max_amount )
            node->current_amount = node->max_amount;
        else
            node->current_amount += regen;
    }
}

/*
 * Create water source
 */
WATER_SOURCE * create_water_source( ROOM_INDEX_DATA *room, int type )
{
    WATER_SOURCE *source;

    if ( !room || type < 0 || type >= MAX_WATER_TYPE )
        return NULL;

    CREATE( source, WATER_SOURCE, 1 );
    source->room = room;
    source->source_type = type;
    source->depleted = FALSE;

    /* Set capacity based on type */
    switch( type )
    {
        case WATER_WELL:
            source->max_capacity = 10000;
            source->regen_rate = 500;
            break;
        case WATER_SPRING:
            source->max_capacity = 50000;
            source->regen_rate = 2500;
            break;
        case WATER_RIVER:
            source->max_capacity = 999999;
            source->regen_rate = 999999;
            break;
        default:
            source->max_capacity = 1000;
            source->regen_rate = 100;
            break;
    }

    source->current_water = source->max_capacity;

    /* Link into global list */
    LINK( source, first_water_source, last_water_source, next, prev );

    return source;
}

/*
 * Destroy water source
 */
void destroy_water_source( WATER_SOURCE *source )
{
    if ( !source )
        return;

    UNLINK( source, first_water_source, last_water_source, next, prev );
    DISPOSE( source );
}

/*
 * Get water from source
 */
int get_water( CHAR_DATA *ch, WATER_SOURCE *source, int amount )
{
    int obtained;

    if ( !ch || !source )
        return 0;

    if ( source->depleted )
        return 0;

    /* Can't get more than available */
    obtained = UMIN( amount, source->current_water );

    /* Infinite sources never deplete */
    if ( source->source_type != WATER_RIVER )
    {
        source->current_water -= obtained;

        if ( source->current_water <= 0 )
        {
            source->depleted = TRUE;
            source->current_water = 0;
        }
    }

    return obtained;
}

/*
 * Regenerate all water sources
 */
void regenerate_water( void )
{
    WATER_SOURCE *source;

    for ( source = first_water_source; source; source = source->next )
    {
        /* Rivers never need regen */
        if ( source->source_type == WATER_RIVER )
            continue;

        /* Regenerate */
        source->current_water += source->regen_rate;

        if ( source->current_water >= source->max_capacity )
        {
            source->current_water = source->max_capacity;
            source->depleted = FALSE;
        }
    }
}

/*
 * Calculate dynamic price based on supply and demand
 */
int calculate_price( AREA_ECONOMY *aecon, int resource )
{
    int price;
    float ratio;

    if ( !aecon || resource < 0 || resource >= MAX_RESOURCE )
        return 100;

    /* Avoid division by zero */
    if ( aecon->supply[resource] <= 0 )
        ratio = 3.0;  /* Maximum ratio when no supply */
    else
        ratio = (float)aecon->demand[resource] / (float)aecon->supply[resource];

    /* Price = base * ratio, capped at 50%-300% */
    price = (int)( aecon->base_price[resource] * ratio );

    /* Cap at min 50% */
    if ( price < aecon->base_price[resource] / 2 )
        price = aecon->base_price[resource] / 2;

    /* Cap at max 300% */
    if ( price > aecon->base_price[resource] * 3 )
        price = aecon->base_price[resource] * 3;

    return price;
}

/*
 * Update all prices in area economy
 */
void update_prices( AREA_ECONOMY *aecon )
{
    int i;

    if ( !aecon )
        return;

    for ( i = 0; i < MAX_RESOURCE; i++ )
    {
        aecon->current_price[i] = calculate_price( aecon, i );
    }
}

/*
 * Process active event
 */
void process_event( AREA_ECONOMY *aecon )
{
    if ( !aecon || aecon->active_event == EVENT_NONE )
        return;

    /* Apply event effects */
    switch( aecon->active_event )
    {
        case EVENT_PLAGUE:
            /* -30% meat production */
            aecon->production[RES_MEAT] = aecon->production[RES_MEAT] * 70 / 100;
            break;

        case EVENT_DROUGHT:
            /* -50% water, -30% crops */
            aecon->production[RES_WATER] = aecon->production[RES_WATER] * 50 / 100;
            aecon->production[RES_GRAIN] = aecon->production[RES_GRAIN] * 70 / 100;
            aecon->production[RES_VEGETABLES] = aecon->production[RES_VEGETABLES] * 70 / 100;
            break;

        case EVENT_BUMPER_CROP:
            /* +50% grain/vegetables */
            aecon->production[RES_GRAIN] = aecon->production[RES_GRAIN] * 150 / 100;
            aecon->production[RES_VEGETABLES] = aecon->production[RES_VEGETABLES] * 150 / 100;
            break;

        case EVENT_MINE_COLLAPSE:
            /* -80% ore */
            aecon->production[RES_ORE] = aecon->production[RES_ORE] * 20 / 100;
            aecon->production[RES_STONE] = aecon->production[RES_STONE] * 50 / 100;
            break;

        case EVENT_WOLF_ATTACK:
            /* -20% livestock */
            aecon->production[RES_MEAT] = aecon->production[RES_MEAT] * 80 / 100;
            break;

        case EVENT_DISCOVERY:
            /* +30% ore/stone */
            aecon->production[RES_ORE] = aecon->production[RES_ORE] * 130 / 100;
            aecon->production[RES_STONE] = aecon->production[RES_STONE] * 130 / 100;
            break;
    }

    /* Decrease duration */
    aecon->event_duration--;

    if ( aecon->event_duration <= 0 )
    {
        /* Event ended - restore production to normal */
        aecon->active_event = EVENT_NONE;
        /* Note: production will be recalculated on next update */
    }
}

/*
 * Trigger random event (very rare - 1 per year)
 */
void trigger_random_event( AREA_ECONOMY *aecon )
{
    int event_type;
    time_t time_since_last;
    char buf[MAX_STRING_LENGTH];

    if ( !aecon )
        return;

    /* Check if enough time has passed (1 year = 365 days = 31536000 seconds) */
    time_since_last = current_time - aecon->last_event;

    /* Very rare - only 1% chance per day if a year has passed */
    if ( time_since_last < 31536000 || number_percent() > 1 )
        return;

    /* Random event type */
    event_type = number_range( EVENT_PLAGUE, MAX_EVENT - 1 );

    /* Trigger it */
    trigger_specific_event( aecon, event_type );
}

/*
 * Trigger specific event
 */
void trigger_specific_event( AREA_ECONOMY *aecon, int event_type )
{
    char buf[MAX_STRING_LENGTH];

    if ( !aecon || event_type <= EVENT_NONE || event_type >= MAX_EVENT )
        return;

    /* Cancel existing event */
    if ( aecon->active_event != EVENT_NONE )
        aecon->active_event = EVENT_NONE;

    /* Set new event */
    aecon->active_event = event_type;
    aecon->event_duration = number_range( 7, 30 );  /* 7-30 days */
    aecon->last_event = current_time;

    /* Generate news */
    sprintf( buf, "%s strikes %s!", event_name( event_type ), aecon->area->name );
    add_economic_news( buf, "Economic impact expected.", aecon->region_name );
}

/*
 * Add economic news
 */
void add_economic_news( const char *headline, const char *message, const char *region )
{
    ECON_NEWS *news;

    CREATE( news, ECON_NEWS, 1 );
    news->headline = str_dup( headline );
    news->message = str_dup( message );
    news->region = str_dup( region );
    news->timestamp = current_time;

    LINK( news, first_econ_news, last_econ_news, next, prev );
}

/*
 * Show economic news to character
 */
void show_economic_news( CHAR_DATA *ch, const char *region )
{
    ECON_NEWS *news;
    int count = 0;

    send_to_char( "&Y-=[ Economic News ]=-&w\n\r\n\r", ch );

    for ( news = first_econ_news; news; news = news->next )
    {
        /* Filter by region if specified */
        if ( region && str_cmp( news->region, region ) )
            continue;

        ch_printf( ch, "&W%s&w\n\r", news->headline );
        ch_printf( ch, "%s\n\r", news->message );
        ch_printf( ch, "&g[%s - %s]&w\n\r\n\r",
                   news->region, ctime( &news->timestamp ) );

        count++;
        if ( count >= 10 )
            break;
    }

    if ( count == 0 )
        send_to_char( "No recent news.\n\r", ch );
}

/*
 * Clean up old news (older than 30 days)
 */
void cleanup_old_news( void )
{
    ECON_NEWS *news, *news_next;
    time_t cutoff = current_time - ( 30 * 24 * 60 * 60 );

    for ( news = first_econ_news; news; news = news_next )
    {
        news_next = news->next;

        if ( news->timestamp < cutoff )
        {
            UNLINK( news, first_econ_news, last_econ_news, next, prev );
            DISPOSE( news->headline );
            DISPOSE( news->message );
            DISPOSE( news->region );
            DISPOSE( news );
        }
    }
}

/*
 * Auto-balance system - only intervenes when critical
 */
void check_auto_balance( AREA_ECONOMY *aecon )
{
    int i;
    int deficit;
    char buf[MAX_STRING_LENGTH];

    if ( !aecon )
        return;

    for ( i = 0; i < MAX_RESOURCE; i++ )
    {
        /* Check if supply is critically low (<10% of demand) */
        if ( aecon->supply[i] < aecon->demand[i] / 10 )
        {
            /* Calculate deficit */
            deficit = aecon->demand[i] / 10 - aecon->supply[i];

            /* Add only 10% of deficit */
            aecon->supply[i] += deficit / 10;

            /* Track intervention */
            aecon->interventions++;

            /* Generate news */
            sprintf( buf, "Emergency %s supplies arrive in %s",
                     resource_name( i ), aecon->area->name );
            add_economic_news( buf, "Crisis temporarily averted.", aecon->region_name );
        }
    }
}

/*
 * Update area economy (called daily)
 */
void update_area_economy( AREA_ECONOMY *aecon )
{
    int i;

    if ( !aecon )
        return;

    /* Apply production */
    for ( i = 0; i < MAX_RESOURCE; i++ )
    {
        aecon->supply[i] += aecon->production[i];
    }

    /* Apply consumption */
    for ( i = 0; i < MAX_RESOURCE; i++ )
    {
        aecon->supply[i] -= aecon->consumption[i];

        /* Don't go negative */
        if ( aecon->supply[i] < 0 )
            aecon->supply[i] = 0;
    }

    /* Process active events */
    if ( aecon->active_event != EVENT_NONE )
        process_event( aecon );

    /* Check for random events (very rare) */
    trigger_random_event( aecon );

    /* Update prices */
    update_prices( aecon );

    /* Check auto-balance */
    check_auto_balance( aecon );

    aecon->last_update = current_time;
}

/*
 * Main economy update (called periodically)
 */
void update_economy( void )
{
    AREA_ECONOMY *aecon;
    REGIONAL_ECONOMY *recon;

    /* Update all area economies */
    for ( aecon = first_area_economy; aecon; aecon = aecon->next )
    {
        update_area_economy( aecon );
    }

    /* Update regional economies */
    for ( recon = first_regional_economy; recon; recon = recon->next )
    {
        update_regional_economy( recon );
    }

    /* Regenerate resources */
    regenerate_resources();
    regenerate_water();

    /* Clean up old news */
    cleanup_old_news();
}

/*
 * Save economy data
 */
void save_economy_data( void )
{
    FILE *fp;
    AREA_ECONOMY *aecon;
    int i;

    log_string( "Saving economy data..." );

    if ( !( fp = fopen( ECONOMY_FILE, "w" ) ) )
    {
        bug( "save_economy: cannot open economy.dat for writing" );
        perror( ECONOMY_FILE );
        return;
    }

    /* Save each area economy */
    for ( aecon = first_area_economy; aecon; aecon = aecon->next )
    {
        fprintf( fp, "#AREAECONOMY\n" );
        fprintf( fp, "AreaName     %s~\n", aecon->area->filename );
        fprintf( fp, "RegionName   %s~\n", aecon->region_name );

        /* Save supply/demand */
        fprintf( fp, "Supply      " );
        for ( i = 0; i < MAX_RESOURCE; i++ )
            fprintf( fp, " %d", aecon->supply[i] );
        fprintf( fp, "\n" );

        fprintf( fp, "Demand      " );
        for ( i = 0; i < MAX_RESOURCE; i++ )
            fprintf( fp, " %d", aecon->demand[i] );
        fprintf( fp, "\n" );

        fprintf( fp, "Production  " );
        for ( i = 0; i < MAX_RESOURCE; i++ )
            fprintf( fp, " %d", aecon->production[i] );
        fprintf( fp, "\n" );

        fprintf( fp, "Consumption " );
        for ( i = 0; i < MAX_RESOURCE; i++ )
            fprintf( fp, " %d", aecon->consumption[i] );
        fprintf( fp, "\n" );

        /* Save workers */
        fprintf( fp, "Workers     " );
        for ( i = 0; i < MAX_PROFESSION; i++ )
            fprintf( fp, " %d", aecon->workers[i] );
        fprintf( fp, "\n" );

        fprintf( fp, "Productivity" );
        for ( i = 0; i < MAX_PROFESSION; i++ )
            fprintf( fp, " %d", aecon->productivity[i] );
        fprintf( fp, "\n" );

        /* Save event data */
        fprintf( fp, "Event        %d %d %ld\n",
                 aecon->active_event, aecon->event_duration, aecon->last_event );

        fprintf( fp, "Interventions %d\n", aecon->interventions );
        fprintf( fp, "End\n\n" );
    }

    fprintf( fp, "#END\n" );
    fclose( fp );

    log_string( "Economy data saved." );
}

/*
 * Load economy data
 */
void load_economy_data( void )
{
    FILE *fp;
    AREA_ECONOMY *aecon;
    AREA_DATA *area;
    char *word;
    char area_name[256];
    char region_name[256];
    bool fMatch;
    int i;

    log_string( "Loading economy data..." );

    if ( !( fp = fopen( ECONOMY_FILE, "r" ) ) )
    {
        log_string( "No economy file found, starting fresh." );
        return;
    }

    for ( ; ; )
    {
        word = fread_word( fp );

        if ( !str_cmp( word, "#AREAECONOMY" ) )
        {
            aecon = NULL;
            area_name[0] = '\0';
            region_name[0] = '\0';

            for ( ; ; )
            {
                word = fread_word( fp );
                fMatch = FALSE;

                switch ( UPPER( word[0] ) )
                {
                    case '*':
                        fMatch = TRUE;
                        fread_to_eol( fp );
                        break;

                    case 'A':
                        if ( !str_cmp( word, "AreaName" ) )
                        {
                            fMatch = TRUE;
                            strcpy( area_name, fread_string_nohash( fp ) );
                        }
                        break;

                    case 'C':
                        if ( !str_cmp( word, "Consumption" ) )
                        {
                            fMatch = TRUE;
                            for ( i = 0; i < MAX_RESOURCE; i++ )
                                aecon->consumption[i] = fread_number( fp );
                        }
                        break;

                    case 'D':
                        if ( !str_cmp( word, "Demand" ) )
                        {
                            fMatch = TRUE;
                            for ( i = 0; i < MAX_RESOURCE; i++ )
                                aecon->demand[i] = fread_number( fp );
                        }
                        break;

                    case 'E':
                        if ( !str_cmp( word, "Event" ) )
                        {
                            fMatch = TRUE;
                            aecon->active_event = fread_number( fp );
                            aecon->event_duration = fread_number( fp );
                            aecon->last_event = fread_number( fp );
                        }
                        if ( !str_cmp( word, "End" ) )
                        {
                            fMatch = TRUE;
                            goto area_done;
                        }
                        break;

                    case 'I':
                        if ( !str_cmp( word, "Interventions" ) )
                        {
                            fMatch = TRUE;
                            aecon->interventions = fread_number( fp );
                        }
                        break;

                    case 'P':
                        if ( !str_cmp( word, "Production" ) )
                        {
                            fMatch = TRUE;
                            for ( i = 0; i < MAX_RESOURCE; i++ )
                                aecon->production[i] = fread_number( fp );
                        }
                        if ( !str_cmp( word, "Productivity" ) )
                        {
                            fMatch = TRUE;
                            for ( i = 0; i < MAX_PROFESSION; i++ )
                                aecon->productivity[i] = fread_number( fp );
                        }
                        break;

                    case 'R':
                        if ( !str_cmp( word, "RegionName" ) )
                        {
                            fMatch = TRUE;
                            strcpy( region_name, fread_string_nohash( fp ) );

                            /* Now we have both names, create economy */
                            area = get_area( area_name );
                            if ( area )
                            {
                                aecon = create_area_economy( area, region_name );
                            }
                        }
                        break;

                    case 'S':
                        if ( !str_cmp( word, "Supply" ) )
                        {
                            fMatch = TRUE;
                            for ( i = 0; i < MAX_RESOURCE; i++ )
                                aecon->supply[i] = fread_number( fp );
                        }
                        break;

                    case 'W':
                        if ( !str_cmp( word, "Workers" ) )
                        {
                            fMatch = TRUE;
                            for ( i = 0; i < MAX_PROFESSION; i++ )
                                aecon->workers[i] = fread_number( fp );
                        }
                        break;
                }

                if ( !fMatch )
                    bug( "load_economy: unknown keyword %s", word );
            }

            area_done:
            continue;
        }
        else if ( !str_cmp( word, "#END" ) )
            break;
    }

    fclose( fp );
    log_string( "Economy data loaded." );
}

/*
 * Shutdown economy system
 */
void shutdown_economy_system( void )
{
    save_economy_data();
    log_string( "Economy system shutdown." );
}

/*
 * Set NPC profession
 */
void set_npc_profession( CHAR_DATA *mob, int profession )
{
    AREA_ECONOMY *aecon;

    if ( !mob || !IS_NPC( mob ) )
        return;

    if ( profession < 0 || profession >= MAX_PROFESSION )
        return;

    /* Get area economy */
    aecon = get_area_economy( mob->in_room->area );
    if ( !aecon )
        return;

    /* Remove from old profession */
    if ( mob->profession > 0 && mob->profession < MAX_PROFESSION )
        aecon->workers[mob->profession]--;

    /* Set new profession */
    mob->profession = profession;

    /* Add to new profession */
    if ( profession > 0 )
        aecon->workers[profession]++;
}

/*
 * Get NPC profession
 */
int get_npc_profession( CHAR_DATA *mob )
{
    if ( !mob || !IS_NPC( mob ) )
        return PROF_NONE;

    return mob->profession;
}

/*
 * NPC performs work based on profession
 */
void npc_work_profession( CHAR_DATA *mob )
{
    AREA_ECONOMY *aecon;
    int profession;
    int productivity;
    int amount;

    if ( !mob || !IS_NPC( mob ) )
        return;

    profession = get_npc_profession( mob );
    if ( profession == PROF_NONE )
        return;

    aecon = get_area_economy( mob->in_room->area );
    if ( !aecon )
        return;

    /* Get productivity */
    productivity = aecon->productivity[profession];

    /* Simple work: add to production based on profession */
    switch( profession )
    {
        case PROF_FARMER:
            amount = number_range( 5, 15 ) * productivity / 100;
            aecon->production[RES_GRAIN] += amount;
            aecon->production[RES_VEGETABLES] += amount / 2;
            break;

        case PROF_RANCHER:
            amount = number_range( 3, 10 ) * productivity / 100;
            aecon->production[RES_MEAT] += amount;
            aecon->production[RES_HIDE] += amount / 2;
            break;

        case PROF_HUNTER:
            amount = number_range( 2, 8 ) * productivity / 100;
            aecon->production[RES_MEAT] += amount;
            aecon->production[RES_HIDE] += amount;
            break;

        case PROF_MINER:
            amount = number_range( 4, 12 ) * productivity / 100;
            aecon->production[RES_ORE] += amount;
            aecon->production[RES_STONE] += amount;
            break;

        case PROF_LUMBERJACK:
            amount = number_range( 5, 15 ) * productivity / 100;
            aecon->production[RES_WOOD] += amount;
            break;

        case PROF_WATER_KEEPER:
            amount = number_range( 20, 50 ) * productivity / 100;
            aecon->production[RES_WATER] += amount;
            break;

        case PROF_HERBALIST:
            amount = number_range( 2, 6 ) * productivity / 100;
            aecon->production[RES_HERBS] += amount;
            break;

        /* Specialists process resources */
        case PROF_BUTCHER:
        case PROF_TANNER:
        case PROF_BLACKSMITH:
        case PROF_WEAVER:
            /* Specialists convert resources (handled elsewhere) */
            break;
    }
}
