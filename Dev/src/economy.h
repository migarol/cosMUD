/***************************************************************************
 * Economy System for cosMUD
 * Comprehensive economic simulation with dual-layer economy
 ***************************************************************************/

#ifndef ECONOMY_H
#define ECONOMY_H

/* Resource Types */
typedef enum
{
    RES_MEAT,           /* 0 - Animal meat for food */
    RES_GRAIN,          /* 1 - Wheat, corn, rice */
    RES_VEGETABLES,     /* 2 - Various vegetables */
    RES_WATER,          /* 3 - Critical resource */
    RES_HIDE,           /* 4 - Animal hides for leather */
    RES_WOOD,           /* 5 - Lumber for building */
    RES_STONE,          /* 6 - Building material */
    RES_ORE,            /* 7 - Metal ore for smelting */
    RES_CLOTH,          /* 8 - Processed fabric */
    RES_HERBS,          /* 9 - Medicinal plants */
    MAX_RESOURCE
} resource_type;

/* Profession Types */
typedef enum
{
    PROF_NONE,          /* 0 - No profession */
    PROF_FARMER,        /* 1 - Grows grain/vegetables */
    PROF_RANCHER,       /* 2 - Raises livestock */
    PROF_HUNTER,        /* 3 - Hunts animals */
    PROF_MINER,         /* 4 - Extracts ore/stone */
    PROF_LUMBERJACK,    /* 5 - Cuts wood */
    PROF_MERCHANT,      /* 6 - Trades goods */
    PROF_WATER_KEEPER,  /* 7 - Manages water sources */
    PROF_BUTCHER,       /* 8 - Specialist: processes meat */
    PROF_TANNER,        /* 9 - Specialist: processes hide */
    PROF_BLACKSMITH,    /* 10 - Specialist: processes ore */
    PROF_WEAVER,        /* 11 - Specialist: makes cloth */
    PROF_HERBALIST,     /* 12 - Specialist: grows herbs */
    MAX_PROFESSION
} profession_type;

/* Water Source Types */
typedef enum
{
    WATER_WELL,         /* 0 - 10,000 capacity */
    WATER_SPRING,       /* 1 - 50,000 capacity */
    WATER_RIVER,        /* 2 - Infinite capacity */
    MAX_WATER_TYPE
} water_type;

/* Economic Event Types */
typedef enum
{
    EVENT_NONE,
    EVENT_PLAGUE,       /* Kills 30% livestock */
    EVENT_DROUGHT,      /* -50% water, -30% crops */
    EVENT_BUMPER_CROP,  /* +50% grain/vegetables */
    EVENT_MINE_COLLAPSE,/* -80% ore production */
    EVENT_WOLF_ATTACK,  /* -20% livestock */
    EVENT_DISCOVERY,    /* +30% ore/stone */
    MAX_EVENT
} event_type;

/* Area Economy Data */
typedef struct area_economy AREA_ECONOMY;
struct area_economy
{
    AREA_DATA *     area;                   /* Pointer to area */
    char *          region_name;            /* Regional name */

    /* Supply & Demand */
    int             supply[MAX_RESOURCE];   /* Current supply */
    int             demand[MAX_RESOURCE];   /* Current demand */
    int             production[MAX_RESOURCE]; /* Daily production */
    int             consumption[MAX_RESOURCE]; /* Daily consumption */

    /* Pricing */
    int             base_price[MAX_RESOURCE]; /* Base price in copper */
    int             current_price[MAX_RESOURCE]; /* Dynamic price */

    /* Professions */
    int             workers[MAX_PROFESSION]; /* Workers by profession */
    int             productivity[MAX_PROFESSION]; /* Productivity % */

    /* Events */
    int             active_event;           /* Current event type */
    int             event_duration;         /* Days remaining */
    time_t          last_event;             /* Time of last event */

    /* Auto-balance tracking */
    int             interventions;          /* Count of interventions */
    time_t          last_update;            /* Last economic update */

    AREA_ECONOMY *  next;
    AREA_ECONOMY *  prev;
};

/* Regional Economy Data (Macro) */
typedef struct regional_economy REGIONAL_ECONOMY;
struct regional_economy
{
    char *          name;                   /* Region name */

    /* Aggregated data from all areas */
    int             total_supply[MAX_RESOURCE];
    int             total_demand[MAX_RESOURCE];
    int             avg_price[MAX_RESOURCE];

    /* Regional stats */
    int             total_population;       /* Total NPCs */
    int             total_workers;          /* Working NPCs */
    int             num_areas;              /* Areas in region */

    REGIONAL_ECONOMY * next;
    REGIONAL_ECONOMY * prev;
};

/* Resource Node (harvestable location) */
typedef struct resource_node RESOURCE_NODE;
struct resource_node
{
    ROOM_INDEX_DATA * room;                 /* Location */
    int             resource_type;          /* RES_* type */
    int             current_amount;         /* Current harvestable */
    int             max_amount;             /* Maximum capacity */
    int             regen_rate;             /* Daily regeneration */
    time_t          last_harvest;           /* Last harvest time */

