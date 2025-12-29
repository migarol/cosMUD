/*****************************************************************************
 * Global Trade System - Inter-Area Trade
 *
 * Establishes trade routes between areas
 * Trade caravans (actual mobs) travel between cities
 * Resource exchange affects local economies
 * Economic impact ripples through connected areas
 *
 * Integration: economy.h, world_context.h, world_history_tracker.h
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "global_trade.h"
#include "world_context.h"
#include "world_history_tracker.h"
#include "economy.h"
#include "periodicos.h"

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

TRADE_ROUTE *first_route = NULL;
TRADE_CARAVAN *first_caravan = NULL;
int total_routes = 0;
int total_caravans = 0;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_global_trade(void)
{
    log_string("Initializing Global Trade System...");
    first_route = NULL;
    first_caravan = NULL;
    total_routes = 0;
    total_caravans = 0;
    log_string("Global Trade System initialized.");
}

void load_trade_routes(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "trade_routes.dat");

    if ((fp = fopen(filename, "r")) == NULL)
    {
        log_string("No saved trade routes to load.");
        return;
    }

    log_string("Loading trade routes...");
    fclose(fp);
}

void save_trade_routes(void)
{
    FILE *fp;
    char filename[256];
    TRADE_ROUTE *route;

    sprintf(filename, "%s%s", SYSTEM_DIR, "trade_routes.dat");

    if ((fp = fopen(filename, "w")) == NULL)
    {
        log_string("ERROR: Cannot save trade routes!");
        return;
    }

    fprintf(fp, "#TRADE_ROUTES\n");

    for (route = first_route; route; route = route->next)
    {
        if (route->active)
        {
            fprintf(fp, "Route %d %d\n", route->area_from->vnum, route->area_to->vnum);
            fprintf(fp, "Volume %d\n", route->trade_volume_daily);
            fprintf(fp, "Distance %d\n", route->distance);
            fprintf(fp, "Safety %d\n", route->safety);
            fprintf(fp, "End\n\n");
        }
    }

    fprintf(fp, "#END\n");
    fclose(fp);
}

/*****************************************************************************
 * Trade Route Creation
 *****************************************************************************/

TRADE_ROUTE *create_trade_route(AREA_DATA *from, AREA_DATA *to)
{
    TRADE_ROUTE *route;
    AREA_CONTEXT *ctx_from, *ctx_to;

    if (!from || !to)
        return NULL;

    /* Check if route already exists */
    for (route = first_route; route; route = route->next)
    {
        if ((route->area_from == from && route->area_to == to) ||
            (route->area_from == to && route->area_to == from))
        {
            log_string("TRADE: Route already exists between %s and %s",
                       from->name, to->name);
            return route;
        }
    }

    CREATE(route, TRADE_ROUTE, 1);
    route->area_from = from;
    route->area_to = to;
    route->exported_goods = NULL;
    route->imported_goods = NULL;
    route->num_exports = 0;
    route->num_imports = 0;
    route->trade_volume_daily = 0;
    route->established_year = 0;
    route->active = TRUE;
    route->num_active_caravans = 0;
    route->next = first_route;
    first_route = route;
    total_routes++;

    /* Calculate route details */
    route->distance = calculate_area_distance(from, to);
    route->travel_time = route->distance / 10; /* ~10 rooms per hour */

    /* Check route safety */
    ctx_from = analyze_area(from);
    ctx_to = analyze_area(to);
    if (ctx_from && ctx_to)
    {
        AREA_VITAL_SIGNS *vitals_from = beeler_get_area_health(from);
        AREA_VITAL_SIGNS *vitals_to = beeler_get_area_health(to);

        if (vitals_from && vitals_to)
        {
            route->safety = (vitals_from->safety_score + vitals_to->safety_score) / 2;
        }
        else
        {
            route->safety = 50; /* Medium safety */
        }
    }
    else
    {
        route->safety = 50;
    }

    /* Determine traded goods based on area resources */
    determine_trade_goods(route);

    log_string("TRADE: Established route between %s and %s (%d rooms, safety %d)",
               from->name, to->name, route->distance, route->safety);

    /* Announce */
    smart_announce(
        "New Trade Route Established",
        "Economic ties strengthen between regions",
        EVENT_CATEGORY_TRADE,
        ANNOUNCE_PRIORITY_MEDIUM,
        from->name
    );

    /* Record in history */
    record_world_event(EVENT_ECONOMIC, "Trade route established", 7);

    return route;
}

void determine_trade_goods(TRADE_ROUTE *route)
{
    AREA_CONTEXT *ctx_from, *ctx_to;
    char **from_exports, **to_exports;

    if (!route)
        return;

    ctx_from = analyze_area(route->area_from);
    ctx_to = analyze_area(route->area_to);

    if (ctx_from)
        from_exports = determine_exportable_resources(ctx_from);

    if (ctx_to)
        to_exports = determine_exportable_resources(ctx_to);

    /* For now, simple placeholder */
    CREATE(route->exported_goods, char *, 5);
    route->exported_goods[0] = str_dup("grain");
    route->exported_goods[1] = str_dup("tools");
    route->num_exports = 2;

    CREATE(route->imported_goods, char *, 5);
    route->imported_goods[0] = str_dup("ore");
    route->imported_goods[1] = str_dup("cloth");
    route->num_imports = 2;

    log_string("TRADE: Route exports: grain, tools; imports: ore, cloth");
}

