/****************************************************************************
 * World History Tracker - Complete event and relationship tracking system
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"
#include "world_history_tracker.h"

/*****************************************************************************
 * Global Variables
 *****************************************************************************/

WORLD_HISTORY *global_history = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_world_history(void)
{
    log_string("WORLD HISTORY: Initializing tracking system...");

    global_history = (WORLD_HISTORY *)calloc(1, sizeof(WORLD_HISTORY));

    global_history->first_event = NULL;
    global_history->last_event = NULL;
    global_history->total_events = 0;

    global_history->first_char_ref = NULL;
    global_history->total_char_refs = 0;

    global_history->first_book = NULL;
    global_history->total_books = 0;

    global_history->first_kill = NULL;
    global_history->total_kills = 0;

    global_history->events_today = 0;
    global_history->events_this_week = 0;
    global_history->player_actions_tracked = 0;
    global_history->mob_creations_tracked = 0;

    global_history->last_save = time(NULL);
    global_history->last_cleanup = time(NULL);

    log_string("WORLD HISTORY: System initialized");
}

/*****************************************************************************
 * Event Creation
 *****************************************************************************/

HISTORY_EVENT *create_history_event(EVENT_TYPE type, char *primary, char *secondary,
                                     char *description, int importance)
{
    HISTORY_EVENT *event;

    event = (HISTORY_EVENT *)calloc(1, sizeof(HISTORY_EVENT));

    event->type = type;
    event->timestamp = time(NULL);
    event->importance = URANGE(1, importance, 10);

    event->primary_actor = primary ? strdup(primary) : NULL;
    event->secondary_actor = secondary ? strdup(secondary) : NULL;
    event->tertiary_actor = NULL;

    event->description = description ? strdup(description) : NULL;
    event->location = NULL;
    event->context_data = NULL;

    event->book_vnum = 0;
    event->mob_vnum = 0;
    event->obj_vnum = 0;
    event->room_vnum = 0;

    event->is_player_event = FALSE;
    event->is_public = TRUE;
    event->archived = FALSE;

    return event;
}

void add_event_to_history(HISTORY_EVENT *event)
{
    if (!global_history)
        return;

    /* Add to linked list */
    if (!global_history->first_event)
    {
        global_history->first_event = event;
        global_history->last_event = event;
        event->prev = NULL;
        event->next = NULL;
    }
    else
    {
        global_history->last_event->next = event;
        event->prev = global_history->last_event;
        event->next = NULL;
        global_history->last_event = event;
    }

    global_history->total_events++;
    global_history->events_today++;

    /* Add to character references */
    if (event->primary_actor)
        add_event_to_char_ref(event->primary_actor, event);
    if (event->secondary_actor)
        add_event_to_char_ref(event->secondary_actor, event);
}

/*****************************************************************************
 * Kill Event Recording
 *****************************************************************************/

void record_kill_event(char *killer, char *victim, char *method, char *location)
{
    HISTORY_EVENT *event;
    KILL_TRACKER *kill_track;
    char buf[MAX_STRING_LENGTH];

    if (!killer || !victim)
        return;

    /* Create history event */
    sprintf(buf, "%s killed %s using %s at %s",
            killer, victim, method ? method : "unknown means",
            location ? location : "unknown location");

    event = create_history_event(EVENT_KILL, killer, victim, buf, 7);
    event->location = location ? strdup(location) : NULL;
    event->context_data = method ? strdup(method) : NULL;
    event->is_public = TRUE;

    add_event_to_history(event);

    /* Create specific kill tracker */
    kill_track = (KILL_TRACKER *)calloc(1, sizeof(KILL_TRACKER));
    kill_track->killer_name = strdup(killer);
    kill_track->victim_name = strdup(victim);
    kill_track->when = time(NULL);
    kill_track->how = method ? strdup(method) : strdup("unknown");
    kill_track->where = location ? strdup(location) : strdup("unknown");
    kill_track->was_player_kill = FALSE; /* Set by caller if needed */

    /* Add to kill list */
    kill_track->next = global_history->first_kill;
    global_history->first_kill = kill_track;
    global_history->total_kills++;

    log_printf("KILL TRACKED: %s -> %s", killer, victim);
}

/*****************************************************************************
 * Book Event Recording
 *****************************************************************************/

