/*****************************************************************************
 * Ollama AI Integration - Implementation
 *
 * Provides AI-generated descriptions with graceful fallback to templates.
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include "mud.h"
#include "ollama_integration.h"
#include "mob_home.h"

/* Response buffer for curl */
struct curl_response {
    char *data;
    size_t size;
};

/* Curl write callback */
static size_t ollama_curl_write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct curl_response *resp = (struct curl_response *)userp;

    char *ptr = realloc(resp->data, resp->size + realsize + 1);
    if(!ptr) {
        /* out of memory */
        return 0;
    }

    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, realsize);
    resp->size += realsize;
    resp->data[resp->size] = 0;

    return realsize;
}

/* Global state */
bool ollama_enabled = FALSE;
char *ollama_last_error = NULL;

/*
 * Initialize Ollama integration
 */
void init_ollama(void)
{
    log_string("  [Ollama] Checking for AI service...");

    /* Test connection */
    if (ollama_test_connection())
    {
        ollama_enabled = TRUE;
        log_string("  [Ollama] AI generation ENABLED");
        sprintf(log_buf, "  [Ollama] Connected to %s:%d", OLLAMA_HOST, OLLAMA_PORT);
        log_string(log_buf);
        sprintf(log_buf, "  [Ollama] Model: %s (timeout: %ds)", OLLAMA_MODEL, OLLAMA_TIMEOUT);
        log_string(log_buf);

        /* Log usage policy for transparency */
        log_string("  [Ollama] AI Usage Policy:");
        sprintf(log_buf, "          NPCs: %s (templates are faster for gameplay)",
            OLLAMA_USE_FOR_NPCS ? "YES" : "NO");
        log_string(log_buf);
        sprintf(log_buf, "          Rooms: %s (templates are faster for gameplay)",
            OLLAMA_USE_FOR_ROOMS ? "YES" : "NO");
        log_string(log_buf);
        sprintf(log_buf, "          Books: %s (quality over speed)",
            OLLAMA_USE_FOR_BOOKS ? "YES" : "NO");
        log_string(log_buf);
        sprintf(log_buf, "          Culture/History: %s (background processing)",
            OLLAMA_USE_FOR_CULTURE ? "YES" : "NO");
        log_string(log_buf);
    }
    else
    {
        ollama_enabled = FALSE;
        log_string("  [Ollama] AI generation DISABLED (using templates)");
        log_string("  [Ollama] To enable: Install Ollama and run 'ollama pull llama3.2:1b'");
    }
}

/*
 * Test connection to Ollama server
 */
bool ollama_test_connection(void)
{
    CURL *curl;
    CURLcode res;
    char url[256];
    bool success = FALSE;

    curl = curl_easy_init();
    if(!curl)
    {
        ollama_last_error = str_dup("Failed to initialize curl");
        return FALSE;
    }

    sprintf(url, "%s:%d/api/tags", OLLAMA_HOST, OLLAMA_PORT);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, OLLAMA_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  /* HEAD request */

    res = curl_easy_perform(curl);

    if(res == CURLE_OK)
    {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if(response_code == 200)
        {
            success = TRUE;
            if(ollama_last_error)
            {
                DISPOSE(ollama_last_error);
                ollama_last_error = NULL;
            }
        }
        else
        {
            char buf[256];
            sprintf(buf, "Ollama returned HTTP %ld", response_code);
            ollama_last_error = str_dup(buf);
        }
    }
    else
    {
        char buf[256];
        sprintf(buf, "Connection failed: %s", curl_easy_strerror(res));
        ollama_last_error = str_dup(buf);
    }

    curl_easy_cleanup(curl);
    return success;
}

/*
 * Shutdown Ollama
 */
void ollama_shutdown(void)
{
    if (ollama_last_error)
    {
        DISPOSE(ollama_last_error);
        ollama_last_error = NULL;
    }
}

/*
 * Generate room description using Ollama or fallback
 */
