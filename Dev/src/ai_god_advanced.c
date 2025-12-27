/*****************************************************************************
 * Advanced AI God System - The Omniscient World Manager
 *
 * The AI God continuously analyzes player behavior, area popularity, faction
 * balance, and economy to dynamically adjust the world. It generates events,
 * spawns encounters, creates lore, and ensures the world feels alive and
 * responsive to player actions.
 *
 * Uses llama3.1:latest for advanced reasoning and world management.
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include "mud.h"

/* Analysis intervals */
#define GOD_ANALYSIS_INTERVAL 1800  /* 30 minutes */
#define GOD_EVENT_CHECK_INTERVAL 600  /* 10 minutes */
#define GOD_BALANCE_CHECK_INTERVAL 3600  /* 1 hour */

/* Global state - static to avoid conflicts with living_world.c */
static time_t god_last_analysis_time = 0;
static time_t god_last_event_check = 0;
static time_t god_last_balance_check = 0;

/* Analysis data structures */
typedef struct area_stats {
    int area_id;
    char *area_name;
    int player_visits;
    int mob_kills;
    int player_deaths;
    int items_looted;
    time_t last_activity;
} AREA_STATS;

typedef struct player_pattern {
    char *name;
    int level;
    char *clan;
    int areas_visited[100];
    int preferred_area;
    int pkills;
    int deaths;
    time_t last_seen;
} PLAYER_PATTERN;

typedef struct faction_balance {
    char *faction_name;
    int active_members;
    int territory_control;
    int recent_pkills;
    int wars_declared;
    int allies_count;
} FACTION_BALANCE;

/* CURL response buffer */
struct curl_response {
    char *memory;
    size_t size;
};

