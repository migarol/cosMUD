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
    /* TODO: Implement actual HTTP connection test to OLLAMA_HOST:OLLAMA_PORT */
    /* For now, assume Ollama is not available */
    /* This prevents dependency on libcurl */

    ollama_last_error = str_dup("Ollama service not configured (requires libcurl)");
    return FALSE;
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
    if (!ollama_enabled || !prompt)
        return NULL;

    /* TODO: Implement actual HTTP POST to Ollama API */
    /* Requires libcurl or similar HTTP library */
    /* For now, return NULL to trigger fallback */

    ollama_last_error = str_dup("HTTP request not implemented");
    return NULL;
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
