/*****************************************************************************
 * Beeler God Mode - Divine World Oversight
 *
 * Beeler observes ALL areas, monitors vital signs, makes intelligent decisions.
 * Regulates growth AND decline. Intervenes when necessary. Allows natural death.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"
#include "beeler_god_mode.h"
#include "world_context.h"
#include "ollama_integration.h"

/* Global divine oversight */
BEELER_DIVINE_OVERSIGHT *beeler_oversight = NULL;

/* Forward declarations for internal functions */
static char *ai_generate_prognosis(AREA_VITAL_SIGNS *vitals);
static int calculate_economic_score(AREA_DATA *area);
static int calculate_population_trend(AREA_DATA *area);
static int calculate_safety_score(AREA_DATA *area);
static int count_guards_in_area(AREA_DATA *area);
static int count_aggressive_mobs(AREA_DATA *area);
static int calculate_housing_availability(AREA_DATA *area);
static int calculate_happiness(AREA_VITAL_SIGNS *vitals);
static int determine_area_status(int overall_health);

/* Vital signs for all areas */
#define MAX_VITAL_SIGNS 100
AREA_VITAL_SIGNS *vital_signs_list[MAX_VITAL_SIGNS];
int num_vital_signs = 0;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_beeler_god_mode(void)
{
    int i;

    log_string("BEELER GOD MODE: Initializing divine oversight...");

    /* Allocate oversight */
    beeler_oversight = (BEELER_DIVINE_OVERSIGHT *)calloc(1, sizeof(BEELER_DIVINE_OVERSIGHT));
    if (!beeler_oversight)
    {
        bug("init_beeler_god_mode: Failed to allocate oversight");
        return;
    }

    /* Initialize vital signs list */
    for (i = 0; i < MAX_VITAL_SIGNS; i++)
        vital_signs_list[i] = NULL;

    num_vital_signs = 0;

    /* Set philosophy parameters */
    beeler_oversight->intervention_threshold = 30; /* Intervene if health < 30 */
    beeler_oversight->allow_natural_death = TRUE;  /* Let areas die naturally */
    beeler_oversight->prevent_all_wars = FALSE;    /* Allow conflict */

    log_string("BEELER GOD MODE: Divine oversight established");
}

/*****************************************************************************
 * Divine Observation - Monitor ALL Areas
 *****************************************************************************/

void beeler_divine_observation(void)
{
    AREA_DATA *area;
    int count = 0;

    if (!beeler_oversight)
        return;

    /* Observe all areas */
    for (area = first_area; area; area = area->next)
    {
        beeler_analyze_vital_signs(area);
        count++;
    }

    {
        char log_buf[256];
    sprintf(log_buf, "BEELER: Observed %d areas", count);
    log_string(log_buf);
    }
}

