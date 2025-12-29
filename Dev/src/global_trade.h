/****************************************************************************
 * Global Trade - Trade routes between areas, caravans, merchants
 ****************************************************************************/

#ifndef GLOBAL_TRADE_H
#define GLOBAL_TRADE_H

typedef struct trade_route {
    AREA_DATA *from_area;
    AREA_DATA *to_area;
    int trade_volume;
    char *goods_traded;
    bool active;
} TRADE_ROUTE;

void init_global_trade(void);
void establish_trade_route(AREA_DATA *from, AREA_DATA *to);
void trade_route_update(void);
void calculate_trade_flow(void);

#endif