void record_book_written(char *author, char *title, int book_vnum, char *content)
{
    HISTORY_EVENT *event;
    BOOK_REFERENCE *book_ref;
    char buf[MAX_STRING_LENGTH];

    if (!author || !title)
        return;

    /* Create history event */
    sprintf(buf, "%s wrote '%s'", author, title);
    event = create_history_event(EVENT_BOOK_WRITTEN, author, NULL, buf, 6);
    event->book_vnum = book_vnum;
    event->is_public = TRUE;

    add_event_to_history(event);

    /* Create book reference */
    book_ref = create_book_reference(book_vnum, title);
    book_ref->creation_event = event;

    log_printf("BOOK WRITTEN: '%s' by %s", title, author);
}

void record_book_mention(int book_vnum, char *book_title, char *mentioned_character)
{
    HISTORY_EVENT *event;
    char buf[MAX_STRING_LENGTH];

    if (!mentioned_character)
        return;

    /* Create history event */
    sprintf(buf, "%s was mentioned in the book '%s'",
            mentioned_character, book_title ? book_title : "Unknown Book");

    event = create_history_event(EVENT_BOOK_MENTION, mentioned_character, NULL, buf, 4);
    event->book_vnum = book_vnum;
    event->is_public = TRUE;

    add_event_to_history(event);

    /* Add to book reference */
    add_mention_to_book(book_vnum, mentioned_character);

    log_printf("BOOK MENTION: %s in book #%d", mentioned_character, book_vnum);
}

/*****************************************************************************
 * Player Action Recording
 *****************************************************************************/

void record_history_player_action(char *player, char *action, int importance)
{
    HISTORY_EVENT *event;

    if (!player || !action)
        return;

    event = create_history_event(EVENT_PLAYER_ACTION, player, NULL, action, importance);
    event->is_player_event = TRUE;
    event->is_public = (importance >= 6); /* Only important actions are public */

    add_event_to_history(event);

    global_history->player_actions_tracked++;

    if (importance >= 7)
        log_printf("PLAYER ACTION: %s - %s", player, action);
}

/*****************************************************************************
 * Mob Creation Recording
 *****************************************************************************/

void record_mob_creation(char *creator, char *created, char *what_created)
{
    HISTORY_EVENT *event;
    char buf[MAX_STRING_LENGTH];

    if (!creator || !created)
        return;

    sprintf(buf, "%s created %s (%s)", creator, created, what_created);

    event = create_history_event(EVENT_MOB_CREATION, creator, created, buf, 5);
    event->is_public = TRUE;

    add_event_to_history(event);

    global_history->mob_creations_tracked++;

    log_printf("MOB CREATION: %s created %s", creator, created);
}

/*****************************************************************************
 * Area Creation Recording
 *****************************************************************************/

void record_area_creation(char *creator, char *area_name, char *reason)
{
    HISTORY_EVENT *event;
    char buf[MAX_STRING_LENGTH];

    if (!creator || !area_name)
        return;

    sprintf(buf, "%s created new area '%s' - %s",
            creator, area_name, reason ? reason : "organic growth");

    event = create_history_event(EVENT_AREA_CREATED, creator, NULL, buf, 9);
    event->location = strdup(area_name);
    event->is_public = TRUE;

    add_event_to_history(event);

    log_printf("AREA CREATED: %s by %s", area_name, creator);
}

/*****************************************************************************
 * Generic World Event Recording
 *****************************************************************************/

void record_world_event(EVENT_TYPE type, char *description, int importance)
{
    HISTORY_EVENT *event;

    if (!description)
        return;

    event = create_history_event(type, "World", NULL, description, importance);
    event->is_public = TRUE;

    add_event_to_history(event);
}

/*****************************************************************************
 * Character Reference System
 *****************************************************************************/

CHAR_EVENT_REF *get_or_create_char_ref(char *character_name)
{
    CHAR_EVENT_REF *ref;

    if (!character_name || !global_history)
        return NULL;

    /* Search for existing reference */
    for (ref = global_history->first_char_ref; ref; ref = ref->next)
    {
        if (!str_cmp(ref->character_name, character_name))
            return ref;
    }

    /* Create new reference */
    ref = (CHAR_EVENT_REF *)calloc(1, sizeof(CHAR_EVENT_REF));
    ref->character_name = strdup(character_name);
    ref->num_events = 0;

    /* Add to list */
    ref->next = global_history->first_char_ref;
    global_history->first_char_ref = ref;
    global_history->total_char_refs++;

    return ref;
}