void beeler_analyze_vital_signs(AREA_DATA *area)
{
    AREA_VITAL_SIGNS *vitals;
    int i;

    if (!area)
        return;

    /* Find existing vital signs */
    vitals = beeler_get_area_health(area);

    if (!vitals)
    {
        /* Create new vital signs */
        vitals = (AREA_VITAL_SIGNS *)calloc(1, sizeof(AREA_VITAL_SIGNS));
        if (!vitals)
        {
            bug("beeler_analyze_vital_signs: Failed to allocate");
            return;
        }

        vitals->area = area;

        /* Add to list */
        if (num_vital_signs < MAX_VITAL_SIGNS)
        {
            vital_signs_list[num_vital_signs] = vitals;
            num_vital_signs++;
        }
    }

    /* ECONOMIC HEALTH */
    vitals->trade_volume = area->high_economy; /* Use area's economy field */
    vitals->economic_score = calculate_economic_score(area);
    vitals->resource_availability = 50; /* Default - TODO: Calculate from actual resources */

    /* POPULATION */
    vitals->population = count_npcs_in_area(area);
    vitals->population_trend = calculate_population_trend(area);
    vitals->births_per_month = (vitals->population > 20) ? 2 : 0;
    vitals->deaths_per_month = 1; /* Base mortality */
    vitals->immigration = (vitals->economic_score > 60) ? 2 : 0;
    vitals->emigration = (vitals->economic_score < 40) ? 3 : 0;

    /* SECURITY */
    vitals->safety_score = calculate_safety_score(area);
    vitals->num_guards = count_guards_in_area(area);
    vitals->crime_rate = 0; /* TODO: Track player crimes */
    vitals->monster_attacks = count_aggressive_mobs(area);
    vitals->at_war = FALSE; /* TODO: War system */

    /* RESOURCES */
    vitals->food_supply = 30; /* Default 30 days - TODO: Calculate */
    vitals->water_access = detect_water_access(area);
    vitals->housing_available = calculate_housing_availability(area);
    vitals->infrastructure_quality = 50; /* Default - TODO: Calculate */

    /* CULTURAL */
    vitals->happiness = calculate_happiness(vitals);
    vitals->cultural_activity = 50; /* Default */

    /* ENVIRONMENTAL */
    vitals->natural_disasters = 0; /* No system yet */
    vitals->disease_outbreaks = 0; /* No system yet */
    vitals->crop_yields = 100; /* 100% normal */

    /* OVERALL HEALTH */
    vitals->overall_health = beeler_calculate_area_status(vitals);
    vitals->status = determine_area_status(vitals->overall_health);

    /* Determine if growing or declining */
    vitals->is_growing = (vitals->economic_score > 60 && vitals->population_trend > 0);
    vitals->is_declining = (vitals->economic_score < 40 && vitals->population_trend < 0);
    vitals->is_stable = !vitals->is_growing && !vitals->is_declining;

    /* AI prognosis if available */
    if (ollama_is_available() && (vitals->is_growing || vitals->is_declining))
    {
        vitals->prognosis = ai_generate_prognosis(vitals);
    }
}

/*****************************************************************************
 * Vital Signs Calculations
 *****************************************************************************/

int calculate_economic_score(AREA_DATA *area)
{
    int score = 50; /* Base */

    if (!area)
        return score;

    /* Use area's economic fields */
    if (area->high_economy > 1000)
        score = 85;
    else if (area->high_economy > 500)
        score = 70;
    else if (area->high_economy > 250)
        score = 55;
    else if (area->low_economy < 100)
        score = 30;
    else if (area->low_economy < 50)
        score = 15;

    return score;
}

int calculate_population_trend(AREA_DATA *area)
{
    /* Simple trend for now - compare current to baseline */
    /* TODO: Track historical population */
    return 0; /* Neutral */
}

int calculate_safety_score(AREA_DATA *area)
{
    int score = 70; /* Default moderately safe */
    int aggressive_count;

    if (!area)
        return score;

    aggressive_count = count_aggressive_mobs(area);

    /* Reduce safety based on aggressive mobs */
    if (aggressive_count > 20)
        score = 30;
    else if (aggressive_count > 10)
        score = 50;
    else if (aggressive_count < 3)
        score = 90;

    return score;
}

int count_guards_in_area(AREA_DATA *area)
{
    ROOM_INDEX_DATA *room;
    CHAR_DATA *mob;
    int vnum, count = 0;

    if (!area)
        return 0;

    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);
        if (room)
        {
            for (mob = room->first_person; mob; mob = mob->next_in_room)
            {
                if (IS_NPC(mob) && xIS_SET(mob->act, ACT_SENTINEL))
                {
                    /* Simple heuristic: Sentinels are often guards */
                    count++;
                }
            }
        }
    }

    return count;
}