/*****************************************************************************
 * Trade Caravan Creation - Actual mobs
 *****************************************************************************/

TRADE_CARAVAN *spawn_caravan(TRADE_ROUTE *route)
{
    TRADE_CARAVAN *caravan;
    CHAR_DATA *merchant;
    ROOM_INDEX_DATA *start_room;
    int i;

    if (!route || !route->active)
        return NULL;

    CREATE(caravan, TRADE_CARAVAN, 1);
    caravan->route = route;
    caravan->origin = route->area_from;
    caravan->destination = route->area_to;
    caravan->outbound = TRUE;
    caravan->arrived = FALSE;
    caravan->attacked = FALSE;
    caravan->goods_lost = 0;
    caravan->departed = time(NULL);
    caravan->expected_arrival = caravan->departed + (route->travel_time * 3600);
    caravan->next = first_caravan;
    first_caravan = caravan;
    total_caravans++;

    /* Find starting room in origin area */
    start_room = route->area_from->first_room;
    if (!start_room)
        start_room = get_room_index(ROOM_VNUM_TEMPLE); /* Fallback */

    /* Create lead merchant NPC */
    /* This uses mob_creation_system.c */
    merchant = mob_create_npc(NULL, CREATE_WORKER, "trade caravan");
    if (merchant && start_room)
    {
        char_to_room(merchant, start_room);
        caravan->lead_merchant = merchant;
    }

    /* Create guards based on route safety */
    caravan->num_guards = (100 - route->safety) / 20; /* Less safe = more guards */
    if (caravan->num_guards > 0)
    {
        CREATE(caravan->guards, CHAR_DATA *, caravan->num_guards);
        for (i = 0; i < caravan->num_guards; i++)
        {
            CHAR_DATA *guard = mob_create_npc(merchant, CREATE_GUARD, "caravan protection");
            if (guard && start_room)
            {
                char_to_room(guard, start_room);
                caravan->guards[i] = guard;
            }
        }
    }

    /* Calculate cargo value */
    caravan->cargo_value = route->trade_volume_daily;

    route->num_active_caravans++;

    log_string("TRADE: Spawned caravan from %s to %s (%d guards, %d gold value)",
               route->area_from->name, route->area_to->name,
               caravan->num_guards, caravan->cargo_value);

    return caravan;
}

/*****************************************************************************
 * Caravan Movement
 *****************************************************************************/

void move_caravan(TRADE_CARAVAN *caravan)
{
    ROOM_INDEX_DATA *next_room;
    EXIT_DATA *exit;
    int dir;

    if (!caravan || !caravan->lead_merchant || !caravan->lead_merchant->in_room)
        return;

    if (caravan->arrived)
        return;

    /* Simple movement: pick random direction towards destination */
    /* In real implementation, would use pathfinding */
    for (dir = 0; dir < 6; dir++)
    {
        exit = caravan->lead_merchant->in_room->exit[dir];
        if (exit && exit->to_room)
        {
            next_room = exit->to_room;

            /* Move merchant */
            char_from_room(caravan->lead_merchant);
            char_to_room(caravan->lead_merchant, next_room);

            /* Move guards */
            if (caravan->guards)
            {
                int i;
                for (i = 0; i < caravan->num_guards; i++)
                {
                    if (caravan->guards[i])
                    {
                        char_from_room(caravan->guards[i]);
                        char_to_room(caravan->guards[i], next_room);
                    }
                }
            }

            caravan->current_location = next_room;

            /* Check if arrived */
            if (next_room->area == caravan->destination)
            {
                caravan_arrives(caravan);
            }

            break; /* Moved once */
        }
    }
}

void caravan_arrives(TRADE_CARAVAN *caravan)
{
    if (!caravan)
        return;

    caravan->arrived = TRUE;

    log_string("TRADE: Caravan arrived at %s from %s",
               caravan->destination->name, caravan->origin->name);

    /* Apply economic impact */
    if (!caravan->attacked)
    {
        /* Full cargo delivered */
        caravan->route->trade_volume_daily += caravan->cargo_value / 10;

        /* Update area economies */
        /* This would integrate with economy.c */

        log_string("TRADE: Delivered %d gold worth of goods", caravan->cargo_value);
    }
    else
    {
        /* Partial delivery */
        int delivered = caravan->cargo_value - caravan->goods_lost;
        log_string("TRADE: Partial delivery - %d lost to bandits", caravan->goods_lost);
    }

    /* Announce significant trade */
    if (caravan->cargo_value > 1000)
    {
        smart_announce(
            "Major Trade Caravan Arrives",
            "Economic prosperity flows between regions",
            EVENT_CATEGORY_ECONOMY,
            ANNOUNCE_PRIORITY_LOW,
            caravan->destination->name
        );
    }

    /* Record in history */
    record_world_event(EVENT_ECONOMIC, "Trade caravan completed journey", 5);
}

