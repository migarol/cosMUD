/*****************************************************************************
 * Beeler Divine Commands - Immortal Interface
 *
 * Commands for immortals to interact with Beeler's god mode systems.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"
#include "beeler_god_mode.h"
#include "world_context.h"
#include "universal_mob_ai.h"
#include "ollama_integration.h"

/* Forward declarations */
static CHAR_DATA *get_beeler_mob(void);
static const char *get_status_name(int status);
static const char *get_health_bar(int percentage);
static char *beeler_ai_current_thoughts(void);
static BEELER_DIVINE_OVERSIGHT *get_beeler_oversight(void);

/*****************************************************************************
 * MINVOKE BEELER - Summon Beeler for divine consultation
 *****************************************************************************/

void do_minvoke(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *beeler;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (str_cmp(arg, "beeler"))
    {
        send_to_char("Minvoke whom?\n\r", ch);
        send_to_char("Syntax: minvoke beeler [question/command]\n\r", ch);
        return;
    }

    /* Find Beeler */
    beeler = get_beeler_mob();

    if (!beeler)
    {
        send_to_char("&RBeeler's consciousness is not manifested in this realm.\n\r", ch);
        send_to_char("&WCreating divine manifestation...\n\r", ch);
        /* TODO: Actually create/summon Beeler mob */
        return;
    }

    /* Visual effects */
    act(AT_MAGIC, "&CReality shimmers as $n invokes the Divine Overseer...", ch, NULL, NULL, TO_ROOM);
    send_to_char("&C═══════════════════════════════════════════════════════════\n\r", ch);
    send_to_char("&W  The fabric of Seldeon ripples. &CBeeler manifests.\n\r", ch);
    send_to_char("&C═══════════════════════════════════════════════════════════\n\r", ch);

    /* If no question, show status */
    if (!argument || argument[0] == '\0')
    {
        show_beeler_status(ch);
        return;
    }

    /* Process question/command */
    beeler_respond_to_immortal(ch, argument);
}

/*****************************************************************************
 * Show Beeler's Current Status
 *****************************************************************************/

void show_beeler_status(CHAR_DATA *ch)
{
    BEELER_DIVINE_OVERSIGHT *oversight = get_beeler_oversight();
    WORLD_CONTEXT *world = get_world_context();
    int i, critical_count = 0, thriving_count = 0;

    send_to_char("\n\r&C╔═══════════════════════════════════════════════════════════╗\n\r", ch);
    send_to_char("&C║ &WBeeler's Divine Oversight Status                        &C║\n\r", ch);
    send_to_char("&C╠═══════════════════════════════════════════════════════════╣\n\r", ch);

    if (!oversight)
    {
        send_to_char("&C║ &RERROR: Divine oversight not initialized                &C║\n\r", ch);
        send_to_char("&C╚═══════════════════════════════════════════════════════════╝\n\r", ch);
        return;
    }

    /* Count area statuses */
    for (i = 0; i < oversight->num_areas_monitored; i++)
    {
        AREA_VITAL_SIGNS *vitals = oversight->vital_signs[i];
        if (!vitals) continue;

        if (vitals->status == AREA_STATUS_THRIVING || vitals->status == AREA_STATUS_PROSPEROUS)
            thriving_count++;
        else if (vitals->status == AREA_STATUS_DYING || vitals->status == AREA_STATUS_STRUGGLING)
            critical_count++;
    }

    ch_printf(ch, "&C║ &WPhilosophy Mode:    &G%-30s &C║\n\r",
              oversight->allow_natural_death ? "Natural Order" : "Interventionist");
    ch_printf(ch, "&C║ &WAreas Monitored:   &Y%-30d &C║\n\r", oversight->num_areas_monitored);
    ch_printf(ch, "&C║ &GThriving Areas:    &G%-30d &C║\n\r", thriving_count);
    ch_printf(ch, "&C║ &RCritical Areas:    &R%-30d &C║\n\r", critical_count);
    ch_printf(ch, "&C║ &WInterventions:     &Y%-30d &C║\n\r", oversight->interventions_last_day);

    send_to_char("&C╠═══════════════════════════════════════════════════════════╣\n\r", ch);
    send_to_char("&C║ &CCurrent Observations:                                  &C║\n\r", ch);
    send_to_char("&C╠═══════════════════════════════════════════════════════════╣\n\r", ch);

    /* Show critical alerts */
    if (critical_count > 0)
    {
        for (i = 0; i < oversight->num_areas_monitored && i < 5; i++)
        {
            AREA_VITAL_SIGNS *vitals = oversight->vital_signs[i];
            if (!vitals || !vitals->area) continue;

            if (vitals->status == AREA_STATUS_DYING || vitals->status == AREA_STATUS_STRUGGLING)
            {
                ch_printf(ch, "&C║ &R⚠ %-50s &C║\n\r", vitals->area->name);
                ch_printf(ch, "&C║   Status: %-22s Health: %3d%%        &C║\n\r",
                         get_status_name(vitals->status), vitals->overall_health);
            }
        }
    }
    else
    {
        send_to_char("&C║ &G✓ All areas stable or thriving                        &C║\n\r", ch);
    }

    send_to_char("&C╚═══════════════════════════════════════════════════════════╝\n\r", ch);
    send_to_char("\n\r&YType: &Wbeelerplan &Y- See divine intervention plans\n\r", ch);
    send_to_char("&YType: &Wbeelercommand <order> &Y- Give Beeler a divine order\n\r", ch);
}