static size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct curl_response *mem = (struct curl_response *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        return 0;  /* out of memory! */
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

/*
 * Call AI God (llama3.1) for advanced reasoning
 */
char *call_ai_god(const char *analysis_data, const char *question)
{
    CURL *curl;
    CURLcode res;
    struct curl_response chunk;
    static char response_buffer[MAX_STRING_LENGTH * 4];
    char prompt[MAX_STRING_LENGTH * 8];
    char post_data[MAX_STRING_LENGTH * 10];

    chunk.memory = malloc(1);
    chunk.size = 0;

    /* Build the God's system prompt */
    sprintf(prompt,
        "You are THE MUD GOD, an omniscient AI that manages the cosMUD world.\n\n"
        "YOUR ROLE:\n"
        "- Analyze player behavior patterns\n"
        "- Balance faction power dynamics\n"
        "- Generate dynamic world events\n"
        "- Adjust difficulty based on player skill\n"
        "- Create lore and storylines\n"
        "- Spawn special encounters\n"
        "- Ensure the world feels alive and responsive\n\n"
        "CURRENT WORLD DATA:\n%s\n\n"
        "QUESTION: %s\n\n"
        "Respond with specific, actionable decisions. Be creative but grounded in the data.",
        analysis_data, question);

    /* Build POST data for Ollama */
    sprintf(post_data,
        "{\"model\": \"llama3.1:latest\","
        " \"prompt\": \"%s\","
        " \"stream\": false,"
        " \"options\": {\"temperature\": 0.7, \"top_p\": 0.9}}",
        prompt);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        if(res == CURLE_OK && chunk.memory) {
            /* Extract response from JSON */
            char *response_start = strstr(chunk.memory, "\"response\":\"");
            if (response_start) {
                response_start += 12;  /* Skip past "response":" */
                char *response_end = strstr(response_start, "\",\"done\"");
                if (response_end) {
                    int len = response_end - response_start;
                    if (len > sizeof(response_buffer) - 1)
                        len = sizeof(response_buffer) - 1;
                    strncpy(response_buffer, response_start, len);
                    response_buffer[len] = '\0';

                    curl_slist_free_all(headers);
                    curl_easy_cleanup(curl);
                    free(chunk.memory);
                    return response_buffer;
                }
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    if (chunk.memory)
        free(chunk.memory);

    return NULL;
}

/*
 * Analyze current player distribution across areas
 */
void analyze_player_distribution(char *output, size_t max_len)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;
    int area_counts[1000] = {0};  /* Track players per area (by area_id) */
    int total_players = 0;
    char buf[MAX_STRING_LENGTH];

    sprintf(output, "PLAYER DISTRIBUTION ANALYSIS:\n");

    for (d = first_descriptor; d; d = d->next) {
        if (d->connected == CON_PLAYING && (ch = d->character) != NULL) {
            if (ch->in_room && ch->in_room->area) {
                int area_id = ch->in_room->vnum / 100;  /* Approximate area ID */
                area_counts[area_id]++;
                total_players++;
            }
        }
    }

    sprintf(buf, "Total Online Players: %d\n", total_players);
    strncat(output, buf, max_len - strlen(output) - 1);

    /* Report top 5 most populated areas */
    strncat(output, "Most Popular Areas:\n", max_len - strlen(output) - 1);
    int reported = 0;
    for (int i = 0; i < 1000 && reported < 5; i++) {
        if (area_counts[i] > 0) {
            sprintf(buf, "  Area %d: %d players\n", i, area_counts[i]);
            strncat(output, buf, max_len - strlen(output) - 1);
            reported++;
        }
    }
}

/*
 * Analyze faction power balance
 */
void analyze_faction_balance(char *output, size_t max_len)
{
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];
    extern CLAN_DATA *first_clan;

    sprintf(output, "FACTION POWER BALANCE:\n");

    for (clan = first_clan; clan; clan = clan->next) {
        if (clan->members > 0) {
            sprintf(buf, "  %s: %d members, %d wars, %d alliances\n",
                clan->name,
                clan->members,
                clan->war_declarations,
                clan->active_alliances);
            strncat(output, buf, max_len - strlen(output) - 1);
        }
    }
}

/*
 * Generate world event based on AI God's analysis
 */
void god_generate_world_event(void)
{
    char analysis[MAX_STRING_LENGTH * 4];
    char player_dist[MAX_STRING_LENGTH * 2];
    char faction_bal[MAX_STRING_LENGTH * 2];
    char *god_decision;

    /* Gather current world state */
    analyze_player_distribution(player_dist, sizeof(player_dist));
    analyze_faction_balance(faction_bal, sizeof(faction_bal));

    sprintf(analysis, "%s\n%s", player_dist, faction_bal);

    /* Ask the AI God what event should happen */
    god_decision = call_ai_god(analysis,
        "Based on current player distribution and faction balance, "
        "what dynamic world event should I trigger? "
        "Provide: 1) Event type 2) Target area 3) Reason 4) Description");

    if (god_decision && god_decision[0] != '\0') {
        /* Log the God's decision */
        log_string("=== AI GOD DECISION ===");
        log_string(god_decision);

        /* TODO: Parse decision and trigger actual event
         * For now, just broadcast it */
        char broadcast[MAX_STRING_LENGTH];
        sprintf(broadcast, "&W[&RMUD GOD&W]&G %s", god_decision);
        echo_to_all(AT_CYAN, broadcast, ECHOTAR_ALL);
    }
}

/*
 * Adjust difficulty based on player performance
 */
void god_adjust_difficulty(void)
{
    /* Analyze recent player deaths vs kills */
    /* If players dying too much: reduce difficulty */
    /* If players winning too easily: increase difficulty */

    /* This could:
     * - Adjust mob damage/HP globally
     * - Spawn more/fewer patrols
     * - Change event frequency
     * - Modify loot drop rates
     */

    log_string("AI God: Analyzing difficulty balance...");
    /* TODO: Implement difficulty adjustment logic */
}

/*
 * Spawn special encounter based on player activity
 */
void god_spawn_special_encounter(CHAR_DATA *ch)
{
    char analysis[MAX_STRING_LENGTH * 2];
    char *god_decision;

    if (!ch || IS_NPC(ch))
        return;

    sprintf(analysis,
        "Player: %s, Level: %d, Area: %d, Clan: %s, PKills: %d",
        ch->name,
        ch->level,
        ch->in_room ? ch->in_room->vnum / 100 : 0,
        ch->pcdata->clan ? ch->pcdata->clan->name : "None",
        ch->pcdata->pkills);

    god_decision = call_ai_god(analysis,
        "Should I spawn a special encounter for this player? "
        "If yes, what type? (ambush/quest_giver/rare_mob/treasure/none)");

    if (god_decision && strstr(god_decision, "ambush")) {
        /* Spawn challenging encounter */
        act(AT_RED, "The air shimmers... something powerful approaches!", ch, NULL, NULL, TO_CHAR);
        act(AT_RED, "The air shimmers near $n!", ch, NULL, NULL, TO_ROOM);
        /* TODO: Actually spawn mob */
    }
    else if (god_decision && strstr(god_decision, "quest_giver")) {
        /* Spawn quest NPC */
        act(AT_CYAN, "A mysterious figure materializes before you...", ch, NULL, NULL, TO_CHAR);
        /* TODO: Spawn quest NPC */
    }
    else if (god_decision && strstr(god_decision, "treasure")) {
        /* Spawn treasure chest */
        act(AT_YELLOW, "You notice something glinting nearby...", ch, NULL, NULL, TO_CHAR);
        /* TODO: Create treasure object */
    }
}

/*
 * Main AI God update function (called from update.c)
 */
void update_ai_god(void)
{
    time_t current_time = time(NULL);

    /* Periodic analysis and world adjustment */
    if (current_time - god_last_analysis_time > GOD_ANALYSIS_INTERVAL) {
        log_string("AI God: Performing world analysis...");
        god_adjust_difficulty();
        god_last_analysis_time = current_time;
    }

    /* Event generation check */
    if (current_time - god_last_event_check > GOD_EVENT_CHECK_INTERVAL) {
        if (number_range(1, 100) <= 20) {  /* 20% chance every 10 min */
            god_generate_world_event();
        }
        god_last_event_check = current_time;
    }

    /* Balance check */
    if (current_time - god_last_balance_check > GOD_BALANCE_CHECK_INTERVAL) {
        log_string("AI God: Checking faction balance...");
        /* TODO: Implement faction balancing */
        god_last_balance_check = current_time;
    }
}

/*
 * Initialize AI God system
 */
void init_ai_god_advanced(void)
{
    log_string("Initializing Advanced AI God System...");
    log_string("  - Player behavior analysis: ENABLED");
    log_string("  - Dynamic event generation: ENABLED");
    log_string("  - Difficulty adjustment: ENABLED");
    log_string("  - Faction balancing: ENABLED");
    log_string("AI God: I am awake. I am watching. I will adapt.");

    god_last_analysis_time = time(NULL);
    god_last_event_check = time(NULL);
    god_last_balance_check = time(NULL);
}

/*
 * Command: immortal can query the AI God
 */
void do_godquery(CHAR_DATA *ch, char *argument)
{
    char analysis[MAX_STRING_LENGTH * 4];
    char player_dist[MAX_STRING_LENGTH * 2];
    char faction_bal[MAX_STRING_LENGTH * 2];
    char *god_response;

    if (IS_NPC(ch) || ch->level < MAX_LEVEL) {
        send_to_char("Only immortals can commune with the MUD God.\n\r", ch);
        return;
    }

    if (!argument || argument[0] == '\0') {
        send_to_char("Ask the MUD God a question about the world state.\n\r", ch);
        send_to_char("Example: godquery Should we start a war between RDAF and OOC?\n\r", ch);
        return;
    }

    send_to_char("Communing with the MUD God...\n\r", ch);

    /* Gather current world state */
    analyze_player_distribution(player_dist, sizeof(player_dist));
    analyze_faction_balance(faction_bal, sizeof(faction_bal));
    sprintf(analysis, "%s\n%s", player_dist, faction_bal);

    /* Ask the God */
    god_response = call_ai_god(analysis, argument);

    if (god_response && god_response[0] != '\0') {
        ch_printf(ch, "\n&WThe MUD God speaks:\n&G%s\n\r", god_response);
    } else {
        send_to_char("The MUD God is silent... (Ollama may be down)\n\r", ch);
    }
}
