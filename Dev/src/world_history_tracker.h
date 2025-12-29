/****************************************************************************
 * World History Tracker - Complete event and relationship tracking system
 * Tracks: kills, book mentions, player actions, mob creations, world events
 * Allows queries: "who killed X", "who's in book Y", "what did player Z do"
 ****************************************************************************/

#ifndef WORLD_HISTORY_TRACKER_H
#define WORLD_HISTORY_TRACKER_H

/*****************************************************************************
 * Event Types
 *****************************************************************************/

typedef enum
{
    EVENT_KILL,              /* Someone killed someone */
    EVENT_BOOK_MENTION,      /* Someone mentioned in a book */
    EVENT_BOOK_WRITTEN,      /* Someone wrote a book */
    EVENT_PLAYER_ACTION,     /* Player did something significant */
    EVENT_MOB_CREATION,      /* Mob created another mob/object */
    EVENT_AREA_CREATED,      /* New area created */
    EVENT_BUILDING_BUILT,    /* Building constructed */
    EVENT_WAR_DECLARED,      /* War between factions */
    EVENT_TREATY_SIGNED,     /* Peace treaty signed */
    EVENT_DISASTER,          /* Natural disaster */
    EVENT_ACHIEVEMENT,       /* Major achievement */
    EVENT_RELATIONSHIP_CHANGE, /* Major relationship shift */
    EVENT_CULTURAL,          /* Cultural event (festival, tradition) */
    EVENT_ECONOMIC,          /* Economic event (trade route, market) */
    EVENT_FAMILY,            /* Family event (birth, marriage, death) */
    EVENT_MAX
} EVENT_TYPE;

/*****************************************************************************
 * Data Structures
 *****************************************************************************/

/* History event - base structure for all events */
typedef struct history_event HISTORY_EVENT;
struct history_event
{
    HISTORY_EVENT  *next;
    HISTORY_EVENT  *prev;

    EVENT_TYPE      type;
    time_t          timestamp;
    int             importance;      /* 1-10, affects if it goes to newspapers */

    /* Who was involved */
    char           *primary_actor;   /* Main character (killer, writer, etc) */
    char           *secondary_actor; /* Secondary character (victim, mentioned) */
    char           *tertiary_actor;  /* Third character if relevant */

    /* What happened */
    char           *description;     /* Human-readable description */
    char           *location;        /* Where it happened (area/room name) */

    /* Additional context */
    char           *context_data;    /* JSON-like string with extra data */

    /* Cross-references */
    int             book_vnum;       /* If related to a book */
    int             mob_vnum;        /* If related to a mob */
    int             obj_vnum;        /* If related to an object */
    int             room_vnum;       /* If related to a room */

    bool            is_player_event; /* Was a player involved? */
    bool            is_public;       /* Should this be public knowledge? */
    bool            archived;        /* Moved to long-term storage */
};

/* Character reference - for quick lookups */
typedef struct char_event_ref CHAR_EVENT_REF;
struct char_event_ref
{
    CHAR_EVENT_REF *next;
    char           *character_name;
    HISTORY_EVENT  *events[1000];   /* Array of events involving this character */
    int             num_events;
};

/* Book reference - for tracking mentions */
typedef struct book_reference BOOK_REFERENCE;
struct book_reference
{
    BOOK_REFERENCE *next;
    int             book_vnum;
    char           *book_title;
    char           *mentioned_characters[100]; /* Who's mentioned in this book */
    int             num_mentions;
    HISTORY_EVENT  *creation_event; /* When/how book was written */
};

/* Kill tracker - specific tracking for combat deaths */
typedef struct kill_tracker KILL_TRACKER;
struct kill_tracker
{
    KILL_TRACKER   *next;
    char           *killer_name;
    char           *victim_name;
    time_t          when;
    char           *how;            /* Method of death */
    char           *where;          /* Location */
    int             killer_level;
    int             victim_level;
    bool            was_player_kill;
};