char *ollama_generate_room_description(int home_type, char *owner_name, char *context)
{
    char prompt[MAX_STRING_LENGTH];
    char *result;
    extern char *home_type_name(int home_type);

    /* If Ollama is available AND policy allows, use it */
    if (ollama_enabled && OLLAMA_USE_FOR_ROOMS)
    {
        sprintf(prompt,
            "Describe a %s in a fantasy MUD. Owner: %s. Context: %s. "
            "Write 2-3 vivid sentences focusing on atmosphere and details. "
            "No meta-commentary, just the description.",
            home_type_name(home_type),
            owner_name ? owner_name : "unknown",
            context ? context : "standard dwelling");

        result = ollama_request(prompt, 200);
        if (result)
            return result;
    }

    /* Fallback to template-based generation */
    static char desc[MAX_STRING_LENGTH];

    switch (home_type)
    {
        case HOME_TYPE_HOVEL:
            sprintf(desc, "This cramped, dingy hovel barely keeps out the elements. "
                         "The walls are cracked, the floor is dirt, and the air is stale. "
                         "A single threadbare blanket lies in the corner.\n");
            break;

        case HOME_TYPE_APARTMENT:
            sprintf(desc, "This small but functional apartment provides basic shelter. "
                         "Simple furnishings line the walls - a bed, a table, a chair. "
                         "Clean but modest, it serves its purpose.\n");
            break;

        case HOME_TYPE_HOUSE:
            sprintf(desc, "This comfortable house speaks of middle-class stability. "
                         "Well-maintained furnishings fill the space efficiently. "
                         "A fireplace crackles warmly in the corner.\n");
            break;

        case HOME_TYPE_MANOR:
            sprintf(desc, "This impressive manor displays wealth and taste. "
                         "Fine furnishings, art on the walls, and polished floors. "
                         "Every detail speaks of success and status.\n");
            break;

        case HOME_TYPE_PALACE:
            sprintf(desc, "This magnificent palace chamber is fit for royalty. "
                         "Opulent decorations, priceless art, and luxurious furnishings. "
                         "The very air seems to shimmer with power and prestige.\n");
            break;

        default:
            sprintf(desc, "A room.\n");
            break;
    }

    return str_dup(desc);
}

/*
 * Generate mob personality using Ollama or fallback
 */
char *ollama_generate_mob_personality(char *mob_name, char *race, int level, char *role)
{
    char prompt[MAX_STRING_LENGTH];
    char *result;

    /* Only use AI if policy allows - NPCs need fast generation */
    if (ollama_enabled && OLLAMA_USE_FOR_NPCS)
    {
        sprintf(prompt,
            "Create a brief personality for a fantasy MUD NPC: "
            "Name: %s, Race: %s, Level: %d, Role: %s. "
            "1-2 sentences describing their demeanor, quirks, and motivation.",
            mob_name ? mob_name : "unknown",
            race ? race : "human",
            level,
            role ? role : "villager");

        result = ollama_request(prompt, 150);
        if (result)
            return result;
    }

    /* Fallback to simple template */
    static char personality[512];

    sprintf(personality, "%s is a %s who goes about daily tasks with quiet determination.",
        mob_name ? mob_name : "This person",
        role ? role : "resident");

    return str_dup(personality);
}

/*
 * Generate district description
 */
char *ollama_generate_district_description(char *district_name, int home_type, char *city_name)
{
    char prompt[MAX_STRING_LENGTH];
    char *result;
    extern char *home_type_name(int home_type);

    if (ollama_enabled && OLLAMA_USE_FOR_ROOMS)
    {
        sprintf(prompt,
            "Describe a %s district called '%s' in the city of %s (fantasy MUD). "
            "2-3 sentences capturing the atmosphere, architecture, and social character.",
            home_type_name(home_type),
            district_name ? district_name : "residential area",
            city_name ? city_name : "the city");

        result = ollama_request(prompt, 200);
        if (result)
            return result;
    }

    /* Fallback template */
    static char desc[512];

    sprintf(desc, "This district is home to many residents of %s. "
                  "The streets are lined with %s homes, each reflecting the lives within.",
        city_name ? city_name : "the city",
        home_type_name(home_type));

    return str_dup(desc);
}

/*
 * Generate inn description
 */