void add_event_to_char_ref(char *character_name, HISTORY_EVENT *event)
{
    CHAR_EVENT_REF *ref;

    ref = get_or_create_char_ref(character_name);

    if (!ref || ref->num_events >= 1000)
        return;

    ref->events[ref->num_events++] = event;
}

/*****************************************************************************
 * Book Reference System
 *****************************************************************************/

BOOK_REFERENCE *create_book_reference(int vnum, char *title)
{
    BOOK_REFERENCE *book;

    if (!global_history)
        return NULL;

    /* Check if already exists */
    for (book = global_history->first_book; book; book = book->next)
    {
        if (book->book_vnum == vnum)
            return book; /* Already exists */
    }

    /* Create new book reference */
    book = (BOOK_REFERENCE *)calloc(1, sizeof(BOOK_REFERENCE));
    book->book_vnum = vnum;
    book->book_title = title ? strdup(title) : NULL;
    book->num_mentions = 0;
    book->creation_event = NULL;

    /* Add to list */
    book->next = global_history->first_book;
    global_history->first_book = book;
    global_history->total_books++;

    return book;
}

void add_mention_to_book(int book_vnum, char *character_name)
{
    BOOK_REFERENCE *book;
    int i;

    book = get_book_by_vnum(book_vnum);

    if (!book || !character_name)
        return;

    /* Check if already mentioned */
    for (i = 0; i < book->num_mentions; i++)
    {
        if (!str_cmp(book->mentioned_characters[i], character_name))
            return; /* Already mentioned */
    }

    /* Add mention */
    if (book->num_mentions < 100)
    {
        book->mentioned_characters[book->num_mentions++] = strdup(character_name);
    }
}

BOOK_REFERENCE *get_book_by_vnum(int vnum)
{
    BOOK_REFERENCE *book;

    if (!global_history)
        return NULL;

    for (book = global_history->first_book; book; book = book->next)
    {
        if (book->book_vnum == vnum)
            return book;
    }

    return NULL;
}

BOOK_REFERENCE *get_book_by_title(char *title)
{
    BOOK_REFERENCE *book;

    if (!global_history || !title)
        return NULL;

    for (book = global_history->first_book; book; book = book->next)
    {
        if (book->book_title && !str_cmp(book->book_title, title))
            return book;
    }

    return NULL;
}

char **get_characters_in_book(int book_vnum, int *num_chars)
{
    BOOK_REFERENCE *book;

    *num_chars = 0;

    book = get_book_by_vnum(book_vnum);
    if (!book)
        return NULL;

    *num_chars = book->num_mentions;
    return book->mentioned_characters;
}

/*****************************************************************************
 * Query Functions
 *****************************************************************************/

HISTORY_EVENT **get_events_for_character(char *character_name, int *num_events)
{
    CHAR_EVENT_REF *ref;

    *num_events = 0;

    ref = get_or_create_char_ref(character_name);
    if (!ref)
        return NULL;

    *num_events = ref->num_events;
    return ref->events;
}

HISTORY_EVENT **get_kills_by_character(char *killer_name, int *num_kills)
{
    static HISTORY_EVENT *kill_events[1000];
    CHAR_EVENT_REF *ref;
    int i, count = 0;

    *num_kills = 0;

    ref = get_or_create_char_ref(killer_name);
    if (!ref)
        return NULL;

    /* Filter for kill events where this char is primary actor */
    for (i = 0; i < ref->num_events && count < 1000; i++)
    {
        if (ref->events[i]->type == EVENT_KILL &&
            !str_cmp(ref->events[i]->primary_actor, killer_name))
        {
            kill_events[count++] = ref->events[i];
        }
    }

    *num_kills = count;
    return count > 0 ? kill_events : NULL;
}

HISTORY_EVENT **get_deaths_of_character(char *victim_name, int *num_deaths)
{
    static HISTORY_EVENT *death_events[1000];
    CHAR_EVENT_REF *ref;
    int i, count = 0;

    *num_deaths = 0;

    ref = get_or_create_char_ref(victim_name);
    if (!ref)
        return NULL;

    /* Filter for kill events where this char is secondary actor (victim) */
    for (i = 0; i < ref->num_events && count < 1000; i++)
    {
        if (ref->events[i]->type == EVENT_KILL &&
            ref->events[i]->secondary_actor &&
            !str_cmp(ref->events[i]->secondary_actor, victim_name))
        {
            death_events[count++] = ref->events[i];
        }
    }

    *num_deaths = count;
    return count > 0 ? death_events : NULL;
}

