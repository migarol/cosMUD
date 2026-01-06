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
#include "mud.h"
#include "ollama_integration.h"
#include "mob_home.h"

#ifdef HAVE_CURL
#include <curl/curl.h>

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

    char *ptr = (char *)realloc(resp->data, resp->size + realsize + 1);
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
        {
            char log_buf[256];
            sprintf(log_buf, "  [Ollama] Connected to %s:%d", OLLAMA_HOST, OLLAMA_PORT);
            log_string(log_buf);
        }
    }
    else
    {
        ollama_enabled = FALSE;
        log_string("  [Ollama] AI generation DISABLED (using templates)");
        log_string("  [Ollama] To enable: Install Ollama and run 'ollama pull llama3.2'");
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

    /* If Ollama is available, use it */
    if (ollama_enabled)
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

    if (ollama_enabled)
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

    if (ollama_enabled)
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

    if (ollama_enabled)
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

    /* Prepare JSON payload - increased size for large Beeler prompts */
    json_payload = (char *)malloc(MAX_STRING_LENGTH * 8);
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

    {
        char log_buf[256];
        sprintf(log_buf, "OLLAMA DEBUG: json_payload size=%d", (int)strlen(json_payload));
        log_string(log_buf);
    }

    /* Initialize response buffer */
    response.data = (char *)malloc(1);
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

    {
        char log_buf[256];
        sprintf(log_buf, "OLLAMA DEBUG: curl_easy_perform result=%d", res);
        log_string(log_buf);
    }

    if(res == CURLE_OK)
    {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        {
            char log_buf[256];
            sprintf(log_buf, "OLLAMA DEBUG: HTTP response_code=%ld, response.size=%d", response_code, (int)response.size);
            log_string(log_buf);
        }

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
                        result = (char *)malloc(len + 1);
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
            char log_buf[512];
            sprintf(buf, "Ollama returned HTTP %ld", response_code);
            ollama_last_error = str_dup(buf);

            /* Log the error response from Ollama */
            if(response.data && response.size > 0)
            {
                int copy_len = response.size < 400 ? response.size : 400;
                sprintf(log_buf, "OLLAMA ERROR: %.*s", copy_len, response.data);
                log_string(log_buf);
            }
        }
    }
    else
    {
        char buf[256];
        sprintf(buf, "Request failed: %s", curl_easy_strerror(res));
        ollama_last_error = str_dup(buf);
        log_string(buf);
    }

    {
        char log_buf[256];
        sprintf(log_buf, "OLLAMA DEBUG: returning result=%s", result ? "SUCCESS" : "NULL");
        log_string(log_buf);
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
 * Increased buffer size to handle large Beeler prompts
 */
char *ollama_escape_json(char *str)
{
    static char escaped[MAX_STRING_LENGTH * 6];
    char *dst = escaped;
    char *src = str;

    if (!str)
        return "null";

    *dst++ = '"';

    while (*src && (dst - escaped) < MAX_STRING_LENGTH * 6 - 3)
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
 * Command: ollama - test and configure Ollama
 * Declared in mud.h with DECLARE_DO_FUN (provides C linkage)
 */
void do_ollama(CHAR_DATA *ch, const char *argument)
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

#else /* !HAVE_CURL */

/* Stub implementations when Ollama/CURL is not available */

char *ollama_generate_room_description(ROOM_INDEX_DATA *room, const char *context) {
    return NULL; /* Will fall back to template */
}

char *ollama_generate_mob_description(CHAR_DATA *mob, const char *context) {
    return NULL; /* Will fall back to template */
}

char *ollama_analyze_world_context(const char *area_name, const char *context_data) {
    return NULL; /* Will fall back to simple analysis */
}

void do_ollama(CHAR_DATA *ch, const char *argument) {
    send_to_char("Ollama AI support was disabled at compile time.\n\r", ch);
    send_to_char("Recompile with libcurl installed to enable AI features.\n\r", ch);
}

#endif /* HAVE_CURL */