/* Global history system */
typedef struct world_history WORLD_HISTORY;
struct world_history
{
    HISTORY_EVENT     *first_event;
    HISTORY_EVENT     *last_event;
    int                total_events;

    CHAR_EVENT_REF    *first_char_ref;
    int                total_char_refs;

    BOOK_REFERENCE    *first_book;
    int                total_books;

    KILL_TRACKER      *first_kill;
    int                total_kills;

    /* Statistics */
    int                events_today;
    int                events_this_week;
    int                player_actions_tracked;
    int                mob_creations_tracked;

    /* Performance */
    time_t             last_save;
    time_t             last_cleanup;
};

/*****************************************************************************
 * Global Variables
 *****************************************************************************/

extern WORLD_HISTORY *global_history;

/*****************************************************************************
 * Core Functions
 *****************************************************************************/

/* Initialization */
void init_world_history(void);
void load_world_history(void);
void save_world_history(void);

/* Event recording */
void record_kill_event(char *killer, char *victim, char *method, char *location);
void record_book_mention(int book_vnum, char *book_title, char *mentioned_character);
void record_book_written(char *author, char *title, int book_vnum, char *content);
void record_player_action(char *player, char *action, int importance);
void record_mob_creation(char *creator, char *created, char *what_created);
void record_area_creation(char *creator, char *area_name, char *reason);
void record_world_event(EVENT_TYPE type, char *description, int importance);

/* Generic event recording */
HISTORY_EVENT *create_history_event(EVENT_TYPE type, char *primary, char *secondary,
                                     char *description, int importance);
void add_event_to_history(HISTORY_EVENT *event);

/* Queries - "who killed X?" */
HISTORY_EVENT **get_events_for_character(char *character_name, int *num_events);
HISTORY_EVENT **get_kills_by_character(char *killer_name, int *num_kills);
HISTORY_EVENT **get_deaths_of_character(char *victim_name, int *num_deaths);

/* Queries - "who's in book X?" */
BOOK_REFERENCE *get_book_by_vnum(int vnum);
BOOK_REFERENCE *get_book_by_title(char *title);
char **get_characters_in_book(int book_vnum, int *num_chars);

/* Queries - "what did player X do?" */
HISTORY_EVENT **get_player_actions(char *player_name, int *num_actions);
HISTORY_EVENT **get_recent_events(int num_events);
HISTORY_EVENT **get_events_by_type(EVENT_TYPE type, int *num_events);

/* Queries - "who killed the most?" */
KILL_TRACKER **get_top_killers(int limit);
KILL_TRACKER **get_kills_in_area(char *area_name, int *num_kills);

/* Character references */
CHAR_EVENT_REF *get_or_create_char_ref(char *character_name);
void add_event_to_char_ref(char *character_name, HISTORY_EVENT *event);

/* Book tracking */
BOOK_REFERENCE *create_book_reference(int vnum, char *title);
void add_mention_to_book(int book_vnum, char *character_name);

/* Maintenance */
void cleanup_old_events(void);
void archive_ancient_events(void);
void rebuild_event_indices(void);

/* Display functions (for immortals) */
void show_character_history(CHAR_DATA *ch, char *character_name);
void show_book_mentions(CHAR_DATA *ch, int book_vnum);
void show_kill_history(CHAR_DATA *ch, char *character_name);
void show_recent_world_events(CHAR_DATA *ch, int num_events);
void show_player_impact(CHAR_DATA *ch, char *player_name);

/* Statistics */
void show_history_statistics(CHAR_DATA *ch);
int get_character_importance_score(char *character_name);
int get_event_count_for_character(char *character_name);

/* Persistence */
void save_events_to_file(void);
void load_events_from_file(void);
void save_book_references(void);
void load_book_references(void);
void save_kill_tracker(void);
void load_kill_tracker(void);

#endif /* WORLD_HISTORY_TRACKER_H */