int count_aggressive_mobs(AREA_DATA *area)
{
    ROOM_INDEX_DATA *room;
    CHAR_DATA *mob;
    int vnum, count = 0;

    if (!area)
        return 0;

    for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++)
    {
        room = get_room_index(vnum);
        if (room)
        {
            for (mob = room->first_person; mob; mob = mob->next_in_room)
            {
                if (IS_NPC(mob) && xIS_SET(mob->act, ACT_AGGRESSIVE))
                    count++;
            }
        }
    }

    return count;
}

int calculate_housing_availability(AREA_DATA *area)
{
    /* Simple calculation for now */
    /* Percentage of population with homes */
    /* TODO: Actually track homes */
    return 60; /* Default 60% housed */
}

int calculate_happiness(AREA_VITAL_SIGNS *vitals)
{
    int happiness = 50; /* Neutral */

    if (!vitals)
        return happiness;

    /* Happiness based on multiple factors */
    if (vitals->economic_score > 70)
        happiness += 20;
    if (vitals->safety_score > 70)
        happiness += 15;
    if (vitals->food_supply > 20)
        happiness += 10;
    else if (vitals->food_supply < 10)
        happiness -= 30;

    /* Clamp */
    if (happiness > 100) happiness = 100;
    if (happiness < 0) happiness = 0;

    return happiness;
}

int beeler_calculate_area_status(AREA_VITAL_SIGNS *vitals)
{
    int health = 0;

    if (!vitals)
        return 0;

    /* Weighted average of all factors */
    health += vitals->economic_score * 3;       /* Weight 3 */
    health += vitals->safety_score * 2;         /* Weight 2 */
    health += (vitals->food_supply > 10 ? 100 : 0) * 2; /* Weight 2 */
    health += vitals->happiness * 1;            /* Weight 1 */

    health /= 8; /* Total weight */

    return health;
}

int determine_area_status(int overall_health)
{
    if (overall_health >= 80)
        return AREA_STATUS_THRIVING;
    else if (overall_health >= 60)
        return AREA_STATUS_PROSPEROUS;
    else if (overall_health >= 40)
        return AREA_STATUS_STABLE;
    else if (overall_health >= 25)
        return AREA_STATUS_DECLINING;
    else if (overall_health >= 10)
        return AREA_STATUS_STRUGGLING;
    else if (overall_health > 0)
        return AREA_STATUS_DYING;
    else
        return AREA_STATUS_RUINS;
}

/*****************************************************************************
 * Decision Making - Should Beeler Intervene?
 *****************************************************************************/

bool beeler_should_intervene(AREA_VITAL_SIGNS *vitals)
{
    if (!vitals || !beeler_oversight)
        return FALSE;

    /* Critical situations always intervene */
    if (vitals->food_supply < 5)
        return TRUE; /* Starvation */

    if (vitals->safety_score < 20)
        return TRUE; /* Extreme danger */

    /* Check against threshold */
    if (vitals->overall_health < beeler_oversight->intervention_threshold)
    {
        /* But respect natural death philosophy */
        if (beeler_oversight->allow_natural_death && vitals->status == AREA_STATUS_DYING)
        {
            /* Let it die naturally unless critical */
            return (vitals->overall_health < 10);
        }

        return TRUE;
    }

    return FALSE;
}

void beeler_make_divine_decisions(void)
{
    int i;
    AREA_VITAL_SIGNS *vitals;

    if (!beeler_oversight)
        return;

    for (i = 0; i < num_vital_signs; i++)
    {
        vitals = vital_signs_list[i];
        if (!vitals)
            continue;

        /* Check if intervention needed */
        if (beeler_should_intervene(vitals))
        {
            beeler_handle_critical_alert(vitals);
        }
    }
}