char *ollama_generate_inn_description(char *inn_name, char *location, int quality)
{
    char prompt[MAX_STRING_LENGTH];
    char *result;

    if (ollama_enabled && OLLAMA_USE_FOR_ROOMS)
    {
        sprintf(prompt,
            "Describe an inn called '%s' in %s (fantasy MUD). Quality: %d/100. "
            "2-3 sentences covering atmosphere, clientele, and amenities.",
            inn_name ? inn_name : "The Inn",
            location ? location : "a remote location",
            quality);

        result = ollama_request(prompt, 200);
        if (result)
            return result;
    }

    /* Fallback template */
    static char desc[512];

    if (quality >= 75)
    {
        sprintf(desc, "%s is a well-maintained establishment with comfortable rooms "
                      "and attentive service. Travelers speak highly of its hospitality.",
            inn_name ? inn_name : "This inn");
    }
    else if (quality >= 40)
    {
        sprintf(desc, "%s provides decent lodging for weary travelers. "
                      "The rooms are clean enough and the price is fair.",
            inn_name ? inn_name : "This inn");
    }
    else
    {
        sprintf(desc, "%s is a rough establishment where only the desperate rest. "
                      "The beds are lumpy and the walls paper-thin.",
            inn_name ? inn_name : "This inn");
    }

    return str_dup(desc);
}

/*
 * Low-level Ollama request
 */
char *ollama_request(char *prompt, int max_tokens)
{
    CURL *curl;
    CURLcode res;
    char url[256];
    char *json_payload;
    struct curl_response response;
    struct curl_slist *headers = NULL;
    char *result = NULL;

    if (!ollama_enabled || !prompt)
        return NULL;

    curl = curl_easy_init();
    if(!curl)
    {
        ollama_last_error = str_dup("Failed to initialize curl");
        return NULL;
    }

    /* Prepare JSON payload */
    json_payload = malloc(MAX_STRING_LENGTH * 2);
    if(!json_payload)
    {
        curl_easy_cleanup(curl);
        return NULL;
    }

    sprintf(json_payload,
        "{\"model\":\"%s\",\"prompt\":%s,\"stream\":false,\"options\":{\"num_predict\":%d}}",
        OLLAMA_MODEL,
        ollama_escape_json(prompt),
        max_tokens);

    /* Initialize response buffer */
    response.data = malloc(1);
    response.size = 0;
    if(!response.data)
    {
        free(json_payload);
        curl_easy_cleanup(curl);
        return NULL;
    }

    sprintf(url, "%s:%d/api/generate", OLLAMA_HOST, OLLAMA_PORT);

    /* Set headers */
    headers = curl_slist_append(headers, "Content-Type: application/json");

    /* Configure curl */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ollama_curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);  /* 30 second timeout for generation */

    /* Perform request */
    res = curl_easy_perform(curl);

    if(res == CURLE_OK)
    {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if(response_code == 200 && response.data)
        {
            /* Parse JSON response to extract the "response" field */
            char *response_start = strstr(response.data, "\"response\":\"");
            if(response_start)
            {
                response_start += 12;  /* Skip past "response":" */
                char *response_end = strstr(response_start, "\",");
                if(!response_end)
                    response_end = strstr(response_start, "\"}");

                if(response_end)
                {
                    int len = response_end - response_start;
                    if(len > 0 && len < MAX_STRING_LENGTH)
                    {
                        result = malloc(len + 1);
                        if(result)
                        {
                            strncpy(result, response_start, len);
                            result[len] = '\0';

                            /* Unescape JSON special characters */
                            char *src = result, *dst = result;
                            while(*src)
                            {
                                if(*src == '\\' && *(src+1))
                                {
                                    src++;  /* Skip backslash */
                                    if(*src == 'n') *dst++ = '\n';
                                    else if(*src == 't') *dst++ = '\t';
                                    else if(*src == 'r') *dst++ = '\r';
                                    else *dst++ = *src;
                                    src++;
                                }
                                else
                                {
                                    *dst++ = *src++;
                                }
                            }
                            *dst = '\0';
                        }
                    }
                }
            }
        }
        else
        {
            char buf[256];
            sprintf(buf, "Ollama returned HTTP %ld", response_code);
            ollama_last_error = str_dup(buf);
        }
    }
    else
    {
        char buf[256];
        sprintf(buf, "Request failed: %s", curl_easy_strerror(res));
        ollama_last_error = str_dup(buf);
    }

    /* Cleanup */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(json_payload);
    free(response.data);

    return result;
}