HISTORY_EVENT **get_player_actions(char *player_name, int *num_actions)
{
    static HISTORY_EVENT *action_events[1000];
    CHAR_EVENT_REF *ref;
    int i, count = 0;

    *num_actions = 0;

    ref = get_or_create_char_ref(player_name);
    if (!ref)
        return NULL;

    /* Filter for player action events */
    for (i = 0; i < ref->num_events && count < 1000; i++)
    {
        if (ref->events[i]->type == EVENT_PLAYER_ACTION)
        {
            action_events[count++] = ref->events[i];
        }
    }

    *num_actions = count;
    return count > 0 ? action_events : NULL;
}

HISTORY_EVENT **get_recent_events(int num_requested)
{
    static HISTORY_EVENT *recent[100];
    HISTORY_EVENT *event;
    int count = 0;

    if (!global_history)
        return NULL;

    /* Walk backwards from last event */
    for (event = global_history->last_event;
         event && count < num_requested && count < 100;
         event = event->prev)
    {
        if (event->is_public)
            recent[count++] = event;
    }

    return count > 0 ? recent : NULL;
}

HISTORY_EVENT **get_events_by_type(EVENT_TYPE type, int *num_events)
{
    static HISTORY_EVENT *type_events[1000];
    HISTORY_EVENT *event;
    int count = 0;

    *num_events = 0;

    if (!global_history)
        return NULL;

    for (event = global_history->first_event;
         event && count < 1000;
         event = event->next)
    {
        if (event->type == type)
            type_events[count++] = event;
    }

    *num_events = count;
    return count > 0 ? type_events : NULL;
}

KILL_TRACKER **get_top_killers(int limit)
{
    /* This would need sorting implementation - simplified for now */
    static KILL_TRACKER *top[100];
    KILL_TRACKER *kill;
    int count = 0;

    if (!global_history)
        return NULL;

    for (kill = global_history->first_kill; kill && count < limit && count < 100; kill = kill->next)
    {
        top[count++] = kill;
    }

    return count > 0 ? top : NULL;
}

/*****************************************************************************
 * Display Functions
 *****************************************************************************/

void show_character_history(CHAR_DATA *ch, char *character_name)
{
    HISTORY_EVENT **events;
    int num_events, i;
    char time_buf[100];

    ch_printf(ch, "\n&C╔════════════════════════════════════════════════════════════════════╗\n\r");
    ch_printf(ch, "&C║              &WHistory of %s%-35s&C ║\n\r", character_name, "");
    ch_printf(ch, "&C╚════════════════════════════════════════════════════════════════════╝\n\r\n\r");

    events = get_events_for_character(character_name, &num_events);

    if (!events || num_events == 0)
    {
        ch_printf(ch, "No historical records found for %s.\n\r", character_name);
        return;
    }

    ch_printf(ch, "&WTotal events: &Y%d\n\r\n\r", num_events);

    for (i = 0; i < num_events && i < 50; i++)
    {
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", localtime(&events[i]->timestamp));

        ch_printf(ch, "&C[&Y%s&C] &W%s\n\r", time_buf, events[i]->description);

        if (events[i]->location)
            ch_printf(ch, "   &GLoc: &g%s\n\r", events[i]->location);
    }

    if (num_events > 50)
        ch_printf(ch, "\n\r&Y... and %d more events (showing first 50)\n\r", num_events - 50);
}

void show_book_mentions(CHAR_DATA *ch, int book_vnum)
{
    BOOK_REFERENCE *book;
    int i;

    book = get_book_by_vnum(book_vnum);

    if (!book)
    {
        ch_printf(ch, "No book found with vnum %d.\n\r", book_vnum);
        return;
    }

    ch_printf(ch, "\n&C╔════════════════════════════════════════════════════════════════════╗\n\r");
    ch_printf(ch, "&C║         &WCharacters Mentioned in: %s%-28s&C ║\n\r",
              book->book_title ? book->book_title : "Unknown", "");
    ch_printf(ch, "&C╚════════════════════════════════════════════════════════════════════╝\n\r\n\r");

    if (book->num_mentions == 0)
    {
        ch_printf(ch, "No characters mentioned in this book yet.\n\r");
        return;
    }

    for (i = 0; i < book->num_mentions; i++)
    {
        ch_printf(ch, "&W%2d. &Y%s\n\r", i + 1, book->mentioned_characters[i]);
    }
}

