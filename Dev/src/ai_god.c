/****************************************************************************
 * AI God - Special function for THE GOD mob
 * Integrates with Ollama-powered AI service for omniscient deity behavior
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "mud.h"

#define AI_SERVICE_URL "http://localhost:3000"
#define AI_GOD_TICK_RATE 30  /* Seconds between god "thoughts" */

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

/* Build JSON payload with world state */
char *build_world_state_json(CHAR_DATA *god)
{
    static char json[MAX_STRING_LENGTH * 4];
    CHAR_DATA *ch;
    DESCRIPTOR_DATA *d;
    int player_count = 0;
    char player_list[MAX_STRING_LENGTH] = "";
    char time_str[100];
    char weather_str[100];

    /* Count online players */
    for (d = first_descriptor; d; d = d->next) {
        if (d->connected == CON_PLAYING && d->character) {
            player_count++;
            if (player_list[0] != '\0')
                strcat(player_list, ", ");
            strcat(player_list, d->character->name);
        }
    }

    /* Get time and weather */
    sprintf(time_str, "Hour %d of Day %d, Month %d, Year %d",
            time_info.hour, time_info.day, time_info.month, time_info.year);

    sprintf(weather_str, "%s, %s",
            weather_info.sky == SKY_CLOUDLESS ? "Clear skies" :
            weather_info.sky == SKY_CLOUDY ? "Cloudy" :
            weather_info.sky == SKY_RAINING ? "Raining" : "Lightning storm",
            weather_info.temp_curr < 20 ? "Cold" :
            weather_info.temp_curr < 40 ? "Cool" :
            weather_info.temp_curr < 60 ? "Mild" :
            weather_info.temp_curr < 80 ? "Warm" : "Hot");

    /* Build JSON */
    sprintf(json,
        "{"
        "\"world_state\":{"
        "\"players\":[],\"mobs\":[],\"clans\":[],"
        "\"recent_events\":[\"Players online: %d (%s)\"],"
        "\"time\":\"%s\","
        "\"weather\":\"%s\""
        "},"
        "\"trigger\":\"periodic_check\","
        "\"context\":\"The God observes the world\""
        "}",
        player_count, player_list, time_str, weather_str);

    return json;
}

/* Call AI service /god/think endpoint */
int ai_god_think(CHAR_DATA *god)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    char url[256];
    char *json_payload;
    struct curl_slist *headers = NULL;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(!curl) {
        free(chunk.memory);
        return FALSE;
    }

    /* Prepare request */
    sprintf(url, "%s/god/think", AI_SERVICE_URL);
    json_payload = build_world_state_json(god);

    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    /* Execute request */
    res = curl_easy_perform(curl);

    if(res == CURLE_OK && chunk.memory) {
        /* Parse response and make god act */
        /* For now, just log it */
        log_printf("AI God thought: %s", chunk.memory);

        /* TODO: Parse JSON response and execute god's action */
        /* if response.should_act is true, call ai_god_act() */
    }

    /* Cleanup */
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(chunk.memory);
    curl_global_cleanup();

    return res == CURLE_OK;
}

/* Special function for THE GOD mob */
bool spec_ai_god(CHAR_DATA *ch)
{
    static time_t last_think = 0;
    time_t now = current_time;

    if (!ch || !ch->in_room)
        return FALSE;

    /* God thinks every AI_GOD_TICK_RATE seconds */
    if (now - last_think < AI_GOD_TICK_RATE)
        return FALSE;

    last_think = now;

    /* Call AI service to process world state */
    ai_god_think(ch);

    return TRUE;
}

/* Initialize AI God system */
void init_ai_god(void)
{
    log_string("AI God system initialized - The God awakens...");
}