/*
 * Low-level Ollama request with specific model
 * Allows choosing model and timeout on a per-request basis
 */
char *ollama_request_with_model(char *prompt, int max_tokens, char *model, int timeout)
{
    CURL *curl;
    CURLcode res;
    char url[256];
    char *json_payload;
    struct curl_response response;
    struct curl_slist *headers = NULL;
    char *result = NULL;

    if (!ollama_enabled || !prompt || !model)
        return NULL;

    curl = curl_easy_init();
    if(!curl)
    {
        ollama_last_error = str_dup("Failed to initialize curl");
        return NULL;
    }

    /* Prepare JSON payload */
    json_payload = malloc(MAX_STRING_LENGTH * 2);
    if(!json_payload)
    {
        curl_easy_cleanup(curl);
        return NULL;
    }

    sprintf(json_payload,
        "{\"model\":\"%s\",\"prompt\":%s,\"stream\":false,\"options\":{\"num_predict\":%d}}",
        model,  /* Use specified model instead of OLLAMA_MODEL */
        ollama_escape_json(prompt),
        max_tokens);

    /* Initialize response buffer */
    response.data = malloc(1);
    response.size = 0;
    if(!response.data)
    {
        free(json_payload);
        curl_easy_cleanup(curl);
        return NULL;
    }

    sprintf(url, "%s:%d/api/generate", OLLAMA_HOST, OLLAMA_PORT);

    /* Set headers */
    headers = curl_slist_append(headers, "Content-Type: application/json");

    /* Configure curl */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ollama_curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)timeout);  /* Use specified timeout */

    /* Perform request */
    res = curl_easy_perform(curl);

    if(res == CURLE_OK)
    {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if(response_code == 200 && response.data)
        {
            /* Parse JSON response to extract the "response" field */
            char *response_start = strstr(response.data, "\"response\":\"");
            if(response_start)
            {
                response_start += 12;  /* Skip past "response":" */
                char *response_end = strstr(response_start, "\",");
                if(!response_end)
                    response_end = strstr(response_start, "\"}");

                if(response_end)
                {
                    int len = response_end - response_start;
                    if(len > 0 && len < MAX_STRING_LENGTH)
                    {
                        result = malloc(len + 1);
                        if(result)
                        {
                            strncpy(result, response_start, len);
                            result[len] = '\0';

                            /* Unescape JSON special characters */
                            char *src = result, *dst = result;
                            while(*src)
                            {
                                if(*src == '\\' && *(src+1))
                                {
                                    src++;  /* Skip backslash */
                                    if(*src == 'n') *dst++ = '\n';
                                    else if(*src == 't') *dst++ = '\t';
                                    else if(*src == 'r') *dst++ = '\r';
                                    else *dst++ = *src;
                                    src++;
                                }
                                else
                                {
                                    *dst++ = *src++;
                                }
                            }
                            *dst = '\0';
                        }
                    }
                }
            }
        }
    }

    /* Cleanup */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(json_payload);
    free(response.data);

    return result;
}

/*
 * Ollama chat interface
 */
char *ollama_chat(char *system_prompt, char *user_prompt, int max_tokens)
{
    if (!ollama_enabled || !user_prompt)
        return NULL;

    /* TODO: Implement chat API with system context */
    return NULL;
}

/*
 * Escape JSON strings
 */
char *ollama_escape_json(char *str)
{
    static char escaped[MAX_STRING_LENGTH * 2];
    char *dst = escaped;
    char *src = str;

    if (!str)
        return "null";

    *dst++ = '"';

    while (*src && (dst - escaped) < MAX_STRING_LENGTH * 2 - 3)
    {
        if (*src == '"' || *src == '\\')
            *dst++ = '\\';

        *dst++ = *src++;
    }

    *dst++ = '"';
    *dst = '\0';

    return escaped;
}

