/****************************************************************************
 * Global Trade - Trade routes between areas, caravans, merchants
 ****************************************************************************/

#ifndef GLOBAL_TRADE_H
#define GLOBAL_TRADE_H

/* Trade route between two areas */
typedef struct trade_route TRADE_ROUTE;
struct trade_route {
    AREA_DATA *area_from;
    AREA_DATA *area_to;

    /* What's being traded */
    char **exported_goods;
    char **imported_goods;
    int num_exports;
    int num_imports;

    /* Economic data */
    int trade_volume_daily;     /* Gold value */
    int trade_volume;           /* Alias for compatibility */
    int established_year;
    bool active;

    /* Route details */
    int distance;               /* Rooms between areas */
    int safety;                 /* 0-100, affects caravans */
    int travel_time;            /* Hours */

    /* Caravans on this route */
    int num_active_caravans;

    TRADE_ROUTE *next;
};

/* Trade caravan (actual mob group) */
typedef struct trade_caravan TRADE_CARAVAN;
struct trade_caravan {
    CHAR_DATA *lead_merchant;
    CHAR_DATA **guards;
    int num_guards;

    /* Cargo */
    OBJ_DATA **cargo_items;
    int num_cargo_items;
    int cargo_value;

    /* Journey */
    TRADE_ROUTE *route;
    AREA_DATA *origin;
    AREA_DATA *destination;
    ROOM_INDEX_DATA *current_location;
    bool outbound;              /* TRUE = going to destination, FALSE = returning */

    /* Status */
    bool arrived;
    bool attacked;
    int goods_lost;

    time_t departed;
    time_t expected_arrival;

    TRADE_CARAVAN *next;
};

void init_global_trade(void);
void load_trade_routes(void);
void save_trade_routes(void);
void establish_trade_route(AREA_DATA *from, AREA_DATA *to);
void create_caravan(TRADE_ROUTE *route);
void caravan_travel_update(void);
void global_trade_update(void);

#endif