void show_kill_history(CHAR_DATA *ch, char *character_name)
{
    HISTORY_EVENT **kills, **deaths;
    int num_kills, num_deaths, i;

    ch_printf(ch, "\n&R╔════════════════════════════════════════════════════════════════════╗\n\r");
    ch_printf(ch, "&R║            &WKill History: %s%-37s&R ║\n\r", character_name, "");
    ch_printf(ch, "&R╚════════════════════════════════════════════════════════════════════╝\n\r\n\r");

    kills = get_kills_by_character(character_name, &num_kills);
    deaths = get_deaths_of_character(character_name, &num_deaths);

    ch_printf(ch, "&WKills: &R%d    &WDeaths: &r%d\n\r\n\r", num_kills, num_deaths);

    if (num_kills > 0)
    {
        ch_printf(ch, "&R═══ Kills ═══\n\r");
        for (i = 0; i < num_kills && i < 20; i++)
        {
            ch_printf(ch, "&W• &Y%s\n\r", kills[i]->description);
        }
        if (num_kills > 20)
            ch_printf(ch, "&Y... and %d more\n\r", num_kills - 20);
    }

    if (num_deaths > 0)
    {
        ch_printf(ch, "\n\r&r═══ Deaths ═══\n\r");
        for (i = 0; i < num_deaths && i < 20; i++)
        {
            ch_printf(ch, "&W• &r%s\n\r", deaths[i]->description);
        }
        if (num_deaths > 20)
            ch_printf(ch, "&r... and %d more\n\r", num_deaths - 20);
    }
}

void show_recent_world_events(CHAR_DATA *ch, int num_events)
{
    HISTORY_EVENT **events;
    int i;
    char time_buf[100];

    events = get_recent_events(num_events);

    if (!events)
    {
        ch_printf(ch, "No recent events found.\n\r");
        return;
    }

    ch_printf(ch, "\n&C╔════════════════════════════════════════════════════════════════════╗\n\r");
    ch_printf(ch, "&C║                    &WRecent World Events&C                          ║\n\r");
    ch_printf(ch, "&C╚════════════════════════════════════════════════════════════════════╝\n\r\n\r");

    for (i = 0; i < num_events; i++)
    {
        if (!events[i])
            break;

        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", localtime(&events[i]->timestamp));

        ch_printf(ch, "&C[&Y%s&C] &W%s\n\r", time_buf, events[i]->description);
    }
}

void show_player_impact(CHAR_DATA *ch, char *player_name)
{
    HISTORY_EVENT **actions;
    int num_actions, i;
    int importance_score = 0;

    ch_printf(ch, "\n&C╔════════════════════════════════════════════════════════════════════╗\n\r");
    ch_printf(ch, "&C║           &WPlayer Impact Report: %s%-29s&C ║\n\r", player_name, "");
    ch_printf(ch, "&C╚════════════════════════════════════════════════════════════════════╝\n\r\n\r");

    actions = get_player_actions(player_name, &num_actions);

    if (!actions || num_actions == 0)
    {
        ch_printf(ch, "No significant actions recorded for %s.\n\r", player_name);
        return;
    }

    /* Calculate importance score */
    for (i = 0; i < num_actions; i++)
    {
        importance_score += actions[i]->importance;
    }

    ch_printf(ch, "&WTotal Actions: &Y%d\n\r", num_actions);
    ch_printf(ch, "&WImportance Score: &Y%d\n\r", importance_score);
    ch_printf(ch, "&WWorld Impact Level: &Y%s\n\r\n\r",
              importance_score > 500 ? "LEGENDARY" :
              importance_score > 200 ? "MAJOR" :
              importance_score > 100 ? "SIGNIFICANT" :
              importance_score > 50  ? "MODERATE" : "MINOR");

    ch_printf(ch, "&C═══ Recent Actions ═══\n\r");
    for (i = 0; i < num_actions && i < 30; i++)
    {
        ch_printf(ch, "&W[&YImp:%d&W] &G%s\n\r",
                  actions[i]->importance, actions[i]->description);
    }
}