/*
 * Check if Ollama is available
 */
bool ollama_is_available(void)
{
    return ollama_enabled;
}

/*
 * Get Ollama status string
 */
char *ollama_get_status(void)
{
    static char status[512];

    if (ollama_enabled)
    {
        sprintf(status, "Ollama AI: &GENABLED&w (%s:%d, model: %s)",
            OLLAMA_HOST, OLLAMA_PORT, OLLAMA_MODEL);
    }
    else
    {
        sprintf(status, "Ollama AI: &RDISABLED&w (using templates)\nReason: %s",
            ollama_last_error ? ollama_last_error : "Unknown");
    }

    return status;
}

/*
 * Multi-tier NPC dialogue generation
 * REAL-TIME function - must be FAST
 */
char *ollama_generate_npc_dialogue(CHAR_DATA *npc, char *player_message)
{
    char *model;
    int timeout;
    char prompt[4096];  /* Larger buffer for rich context */
    char *response;
    char *role_desc;
    char *area_name;
    char *room_name;
    char *personality;
    char *profession_name;
    char level_info[128];
    char context_info[512];

    if (!npc || !player_message)
        return NULL;

    /* Choose model based on NPC importance */
    switch (npc->ai_tier)
    {
        case NPC_AI_TIER_LEADER:
        case NPC_AI_TIER_IMPORTANT:
            model = OLLAMA_MODEL_SPEAKING;  /* TinyLlama - ultra fast */
            timeout = OLLAMA_TIMEOUT_SPEAKING;
            break;

        case NPC_AI_TIER_NORMAL:
            model = OLLAMA_MODEL_NPC;
            timeout = OLLAMA_TIMEOUT_NPC;
            break;

        default:
            /* Guards, vendors - use templates only */
            return NULL;  /* Caller will use template */
    }

    /* Build RICH context for professional roleplay */
    role_desc = npc->short_descr ? npc->short_descr : "a person";
    area_name = (npc->in_room && npc->in_room->area && npc->in_room->area->name)
                ? npc->in_room->area->name : "this area";
    room_name = (npc->in_room && npc->in_room->name)
                ? npc->in_room->name : "here";
    personality = npc->ai_personality ? npc->ai_personality : "professional";

    /* Profession context */
    profession_name = "resident";
    if (npc->profession > 0)
    {
        /* TODO: Map profession enum to name - for now use generic */
        profession_name = "worker";
    }

    /* Level/importance context */
    if (npc->level >= 90)
        sprintf(level_info, "You are extremely powerful and important.");
    else if (npc->level >= 60)
        sprintf(level_info, "You are experienced and respected.");
    else if (npc->level >= 30)
        sprintf(level_info, "You are competent at your work.");
    else
        sprintf(level_info, "You are new but eager to help.");

    /* Build context string */
    sprintf(context_info,
        "You work as a %s in %s. You are currently in %s. %s",
        profession_name,
        area_name,
        room_name,
        level_info);

    /* DIRECT DIALOGUE PROMPT - No narration, just speech */
    sprintf(prompt,
        "You are %s (%s) in %s.\n"
        "\n"
        "SPEAK DIRECTLY (first person, like real conversation):\n"
        "✓ GOOD: \"The training hall is north from here.\"\n"
        "✓ GOOD: \"I teach magic in the eastern wing.\"\n"
        "✓ GOOD: \"Welcome! I'm the headmistress here.\"\n"
        "\n"
        "DO NOT narrate or describe (third person):\n"
        "✗ BAD: \"As you walk down...\"\n"
        "✗ BAD: \"The player asks...\"\n"
        "✗ BAD: \"You proceed through...\"\n"
        "\n"
        "RULES:\n"
        "- English ONLY\n"
        "- Maximum 10 words\n"
        "- Direct speech only\n"
        "- Be helpful\n"
        "\n"
        "Player: \"%s\"\n"
        "You:",
        npc->name ? npc->name : "someone",
        role_desc,
        area_name,
        player_message);

    /* Use fast model with VERY short token limit to force concise responses */
    response = ollama_request_with_model(prompt, 30, model, timeout);

    return response;  /* NULL if timeout or error - caller will use template */
}