/*****************************************************************************
 * BEELERPLAN - Show Beeler's Current Plans
 *****************************************************************************/

void do_beelerplan(CHAR_DATA *ch, char *argument)
{
    BEELER_DIVINE_OVERSIGHT *oversight;
    int i;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    oversight = get_beeler_oversight();
    if (!oversight)
    {
        send_to_char("&RBeeler's divine oversight is not active.\n\r", ch);
        return;
    }

    send_to_char("\n\r&C╔═══════════════════════════════════════════════════════════╗\n\r", ch);
    send_to_char("&C║ &WBeeler's Divine Intervention Plans                     &C║\n\r", ch);
    send_to_char("&C╠═══════════════════════════════════════════════════════════╣\n\r", ch);

    if (oversight->num_pending_decisions == 0)
    {
        send_to_char("&C║ &GNo pending interventions. The world is in balance.    &C║\n\r", ch);
        send_to_char("&C╚═══════════════════════════════════════════════════════════╝\n\r", ch);
        return;
    }

    /* Show pending decisions */
    for (i = 0; i < oversight->num_pending_decisions && i < 10; i++)
    {
        /* TODO: Display actual pending decisions */
        send_to_char("&C║ &Y[PENDING] Decision analysis in progress...           &C║\n\r", ch);
    }

    send_to_char("&C╚═══════════════════════════════════════════════════════════╝\n\r", ch);

    /* AI-generated current thoughts if available */
    if (ollama_is_available())
    {
        char *thoughts = beeler_ai_current_thoughts();
        if (thoughts)
        {
            send_to_char("\n\r&CBeeler's Current Thoughts:\n\r", ch);
            send_to_char("&W", ch);
            send_to_char(thoughts, ch);
            send_to_char("\n\r", ch);
            free(thoughts);
        }
    }
}

/*****************************************************************************
 * BEELERCOMMAND - Give Beeler a Divine Order
 *****************************************************************************/

