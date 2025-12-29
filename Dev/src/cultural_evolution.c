/*****************************************************************************
 * Cultural Evolution System
 *
 * Areas develop unique cultural identities based on:
 * - Geography (coastal vs mountain cultures)
 * - History (past events shape traditions)
 * - Neighbors (cultural exchange)
 * - Player interactions
 *
 * Traditions emerge, festivals are created, cultural drift occurs
 *
 * Integration: world_context.h, world_history_tracker.h
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "cultural_evolution.h"
#include "world_context.h"
#include "world_history_tracker.h"
#include "periodicos.h"
#include "ollama_integration.h"

/* Cultural identity for an area */
AREA_CULTURE *first_culture = NULL;
int total_cultures = 0;

/*****************************************************************************
 * Cultural Values
 *****************************************************************************/

#define VALUE_HONOR         0
#define VALUE_WEALTH        1
#define VALUE_KNOWLEDGE     2
#define VALUE_STRENGTH      3
#define VALUE_FAMILY        4
#define VALUE_FREEDOM       5
#define VALUE_TRADITION     6
#define VALUE_INNOVATION    7
#define VALUE_NATURE        8
#define VALUE_COMMUNITY     9

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_cultural_evolution(void)
{
    log_string("Initializing Cultural Evolution System...");
    first_culture = NULL;
    total_cultures = 0;
    log_string("Cultural Evolution System initialized.");
}

void load_cultures(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "cultures.dat");

    if ((fp = fopen(filename, "r")) == NULL)
    {
        log_string("No saved cultures to load.");
        return;
    }

    log_string("Loading cultures...");
    /* Load culture data */
    fclose(fp);
}

void save_cultures(void)
{
    FILE *fp;
    char filename[256];
    AREA_CULTURE *culture;
    int i;

    sprintf(filename, "%s%s", SYSTEM_DIR, "cultures.dat");

    if ((fp = fopen(filename, "w")) == NULL)
    {
        log_string("ERROR: Cannot save cultures!");
        return;
    }

    fprintf(fp, "#CULTURES\n");

    for (culture = first_culture; culture; culture = culture->next)
    {
        fprintf(fp, "AreaName~ %s~\n", culture->area->name);
        fprintf(fp, "CultureType %d\n", culture->primary_culture_type);
        fprintf(fp, "CulturalName~ %s~\n", culture->cultural_name);
        fprintf(fp, "Identity~ %s~\n", culture->cultural_identity);

        /* Save traditions */
        for (i = 0; i < culture->num_traditions; i++)
        {
            if (culture->traditions[i])
            {
                fprintf(fp, "Tradition~ %s~ %d\n",
                       culture->traditions[i]->name,
                       culture->traditions[i]->importance);
            }
        }

        fprintf(fp, "End\n\n");
    }

    fprintf(fp, "#END\n");
    fclose(fp);
}

/*****************************************************************************
 * Culture Creation
 *****************************************************************************/

AREA_CULTURE *create_area_culture(AREA_DATA *area)
{
    AREA_CULTURE *culture;
    AREA_CONTEXT *ctx;

    if (!area)
        return NULL;

    CREATE(culture, AREA_CULTURE, 1);
    culture->area = area;
    culture->num_traditions = 0;
    culture->num_festivals = 0;
    culture->last_evolution = time(NULL);
    culture->evolution_rate = 50; /* Medium rate */
    culture->next = first_culture;
    first_culture = culture;
    total_cultures++;

    /* Analyze area context to determine culture */
    ctx = analyze_area(area);
    if (ctx)
    {
        culture->primary_culture_type = ctx->cultural_theme;

        /* Generate cultural identity based on geography */
        if (area_is_coastal(area))
        {
            culture->cultural_name = str_dup("The Seafaring Folk");
            culture->greeting_style = str_dup("Sailors greet with 'Fair winds'");
            culture->architecture_style = str_dup("Wooden docks and stone harbors");
            culture->food_culture = str_dup("Fish and seafood are staples");
        }
        else if (area_is_mountainous(area))
        {
            culture->cultural_name = str_dup("The Mountain Clans");
            culture->greeting_style = str_dup("Mountain folk nod respectfully");
            culture->architecture_style = str_dup("Stone fortresses carved into mountains");
            culture->food_culture = str_dup("Hardy stews and preserved meats");
        }
        else
        {
            culture->cultural_name = str_dup("The Plains People");
            culture->greeting_style = str_dup("Common folk wave cheerfully");
            culture->architecture_style = str_dup("Timber and thatch homes");
            culture->food_culture = str_dup("Bread and grains are common");
        }

        /* Set initial values based on culture type */
        culture->values[VALUE_HONOR] = 70;
        culture->values[VALUE_FAMILY] = 80;
        culture->values[VALUE_COMMUNITY] = 75;
    }
    else
    {
        culture->cultural_name = str_dup("The Local Folk");
        culture->greeting_style = str_dup("Simple greetings");
        culture->architecture_style = str_dup("Common buildings");
        culture->food_culture = str_dup("Standard fare");
    }

    /* Generate AI description if available */
    if (ollama_is_available())
    {
        char prompt[MAX_STRING_LENGTH];
        sprintf(prompt,
            "Describe a unique medieval fantasy culture for an area called %s. "
            "The culture is %s. Geography: %s. "
            "Write 2-3 sentences describing their identity, values, and way of life.",
            area->name,
            culture->cultural_name,
            ctx ? describe_terrain(ctx) : "varied terrain");

        culture->cultural_identity = ollama_request(prompt, 200);
    }

    if (!culture->cultural_identity)
    {
        culture->cultural_identity = str_dup("A unique people with their own customs.");
    }

    sprintf(log_buf, "CULTURAL EVOLUTION: Created culture for %s: %s",
            area->name, culture->cultural_name);
    log_string(log_buf);

    return culture;
}