void show_history_statistics(CHAR_DATA *ch)
{
    if (!global_history)
    {
        send_to_char("History system not initialized.\n\r", ch);
        return;
    }

    ch_printf(ch, "\n&C╔════════════════════════════════════════════════════════════════════╗\n\r");
    ch_printf(ch, "&C║                &WWorld History Statistics&C                         ║\n\r");
    ch_printf(ch, "&C╚════════════════════════════════════════════════════════════════════╝\n\r\n\r");

    ch_printf(ch, "&WTotal Events: &Y%d\n\r", global_history->total_events);
    ch_printf(ch, "&WTotal Kills: &R%d\n\r", global_history->total_kills);
    ch_printf(ch, "&WTotal Books: &G%d\n\r", global_history->total_books);
    ch_printf(ch, "&WCharacters Tracked: &Y%d\n\r", global_history->total_char_refs);
    ch_printf(ch, "\n\r");
    ch_printf(ch, "&WEvents Today: &Y%d\n\r", global_history->events_today);
    ch_printf(ch, "&WEvents This Week: &Y%d\n\r", global_history->events_this_week);
    ch_printf(ch, "&WPlayer Actions: &Y%d\n\r", global_history->player_actions_tracked);
    ch_printf(ch, "&WMob Creations: &Y%d\n\r", global_history->mob_creations_tracked);
}

int get_character_importance_score(char *character_name)
{
    HISTORY_EVENT **events;
    int num_events, i, score = 0;

    events = get_events_for_character(character_name, &num_events);

    if (!events)
        return 0;

    for (i = 0; i < num_events; i++)
    {
        score += events[i]->importance;
    }

    return score;
}

int get_event_count_for_character(char *character_name)
{
    int num_events;
    get_events_for_character(character_name, &num_events);
    return num_events;
}

/*****************************************************************************
 * Persistence - Stubs for now (would save to files)
 *****************************************************************************/

void save_world_history(void)
{
    save_events_to_file();
    save_book_references();
    save_kill_tracker();

    if (global_history)
        global_history->last_save = time(NULL);
}

void load_world_history(void)
{
    load_events_from_file();
    load_book_references();
    load_kill_tracker();
}

void save_events_to_file(void)
{
    /* TODO: Implement file saving */
    log_string("WORLD HISTORY: Events saved (stub)");
}

void load_events_from_file(void)
{
    /* TODO: Implement file loading */
    log_string("WORLD HISTORY: Events loaded (stub)");
}

void save_book_references(void)
{
    /* TODO: Implement */
}

void load_book_references(void)
{
    /* TODO: Implement */
}

void save_kill_tracker(void)
{
    /* TODO: Implement */
}

void load_kill_tracker(void)
{
    /* TODO: Implement */
}

/*****************************************************************************
 * Maintenance
 *****************************************************************************/

void cleanup_old_events(void)
{
    /* Auto-cleanup very old low-importance events */
    HISTORY_EVENT *event, *next;
    time_t cutoff = time(NULL) - (90 * 24 * 60 * 60); /* 90 days */
    int cleaned = 0;

    if (!global_history)
        return;

    for (event = global_history->first_event; event; event = next)
    {
        next = event->next;

        /* Keep important events forever */
        if (event->importance >= 7)
            continue;

        /* Clean old unimportant events */
        if (event->timestamp < cutoff && event->importance < 5)
        {
            /* Remove from list and free */
            if (event->prev)
                event->prev->next = event->next;
            else
                global_history->first_event = event->next;

            if (event->next)
                event->next->prev = event->prev;
            else
                global_history->last_event = event->prev;

            /* Free memory */
            if (event->primary_actor) free(event->primary_actor);
            if (event->secondary_actor) free(event->secondary_actor);
            if (event->description) free(event->description);
            if (event->location) free(event->location);
            free(event);

            cleaned++;
        }
    }

    if (cleaned > 0)
        log_printf("WORLD HISTORY: Cleaned %d old events", cleaned);
}

void archive_ancient_events(void)
{
    /* Mark very old events as archived */
    /* Could move to separate data structure for efficiency */
}

void rebuild_event_indices(void)
{
    /* Rebuild character reference indices */
    /* Useful after loading from file */
}
