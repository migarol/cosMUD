/****************************************************************************
 * NPC AI - Give life to NPCs with small LLM models
 * Integrates with AI service for intelligent NPC conversations
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "mud.h"

#define AI_SERVICE_URL "http://localhost:3000"
#define NPC_AI_ENABLED TRUE

/* Response buffer for CURL */
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

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

/* Determine NPC type based on mob flags/properties */
char *get_npc_type(CHAR_DATA *mob)
{
    if (!mob || !IS_NPC(mob))
        return "citizen";

    /* Important NPCs: trainers, shopkeepers, quest givers */
    if (xIS_SET(mob->act, ACT_TRAIN) ||
        xIS_SET(mob->act, ACT_PRACTICE) ||
        xIS_SET(mob->act, ACT_BANKER))
        return "important";

    /* Guild masters and immortals */
    if (xIS_SET(mob->act, ACT_IMMORTAL) || mob->level >= 50)
        return "important";

    /* Default to citizen */
    return "citizen";
}

/* Extract NPC response from JSON */
char *extract_npc_response(char *json_response)
{
    static char response[MAX_STRING_LENGTH];
    char *start, *end;

    /* Look for "npc_says": "..." pattern */
    start = strstr(json_response, "\"npc_says\"");
    if (!start)
        return "...";

    start = strchr(start, ':');
    if (!start)
        return "...";

    start = strchr(start, '"');
    if (!start)
        return "...";
    start++; /* Move past opening quote */

    end = strchr(start, '"');
    if (!end)
        return "...";

    /* Copy response */
    int len = end - start;
    if (len >= MAX_STRING_LENGTH)
        len = MAX_STRING_LENGTH - 1;

    strncpy(response, start, len);
    response[len] = '\0';

    return response;
}

/* Call AI service to get NPC response to player talk */
char *npc_ai_talk(CHAR_DATA *mob, CHAR_DATA *ch, char *message)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    char url[256];
    char json_payload[MAX_STRING_LENGTH * 2];
    char *npc_response;
    struct curl_slist *headers = NULL;
    NPC_MEMORY *mem;
    int reputation = 0;

    if (!NPC_AI_ENABLED || !mob || !ch || !IS_NPC(mob))
        return NULL;

    /* Get NPC's memory of player for reputation */
    mem = load_npc_memory(mob->pIndexData->vnum, ch->name);
    if (mem)
    {
        reputation = mem->reputation;
        free(mem);
    }

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(!curl) {
        free(chunk.memory);
        return NULL;
    }

    /* Build JSON payload */
    sprintf(json_payload,
        "{"
        "\"npc_name\":\"%s\","
        "\"npc_vnum\":%d,"
        "\"player_name\":\"%s\","
        "\"message\":\"%s\","
        "\"reputation\":%d,"
        "\"location\":\"%s\","
        "\"npc_type\":\"%s\""
        "}",
        mob->short_descr,
        mob->pIndexData->vnum,
        ch->name,
        message,
        reputation,
        mob->in_room ? mob->in_room->name : "unknown",
        get_npc_type(mob));

    /* Prepare request */
    sprintf(url, "%s/npc/talk", AI_SERVICE_URL);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    /* Execute request */
    res = curl_easy_perform(curl);

    npc_response = NULL;
    if(res == CURLE_OK && chunk.memory) {
        npc_response = str_dup(extract_npc_response(chunk.memory));
    }

    /* Cleanup */
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(chunk.memory);
    curl_global_cleanup();

    return npc_response;
}

/* Process AI-powered NPC talk command */
void do_ai_talk(CHAR_DATA *ch, char *argument, CHAR_DATA *mob)
{
    char *ai_response;

    if (!mob || !IS_NPC(mob))
        return;

    if (!argument || argument[0] == '\0')
    {
        ch_printf(ch, "%s looks at you expectantly.\n\r", mob->short_descr);
        return;
    }

    /* Get AI response */
    ai_response = npc_ai_talk(mob, ch, argument);

    if (ai_response)
    {
        /* NPC responds with AI-generated text */
        act(AT_SAY, "$n says '$t'", mob, ai_response, ch, TO_VICT);
        act(AT_SAY, "$n says '$t'", mob, ai_response, ch, TO_NOTVICT);

        /* Update NPC memory */
        update_npc_memory(mob, ch, "talked to me", 1);

        STRFREE(ai_response);
    }
    else
    {
        /* Fallback if AI service is down */
        act(AT_SAY, "$n says 'I don't have much to say right now.'", mob, NULL, ch, TO_VICT);
        act(AT_SAY, "$n says 'I don't have much to say right now.'", mob, NULL, ch, TO_NOTVICT);
    }
}

/* Special function for AI-powered NPCs */
bool spec_ai_npc(CHAR_DATA *ch)
{
    /* NPCs with this special function are AI-enabled */
    /* They respond intelligently when talked to */
    /* This is called by the mobile_update loop */

    /* For now, NPCs are passive and only respond when talked to */
    /* Future: NPCs could proactively interact based on world state */

    return FALSE;
}

/* Initialize NPC AI system */
void init_npc_ai(void)
{
    log_string("NPC AI system initialized - NPCs awakening...");
    log_string("  - TinyLlama for citizens/vendors");
    log_string("  - Phi-2 for important NPCs");
}