AREA_CULTURE *get_or_create_culture(AREA_DATA *area)
{
    AREA_CULTURE *culture;

    if (!area)
        return NULL;

    /* Find existing culture */
    for (culture = first_culture; culture; culture = culture->next)
    {
        if (culture->area == area)
            return culture;
    }

    /* Create new culture */
    return create_area_culture(area);
}

/*****************************************************************************
 * Tradition Creation
 *****************************************************************************/

void create_tradition(AREA_CULTURE *culture, char *name, char *description, int importance)
{
    int idx;

    if (!culture || culture->num_traditions >= 20)
        return;

    idx = culture->num_traditions;

    CREATE(culture->traditions[idx], struct tradition, 1);
    culture->traditions[idx]->name = str_dup(name);
    culture->traditions[idx]->description = str_dup(description);
    culture->traditions[idx]->age_in_years = 0;
    culture->traditions[idx]->importance = importance;

    culture->num_traditions++;

    sprintf(log_buf, "CULTURAL EVOLUTION: New tradition in %s: %s",
            culture->area->name, name);
    log_string(log_buf);

    /* Announce significant traditions */
    if (importance >= 7)
    {
        smart_announce(
            name,
            description,
            EVENT_CATEGORY_CULTURE,
            ANNOUNCE_PRIORITY_MEDIUM,
            culture->area->name
        );
    }

    /* Record in history */
    record_world_event(EVENT_CULTURAL, name, importance);
}

/*****************************************************************************
 * Festival Creation
 *****************************************************************************/

void create_festival(AREA_CULTURE *culture, char *name, char *description, int month, int day)
{
    int idx;

    if (!culture || culture->num_festivals >= 10)
        return;

    idx = culture->num_festivals;

    CREATE(culture->festivals[idx], struct festival, 1);
    culture->festivals[idx]->name = str_dup(name);
    culture->festivals[idx]->description = str_dup(description);
    culture->festivals[idx]->month = month;
    culture->festivals[idx]->day = day;
    culture->festivals[idx]->active = TRUE;

    culture->num_festivals++;

    sprintf(log_buf, "CULTURAL EVOLUTION: New festival in %s: %s",
            culture->area->name, name);
    log_string(log_buf);

    /* Announce festival creation */
    smart_announce(
        name,
        description,
        EVENT_CATEGORY_CULTURE,
        ANNOUNCE_PRIORITY_HIGH,
        culture->area->name
    );

    /* Record in history */
    record_world_event(EVENT_CULTURAL, name, 8);
}

/*****************************************************************************
 * Cultural Evolution - Changes over time
 *****************************************************************************/

void evolve_culture(AREA_CULTURE *culture)
{
    HISTORY_EVENT **events;
    int num_events;
    int i;

    if (!culture)
        return;

    sprintf(log_buf, "CULTURAL EVOLUTION: Evolving culture for %s", culture->area->name);
    log_string(log_buf);

    /* Check recent events in this area */
    events = get_events_by_type(EVENT_PLAYER_ACTION, &num_events);

    /* Events can spawn traditions */
    if (num_events > 50 && culture->num_traditions < 20)
    {
        /* Enough events to form a tradition */
        if (number_percent() < 30)
        {
            create_tradition(culture,
                           "Festival of Heroes",
                           "Celebrating the brave adventurers who defended our lands",
                           7);
        }
    }

    /* Age existing traditions */
    for (i = 0; i < culture->num_traditions; i++)
    {
        if (culture->traditions[i])
        {
            culture->traditions[i]->age_in_years++;

            /* Old traditions become more important */
            if (culture->traditions[i]->age_in_years > 50)
                culture->traditions[i]->importance = UMIN(10, culture->traditions[i]->importance + 1);
        }
    }

    /* Create seasonal festivals */
    if (culture->num_festivals == 0)
    {
        /* Create initial festival based on geography */
        if (area_is_coastal(culture->area))
        {
            create_festival(culture,
                          "The Sea Blessing",
                          "A celebration of the ocean's bounty",
                          3, 1); /* Spring */
        }
        else if (area_is_mountainous(culture->area))
        {
            create_festival(culture,
                          "The Stone Dance",
                          "Honoring the eternal mountains",
                          9, 15); /* Autumn */
        }
    }

    culture->last_evolution = time(NULL);
}