    RESOURCE_NODE * next;
    RESOURCE_NODE * prev;
    RESOURCE_NODE * next_in_room;
};

/* Water Source */
typedef struct water_source WATER_SOURCE;
struct water_source
{
    ROOM_INDEX_DATA * room;                 /* Location */
    int             source_type;            /* WATER_* type */
    int             current_water;          /* Current water */
    int             max_capacity;           /* Max capacity */
    int             regen_rate;             /* Daily regen */
    bool            depleted;               /* Is depleted? */

    WATER_SOURCE *  next;
    WATER_SOURCE *  prev;
    WATER_SOURCE *  next_in_room;
};

/* Economic News */
typedef struct econ_news ECON_NEWS;
struct econ_news
{
    char *          headline;               /* News headline */
    char *          message;                /* Full message */
    char *          region;                 /* Region affected */
    time_t          timestamp;              /* When it happened */

    ECON_NEWS *     next;
    ECON_NEWS *     prev;
};

/* Global variables */
extern AREA_ECONOMY *   first_area_economy;
extern AREA_ECONOMY *   last_area_economy;
extern REGIONAL_ECONOMY * first_regional_economy;
extern REGIONAL_ECONOMY * last_regional_economy;
extern RESOURCE_NODE *  first_resource_node;
extern RESOURCE_NODE *  last_resource_node;
extern WATER_SOURCE *   first_water_source;
extern WATER_SOURCE *   last_water_source;
extern ECON_NEWS *      first_econ_news;
extern ECON_NEWS *      last_econ_news;

/* Function declarations */

/* Core functions */
void    init_economy_system     args( ( void ) );
void    save_economy_data       args( ( void ) );
void    load_economy_data       args( ( void ) );
void    update_economy          args( ( void ) );
void    shutdown_economy_system args( ( void ) );

/* Area economy */
AREA_ECONOMY *  create_area_economy     args( ( AREA_DATA *area, const char *region ) );
void            destroy_area_economy    args( ( AREA_ECONOMY *aecon ) );
AREA_ECONOMY *  get_area_economy        args( ( AREA_DATA *area ) );
void            update_area_economy     args( ( AREA_ECONOMY *aecon ) );

/* Regional economy */
REGIONAL_ECONOMY * create_regional_economy  args( ( const char *name ) );
void            destroy_regional_economy    args( ( REGIONAL_ECONOMY *recon ) );
REGIONAL_ECONOMY * get_regional_economy     args( ( const char *name ) );
void            update_regional_economy     args( ( REGIONAL_ECONOMY *recon ) );

/* Resource management */
RESOURCE_NODE * create_resource_node    args( ( ROOM_INDEX_DATA *room, int type, int amount ) );
void            destroy_resource_node   args( ( RESOURCE_NODE *node ) );
int             harvest_resource        args( ( CHAR_DATA *ch, RESOURCE_NODE *node, int amount ) );
void            regenerate_resources    args( ( void ) );

/* Water management */
WATER_SOURCE *  create_water_source     args( ( ROOM_INDEX_DATA *room, int type ) );
void            destroy_water_source    args( ( WATER_SOURCE *source ) );
int             get_water               args( ( CHAR_DATA *ch, WATER_SOURCE *source, int amount ) );
void            regenerate_water        args( ( void ) );

/* Pricing */
int             calculate_price         args( ( AREA_ECONOMY *aecon, int resource ) );
void            update_prices           args( ( AREA_ECONOMY *aecon ) );

/* Events */
void            trigger_random_event    args( ( AREA_ECONOMY *aecon ) );
void            trigger_specific_event  args( ( AREA_ECONOMY *aecon, int event_type ) );
void            process_event           args( ( AREA_ECONOMY *aecon ) );

/* News system */
void            add_economic_news       args( ( const char *headline, const char *message, const char *region ) );
void            show_economic_news      args( ( CHAR_DATA *ch, const char *region ) );
void            cleanup_old_news        args( ( void ) );

/* Auto-balance */
void            check_auto_balance      args( ( AREA_ECONOMY *aecon ) );

/* NPC professions */
void            set_npc_profession      args( ( CHAR_DATA *mob, int profession ) );
int             get_npc_profession      args( ( CHAR_DATA *mob ) );
void            npc_work_profession     args( ( CHAR_DATA *mob ) );

/* Utility functions */
const char *    resource_name           args( ( int type ) );
const char *    profession_name         args( ( int type ) );
const char *    event_name              args( ( int type ) );
int             get_profession_for_resource args( ( int resource ) );

/* Command declarations */
DECLARE_DO_FUN( do_ecoview      );
DECLARE_DO_FUN( do_ecoset       );
DECLARE_DO_FUN( do_ecoevent     );
DECLARE_DO_FUN( do_ecobalance   );
DECLARE_DO_FUN( do_econews      );
DECLARE_DO_FUN( do_harvest      );
DECLARE_DO_FUN( do_getwater     );
DECLARE_DO_FUN( do_profession   );

#endif /* ECONOMY_H */