/*
 * Leader strategic thinking - BACKGROUND function
 * Called every 10 minutes from update_handler
 */
void leader_think_strategically(CHAR_DATA *leader)
{
    char prompt[MAX_STRING_LENGTH];
    char *decision;
    int economy_health;

    if (!leader || leader->ai_tier != NPC_AI_TIER_LEADER)
        return;

    /* Only think every 10 minutes */
    if (leader->last_think_time && (current_time - leader->last_think_time < 600))
        return;

    leader->last_think_time = current_time;

    /* Don't think if not in a valid area */
    if (!leader->in_room || !leader->in_room->area)
        return;

    /* Build strategic context prompt */
    economy_health = 50;  /* TODO: get real economy health from economy system */

    sprintf(prompt,
        "You are %s, leader of %s. "
        "Economy: %d/100. "
        "Decide one strategic action to improve the realm. One brief sentence.",
        leader->name ? leader->name : "the leader",
        leader->in_room->area->name ? leader->in_room->area->name : "the realm",
        economy_health);

    /* Use Phi-3 Mini for better strategic reasoning */
    decision = ollama_request_with_model(
        prompt,
        100,
        OLLAMA_MODEL_THINKING,
        OLLAMA_TIMEOUT_THINKING
    );

    if (decision)
    {
        /* Store decision */
        if (leader->current_strategy)
            DISPOSE(leader->current_strategy);
        leader->current_strategy = str_dup(decision);

        /* Log for debugging */
        sprintf(log_buf, "LEADER AI: %s decided: %s",
            leader->name, decision);
        log_string(log_buf);

        /* TODO: Apply decision to world state */
        /* This could adjust economy, spawn guards, etc. */

        free(decision);
    }
}

/*
 * Command: ollama - test and configure Ollama
 */
void do_ollama(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;

    if (get_trust(ch) < LEVEL_IMMORTAL)
    {
        send_to_char("Only immortals can manage AI integration.\n\r", ch);
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0' || !str_cmp(arg, "status"))
    {
        send_to_char("\n\r&Y=== Ollama AI Status ===&w\n\r\n\r", ch);
        ch_printf(ch, "%s\n\r\n\r", ollama_get_status());

        if (!ollama_enabled)
        {
            send_to_char("&WTo enable Ollama:&w\n\r", ch);
            send_to_char("1. Install Ollama: https://ollama.ai\n\r", ch);
            send_to_char("2. Run: ollama pull llama3.2\n\r", ch);
            send_to_char("3. Restart the MUD\n\r\n\r", ch);
        }

        return;
    }

    if (!str_cmp(arg, "test"))
    {
        send_to_char("Testing Ollama connection...\n\r", ch);

        if (ollama_test_connection())
        {
            send_to_char("&G[SUCCESS]&w Ollama is responding!\n\r", ch);
            ollama_enabled = TRUE;
        }
        else
        {
            send_to_char("&R[FAILED]&w Could not connect to Ollama.\n\r", ch);
            ch_printf(ch, "Error: %s\n\r", ollama_last_error ? ollama_last_error : "Unknown");
        }

        return;
    }

    if (!str_cmp(arg, "generate"))
    {
        char *result;

        send_to_char("Testing AI generation...\n\r", ch);

        result = ollama_generate_room_description(HOME_TYPE_HOUSE, "Test Character", "wizard's study");

        if (result)
        {
            ch_printf(ch, "\n\r&YGenerated Description:&w\n\r%s\n\r", result);
            DISPOSE(result);
        }
        else
        {
            send_to_char("&R[FAILED]&w Using template fallback.\n\r", ch);
        }

        return;
    }

    /* Help */
    send_to_char("\n\r&Y=== Ollama Commands ===&w\n\r\n\r", ch);
    send_to_char("ollama status   - Show AI status\n\r", ch);
    send_to_char("ollama test     - Test connection\n\r", ch);
    send_to_char("ollama generate - Test generation\n\r\n\r", ch);
}