/*****************************************************************************
 * Cultural Drift - Influence from neighbors
 *****************************************************************************/

void calculate_cultural_drift(AREA_CULTURE *culture)
{
    AREA_CONTEXT *ctx;
    int i;
    int total_similarity = 0;
    int num_neighbors = 0;

    if (!culture || !culture->area)
        return;

    ctx = analyze_area(culture->area);
    if (!ctx)
        return;

    /* Check neighbor cultures */
    for (i = 0; i < ctx->num_neighbor_areas; i++)
    {
        if (ctx->neighbors[i].neighbor)
        {
            AREA_CULTURE *neighbor_culture = get_or_create_culture(ctx->neighbors[i].neighbor);
            if (neighbor_culture)
            {
                /* Calculate similarity */
                if (neighbor_culture->primary_culture_type == culture->primary_culture_type)
                    total_similarity += 50;

                /* Distance affects influence */
                if (ctx->neighbors[i].distance < 50)
                    total_similarity += 20;

                num_neighbors++;
            }
        }
    }

    if (num_neighbors > 0)
    {
        culture->similarity_to_neighbors = total_similarity / num_neighbors;
        culture->cultural_isolation = 100 - culture->similarity_to_neighbors;
    }
    else
    {
        culture->cultural_isolation = 100; /* Completely isolated */
    }

    sprintf(log_buf, "CULTURAL DRIFT: %s - Isolation: %d, Similarity: %d",
            culture->area->name,
            culture->cultural_isolation,
            culture->similarity_to_neighbors);
    log_string(log_buf);
}

/*****************************************************************************
 * Update Loop
 *****************************************************************************/

void cultural_evolution_update(void)
{
    AREA_CULTURE *culture;
    static time_t last_evolution = 0;
    time_t now = time(NULL);

    /* Evolve cultures every hour */
    if (difftime(now, last_evolution) < 3600)
        return;

    last_evolution = now;

    log_string("CULTURAL EVOLUTION: Monthly evolution update...");

    for (culture = first_culture; culture; culture = culture->next)
    {
        if (number_percent() < 50) /* 50% chance per month */
        {
            evolve_culture(culture);
        }

        calculate_cultural_drift(culture);
    }
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_culture(CHAR_DATA *ch, char *argument)
{
    AREA_DATA *area;
    AREA_CULTURE *culture;
    char buf[MAX_STRING_LENGTH];
    int i;

    if (IS_NPC(ch))
        return;

    area = ch->in_room ? ch->in_room->area : NULL;
    if (!area)
    {
        send_to_char("You are nowhere.\n\r", ch);
        return;
    }

    culture = get_or_create_culture(area);
    if (!culture)
    {
        send_to_char("This area has no cultural identity.\n\r", ch);
        return;
    }

    sprintf(buf, "&c=== %s ===&w\n\r\n\r", culture->cultural_name);
    send_to_char(buf, ch);

    sprintf(buf, "%s\n\r\n\r", culture->cultural_identity);
    send_to_char(buf, ch);

    send_to_char("&GGreetings:&w ", ch);
    sprintf(buf, "%s\n\r", culture->greeting_style);
    send_to_char(buf, ch);

    send_to_char("&GArchitecture:&w ", ch);
    sprintf(buf, "%s\n\r", culture->architecture_style);
    send_to_char(buf, ch);

    send_to_char("&GFood:&w ", ch);
    sprintf(buf, "%s\n\r\n\r", culture->food_culture);
    send_to_char(buf, ch);

    if (culture->num_traditions > 0)
    {
        send_to_char("&YTraditions:&w\n\r", ch);
        for (i = 0; i < culture->num_traditions; i++)
        {
            if (culture->traditions[i])
            {
                sprintf(buf, "  %s - %s\n\r",
                       culture->traditions[i]->name,
                       culture->traditions[i]->description);
                send_to_char(buf, ch);
            }
        }
        send_to_char("\n\r", ch);
    }

    if (culture->num_festivals > 0)
    {
        send_to_char("&BFestivals:&w\n\r", ch);
        for (i = 0; i < culture->num_festivals; i++)
        {
            if (culture->festivals[i])
            {
                sprintf(buf, "  %s - %s\n\r",
                       culture->festivals[i]->name,
                       culture->festivals[i]->description);
                send_to_char(buf, ch);
            }
        }
    }
}

void do_createtradition(CHAR_DATA *ch, char *argument)
{
    AREA_CULTURE *culture;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

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
        send_to_char("Usage: createtradition <name> <description>\n\r", ch);
        return;
    }

    culture = get_or_create_culture(ch->in_room->area);
    if (!culture)
    {
        send_to_char("Failed to get area culture.\n\r", ch);
        return;
    }

    create_tradition(culture, arg1, argument, 7);
    send_to_char("Tradition created!\n\r", ch);
}