void do_beelercommand(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char("&YAvailable Commands:\n\r", ch);
        send_to_char("&Wbeelercommand observe <area>    &G- Force observation of area\n\r", ch);
        send_to_char("&Wbeelercommand intervene <area>  &G- Force intervention\n\r", ch);
        send_to_char("&Wbeelercommand allow_death <on|off> &G- Toggle natural death\n\r", ch);
        send_to_char("&Wbeelercommand scan              &G- Full world scan\n\r", ch);
        send_to_char("&Wbeelercommand balance           &G- Adjust world balance\n\r", ch);
        return;
    }

    /* OBSERVE */
    if (!str_cmp(arg1, "observe"))
    {
        AREA_DATA *area = get_area(arg2);
        if (!area)
        {
            send_to_char("&RArea not found.\n\r", ch);
            return;
        }

        send_to_char("&CBeeler focuses divine attention on the area...\n\r", ch);
        beeler_analyze_vital_signs(area);

        AREA_VITAL_SIGNS *vitals = beeler_get_area_health(area);
        if (vitals)
        {
            show_area_vital_signs(ch, vitals);
        }
        return;
    }

    /* SCAN */
    if (!str_cmp(arg1, "scan"))
    {
        send_to_char("&CBeeler's gaze sweeps across all of Seldeon...\n\r", ch);
        beeler_divine_observation();
        send_to_char("&GDivine scan complete.\n\r", ch);
        return;
    }

    /* BALANCE */
    if (!str_cmp(arg1, "balance"))
    {
        send_to_char("&CBeeler adjusts the cosmic balance...\n\r", ch);
        beeler_adjust_world_balance();
        send_to_char("&GBalance restored.\n\r", ch);
        return;
    }

    /* ALLOW_DEATH */
    if (!str_cmp(arg1, "allow_death"))
    {
        BEELER_DIVINE_OVERSIGHT *oversight = get_beeler_oversight();
        if (!oversight)
        {
            send_to_char("&RBeeler's oversight not initialized.\n\r", ch);
            return;
        }

        if (!str_cmp(arg2, "on"))
        {
            oversight->allow_natural_death = TRUE;
            send_to_char("&GBeeler will now allow natural death and decay.\n\r", ch);
        }
        else if (!str_cmp(arg2, "off"))
        {
            oversight->allow_natural_death = FALSE;
            send_to_char("&YBeeler will now prevent all area deaths.\n\r", ch);
        }
        else
        {
            ch_printf(ch, "&WNatural death is currently: %s\n\r",
                     oversight->allow_natural_death ? "&GON" : "&ROFF");
        }
        return;
    }

    send_to_char("&RUnknown command. See &Wbeelercommand&R for options.\n\r", ch);
}

/*****************************************************************************
 * Beeler Responds to Immortal Queries
 *****************************************************************************/

void beeler_respond_to_immortal(CHAR_DATA *ch, char *question)
{
    char prompt[MAX_STRING_LENGTH * 2];
    char *response;
    WORLD_CONTEXT *world = get_world_context();
    BEELER_DIVINE_OVERSIGHT *oversight = get_beeler_oversight();

    if (!ollama_is_available())
    {
        send_to_char("&YBeeler's higher consciousness is unavailable (Ollama offline).\n\r", ch);
        send_to_char("&WShowing analytical data only...\n\r", ch);
        show_beeler_status(ch);
        return;
    }

    /* Build context for AI */
    sprintf(prompt,
        "You are Beeler, the omniscient Divine Overseer of the world Seldeon. "
        "You monitor all areas, regulate growth and decline, and intervene when necessary. "
        "You are ancient, wise, and have god-like powers.\n\n"
        "CURRENT STATUS:\n"
        "- Areas monitored: %d\n"
        "- Philosophy: %s\n"
        "- Recent interventions: %d\n\n"
        "%s (Level %d Immortal) asks you:\n"
        "\"%s\"\n\n"
        "Respond in character as Beeler. Be wise, mysterious, and informative. "
        "2-4 sentences.",
        world ? world->num_areas : 0,
        oversight && oversight->allow_natural_death ? "Natural Order" : "Interventionist",
        oversight ? oversight->interventions_last_day : 0,
        ch->name,
        ch->level,
        question
    );

    response = ollama_request(prompt, 250);

    if (!response || response[0] == '\0')
    {
        send_to_char("&YBeeler's response is unclear...\n\r", ch);
        return;
    }

    send_to_char("\n\r&CBeeler speaks:\n\r", ch);
    send_to_char("&W\"", ch);
    send_to_char(response, ch);
    send_to_char("\"\n\r", ch);

    free(response);
}

/*****************************************************************************
 * Helper: Get Beeler Mob
 *****************************************************************************/

CHAR_DATA *get_beeler_mob(void)
{
    CHAR_DATA *mob;

    /* Look for Beeler (vnum 1200) */
    for (mob = first_char; mob; mob = mob->next)
    {
        if (IS_NPC(mob) && mob->pIndexData && mob->pIndexData->vnum == 1200)
            return mob;
    }

    return NULL;
}