void beeler_handle_critical_alert(AREA_VITAL_SIGNS *vitals)
{
    char alert[MAX_STRING_LENGTH];

    if (!vitals || !vitals->area)
        return;

    /* Determine alert type */
    if (vitals->food_supply < 5)
    {
        sprintf(alert, "CRITICAL: %s is starving (food supply: %d days)",
                vitals->area->name, vitals->food_supply);
        log_string(alert);

        /* Emergency intervention - create food */
        beeler_emergency_intervention("food shortage");
    }
    else if (vitals->safety_score < 20)
    {
        sprintf(alert, "CRITICAL: %s under severe threat (safety: %d%%)",
                vitals->area->name, vitals->safety_score);
        log_string(alert);

        /* Emergency intervention - create guards */
        beeler_emergency_intervention("security crisis");
    }
    else if (vitals->status == AREA_STATUS_DYING)
    {
        sprintf(alert, "WARNING: %s is dying (health: %d%%)",
                vitals->area->name, vitals->overall_health);
        log_string(alert);

        /* Decide: intervene or allow natural death */
        if (beeler_oversight->allow_natural_death)
        {
            log_string("BEELER: Allowing natural death");
        }
        else
        {
            beeler_emergency_intervention("area collapse");
        }
    }
}

void beeler_emergency_intervention(char *crisis)
{
    {
        char log_buf[256];
    sprintf(log_buf, "BEELER: Emergency intervention for %s", crisis);
    log_string(log_buf);
    }

    /* TODO: Actual intervention logic */
    /* For now just log */
}

/*****************************************************************************
 * Natural Processes
 *****************************************************************************/

void beeler_allow_natural_death(AREA_DATA *area)
{
    if (!area)
        return;

    {
        char log_buf[256];
    sprintf(log_buf, "BEELER: Allowing natural death of %s", area->name);
    log_string(log_buf);
    }

    /* TODO: Gradual conversion to ruins */
}

void beeler_convert_to_ruins(AREA_DATA *area)
{
    if (!area)
        return;

    {
        char log_buf[256];
    sprintf(log_buf, "BEELER: Converting %s to ruins", area->name);
    log_string(log_buf);
    }

    /* TODO: Actual conversion */
    /* - Remove living NPCs */
    /* - Add ghost NPCs */
    /* - Decay buildings */
    /* - Add treasure */
}

/*****************************************************************************
 * Utility Functions
 *****************************************************************************/

AREA_VITAL_SIGNS *beeler_get_area_health(AREA_DATA *area)
{
    int i;

    if (!area)
        return NULL;

    for (i = 0; i < num_vital_signs; i++)
    {
        if (vital_signs_list[i] && vital_signs_list[i]->area == area)
            return vital_signs_list[i];
    }

    return NULL;
}

void beeler_adjust_world_balance(void)
{
    /* TODO: Global balance regulation */
    log_string("BEELER: Adjusting world balance");
}

void beeler_prevent_runaway_growth(void)
{
    /* TODO: Prevent explosive growth */
}

void beeler_prevent_total_collapse(void)
{
    /* TODO: Prevent world-wide collapse */
}

/*****************************************************************************
 * AI Integration
 *****************************************************************************/

static char *ai_generate_prognosis(AREA_VITAL_SIGNS *vitals)
{
    char prompt[MAX_STRING_LENGTH];
    char *response;

    if (!vitals || !vitals->area)
        return strdup("Unknown prognosis");

    sprintf(prompt,
        "Predict the future of this area:\n\n"
        "AREA: %s\n"
        "Economic score: %d/100\n"
        "Safety score: %d/100\n"
        "Population: %d (trend: %d)\n"
        "Food supply: %d days\n"
        "Status: %s\n\n"
        "Will this area thrive, decline, or stabilize? Why? (2-3 sentences)",
        vitals->area->name,
        vitals->economic_score,
        vitals->safety_score,
        vitals->population,
        vitals->population_trend,
        vitals->food_supply,
        vitals->is_growing ? "GROWING" : vitals->is_declining ? "DECLINING" : "STABLE"
    );

    response = ollama_request(prompt, 200);

    if (!response || response[0] == '\0')
        return strdup("Prognosis unavailable");

    return response;
}