/*****************************************************************************
 * Automatic Trade Route Detection
 *****************************************************************************/

void detect_trade_opportunities(void)
{
    AREA_DATA *area1, *area2;
    AREA_CONTEXT *ctx1, *ctx2;

    /* Check pairs of areas for trade potential */
    for (area1 = first_area; area1; area1 = area1->next)
    {
        for (area2 = area1->next; area2; area2 = area2->next)
        {
            /* Check if close enough */
            int distance = calculate_area_distance(area1, area2);
            if (distance > 0 && distance < 100) /* Within 100 rooms */
            {
                ctx1 = analyze_area(area1);
                ctx2 = analyze_area(area2);

                if (ctx1 && ctx2)
                {
                    /* Check if they have complementary resources */
                    /* This is simplified - would do actual analysis */
                    if (number_percent() < 20)
                    {
                        create_trade_route(area1, area2);
                    }
                }
            }
        }
    }
}

/*****************************************************************************
 * Update Loop
 *****************************************************************************/

void global_trade_update(void)
{
    TRADE_ROUTE *route;
    TRADE_CARAVAN *caravan, *caravan_next;
    static time_t last_spawn = 0;
    time_t now = time(NULL);

    /* Move active caravans */
    for (caravan = first_caravan; caravan; caravan = caravan_next)
    {
        caravan_next = caravan->next;

        if (!caravan->arrived)
        {
            move_caravan(caravan);
        }
    }

    /* Spawn new caravans (every 2 hours) */
    if (difftime(now, last_spawn) >= 7200)
    {
        last_spawn = now;

        for (route = first_route; route; route = route->next)
        {
            if (route->active && route->num_active_caravans < 2)
            {
                if (number_percent() < 50) /* 50% chance */
                {
                    spawn_caravan(route);
                }
            }
        }
    }
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_trade(CHAR_DATA *ch, char *argument)
{
    TRADE_ROUTE *route;
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    if (IS_NPC(ch))
        return;

    send_to_char("&c=== Global Trade Routes ===&w\n\r\n\r", ch);

    sprintf(buf, "Total routes: %d\n\r", total_routes);
    send_to_char(buf, ch);
    sprintf(buf, "Active caravans: %d\n\r\n\r", total_caravans);
    send_to_char(buf, ch);

    send_to_char("&GActive Routes:&w\n\r", ch);

    for (route = first_route; route && count < 20; route = route->next)
    {
        if (route->active)
        {
            sprintf(buf, "%s <-> %s\n\r",
                    route->area_from->name,
                    route->area_to->name);
            send_to_char(buf, ch);

            sprintf(buf, "  Volume: %d gold/day, Distance: %d rooms, Safety: %d%%\n\r",
                    route->trade_volume_daily,
                    route->distance,
                    route->safety);
            send_to_char(buf, ch);

            sprintf(buf, "  Active caravans: %d\n\r\n\r",
                    route->num_active_caravans);
            send_to_char(buf, ch);

            count++;
        }
    }

    if (count == 0)
        send_to_char("No active trade routes.\n\r", ch);
}

void do_caravans(CHAR_DATA *ch, char *argument)
{
    TRADE_CARAVAN *caravan;
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    send_to_char("&c=== Active Caravans ===&w\n\r\n\r", ch);

    for (caravan = first_caravan; caravan && count < 20; caravan = caravan->next)
    {
        sprintf(buf, "&Y[%d]&w %s -> %s\n\r",
                count + 1,
                caravan->origin->name,
                caravan->destination->name);
        send_to_char(buf, ch);

        sprintf(buf, "  Cargo: %d gold, Guards: %d, Status: %s\n\r",
                caravan->cargo_value,
                caravan->num_guards,
                caravan->arrived ? "ARRIVED" : "IN TRANSIT");
        send_to_char(buf, ch);

        if (caravan->current_location)
        {
            sprintf(buf, "  Location: %s\n\r", caravan->current_location->name);
            send_to_char(buf, ch);
        }

        send_to_char("\n\r", ch);
        count++;
    }

    if (count == 0)
        send_to_char("No active caravans.\n\r", ch);
}

void do_createroute(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AREA_DATA *area1, *area2;
    TRADE_ROUTE *route;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Usage: createroute <area1_vnum> <area2_vnum>\n\r", ch);
        return;
    }

    area1 = find_area_by_vnum(atoi(arg1));
    area2 = find_area_by_vnum(atoi(arg2));

    if (!area1 || !area2)
    {
        send_to_char("Area not found.\n\r", ch);
        return;
    }

    route = create_trade_route(area1, area2);

    if (route)
        send_to_char("Trade route created!\n\r", ch);
    else
        send_to_char("Failed to create route.\n\r", ch);
}