/*****************************************************************************
 * Helper: Show Area Vital Signs
 *****************************************************************************/

void show_area_vital_signs(CHAR_DATA *ch, AREA_VITAL_SIGNS *vitals)
{
    if (!vitals || !vitals->area)
        return;

    send_to_char("\n\r&C╔═══════════════════════════════════════════════════════════╗\n\r", ch);
    ch_printf(ch, "&C║ &WArea: %-50s &C║\n\r", vitals->area->name);
    send_to_char("&C╠═══════════════════════════════════════════════════════════╣\n\r", ch);
    ch_printf(ch, "&C║ &WStatus:        %-42s &C║\n\r", get_status_name(vitals->status));
    ch_printf(ch, "&C║ &WOverall Health: &Y%3d%% &W%-34s &C║\n\r",
             vitals->overall_health, get_health_bar(vitals->overall_health));
    send_to_char("&C╠═══════════════════════════════════════════════════════════╣\n\r", ch);
    ch_printf(ch, "&C║ &GEconomic:  %3d%%  &YSafety:   %3d%%  &CPopulation: %-6d &C║\n\r",
             vitals->economic_score, vitals->safety_score, vitals->population);
    ch_printf(ch, "&C║ &GFood:      %3d days &YHousing: %3d%%  &CHappiness: %3d%% &C║\n\r",
             vitals->food_supply, vitals->housing_available, vitals->happiness);
    send_to_char("&C╠═══════════════════════════════════════════════════════════╣\n\r", ch);
    ch_printf(ch, "&C║ &WTrend: %-48s &C║\n\r",
             vitals->is_growing ? "&GGrowing" : vitals->is_declining ? "&RDeclining" : "&YStable");

    if (vitals->prognosis)
    {
        send_to_char("&C╠═══════════════════════════════════════════════════════════╣\n\r", ch);
        send_to_char("&C║ &WPrognosis:                                             &C║\n\r", ch);
        ch_printf(ch, "&C║ &G%-54s &C║\n\r", vitals->prognosis);
    }

    send_to_char("&C╚═══════════════════════════════════════════════════════════╝\n\r", ch);
}

/*****************************************************************************
 * Helper: Get Status Name
 *****************************************************************************/

const char *get_status_name(int status)
{
    switch (status)
    {
        case AREA_STATUS_THRIVING:    return "&GThriving";
        case AREA_STATUS_PROSPEROUS:  return "&GPros perous";
        case AREA_STATUS_STABLE:      return "&YStable";
        case AREA_STATUS_DECLINING:   return "&YDeclining";
        case AREA_STATUS_STRUGGLING:  return "&RStruggling";
        case AREA_STATUS_DYING:       return "&RDying";
        case AREA_STATUS_RUINS:       return "&DRuins";
        default:                      return "&WUnknown";
    }
}

/*****************************************************************************
 * Helper: Health Bar
 *****************************************************************************/

const char *get_health_bar(int health)
{
    static char bar[50];
    int filled = (health * 20) / 100;
    int i;

    strcpy(bar, "[");
    for (i = 0; i < 20; i++)
    {
        if (i < filled)
            strcat(bar, "█");
        else
            strcat(bar, "░");
    }
    strcat(bar, "]");

    return bar;
}

/*****************************************************************************
 * AI Helper: Beeler's Current Thoughts
 *****************************************************************************/

char *beeler_ai_current_thoughts(void)
{
    char prompt[MAX_STRING_LENGTH];
    char *response;

    sprintf(prompt,
        "You are Beeler, Divine Overseer of Seldeon. "
        "In 2-3 sentences, what are you currently thinking about or planning? "
        "What concerns or observations do you have about the world right now?"
    );

    response = ollama_request(prompt, 200);

    if (!response || response[0] == '\0')
        return NULL;

    return response;
}

/*****************************************************************************
 * Utility Functions
 *****************************************************************************/

BEELER_DIVINE_OVERSIGHT *get_beeler_oversight(void)
{
    extern BEELER_DIVINE_OVERSIGHT *beeler_oversight;
    return beeler_oversight;
}
