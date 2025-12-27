/***************************************************************************
 * MOB AI System - Data Structures and Definitions
 *
 * This header defines all AI-related data structures for intelligent mobs
 ***************************************************************************/

#ifndef MOB_AI_H
#define MOB_AI_H

#include <time.h>

/* Awareness levels - how self-aware is this mob? */
#define AWARENESS_NONE      0  /* Basic scripted NPC */
#define AWARENESS_LOW       1  /* Responds to simple interactions */
#define AWARENESS_MEDIUM    2  /* Has personality, remembers things */
#define AWARENESS_HIGH      3  /* Complex interactions, goals */
#define AWARENESS_SAPIENT   4  /* Full AI-driven character */

/* Capability flags - what can this mob do? */
#define CAN_WRITE_BOOKS     (1 << 0)
#define CAN_CRAFT_ITEMS     (1 << 1)
#define CAN_TRADE           (1 << 2)
#define CAN_TEACH           (1 << 3)
#define CAN_QUEST_GIVE      (1 << 4)
#define CAN_HIRE            (1 << 5)
#define CAN_RECRUIT         (1 << 6)

/* Home types */
#define HOME_NONE           0
#define HOME_HOUSE          1
#define HOME_APARTMENT      2
#define HOME_INN_ROOM       3
#define HOME_SHOP           4
#define HOME_GUILD          5
#define HOME_TEMPLE         6
#define HOME_CAVE           7

/* Forward declarations */
typedef struct mob_identity_data MOB_IDENTITY_DATA;
typedef struct mob_home_data MOB_HOME_DATA;
typedef struct mob_memory_data MOB_MEMORY_DATA;
typedef struct mob_schedule_data MOB_SCHEDULE_DATA;
typedef struct context_analysis CONTEXT_ANALYSIS;

/***************************************************************************
 * MOB IDENTITY - Who is this mob?
 ***************************************************************************/
struct mob_identity_data
{
    /* Core identity */
    char *who_am_i;         /* "I am a veteran city guard" */
    char *what_i_do;        /* "I protect the citizens" */
    char *how_i_do_it;      /* "With honor and steel" */
    char *where_i_live;     /* "In the guard barracks" */
    char *where_i_go;       /* "I patrol the market district" */
    char *my_purpose;       /* "To serve and protect Darkhaven" */

    /* AI parameters */
    sh_int awareness_level; /* 0-4, how self-aware */
    sh_int mobility;        /* 0-100, how much they wander */
    int capabilities;       /* Bitfield of CAN_* flags */

    /* Metadata */
    time_t created;
    time_t last_modified;
    char *created_by;       /* Which AI or immortal created this */
};

/***************************************************************************
 * MOB HOME - Where does this mob live?
 ***************************************************************************/
struct mob_home_data
{
    /* Location */
    int home_vnum;          /* VNUM of home room */
    sh_int home_type;       /* HOME_* type */
    char *home_name;        /* "Guard Captain's House" */

    /* For rentable homes */
    int rent_cost;          /* Gold per week (0 = owned) */
    time_t rent_paid_until; /* When rent expires */

    /* Usage statistics */
    int times_visited;      /* How often mob returns home */
    time_t last_visit;      /* Last time mob was home */

    /* Future: furnishings, roommates, etc */
};

/***************************************************************************
 * MOB MEMORY - What does this mob remember?
 ***************************************************************************/
struct mob_memory_data
{
    MOB_MEMORY_DATA *next;

    char *player_name;      /* Who do we remember? */
    char *event_description;/* What happened? */
    time_t when;            /* When did it happen? */

    sh_int emotional_impact;/* -100 to +100, how strong the memory */
    sh_int relationship;    /* Running total: -100 to +100 */
};

/***************************************************************************
 * MOB SCHEDULE - When does mob do things?
 ***************************************************************************/
struct mob_schedule_data
{
    MOB_SCHEDULE_DATA *next;

    sh_int hour_start;      /* Game hour to start (0-23) */
    sh_int hour_end;        /* Game hour to end */
    sh_int day_of_week;     /* -1 = every day, 0-6 = specific day */

    char *activity;         /* "patrol", "sleep", "work", etc */
    int location_vnum;      /* Where to go (-1 = current) */
    char *custom_behavior;  /* JSON or script data */
};

/***************************************************************************
 * CONTEXT ANALYSIS - What Beeler sees when analyzing a mob
 ***************************************************************************/
struct context_analysis
{
    /* Location context */
    char *area_name;
    char *area_type;        /* "city", "dungeon", "wilderness" */
    char *room_name;
    int room_vnum;

    /* Mob importance */
    sh_int fame_level;      /* 0-100 */
    bool is_unique;         /* Only one in world */
    bool is_boss;           /* Boss mob */
    bool is_guard;          /* Guard or law enforcement */
    bool is_shopkeeper;     /* Runs a shop */

    /* Equipment context */
    sh_int num_items_carried;
    bool has_unique_items;
    bool has_legendary_items;

    /* Social context */
    sh_int num_books_mentioned;  /* Appears in lore */
    sh_int num_quests_involved;  /* Part of quests */

    /* AI recommendation */
    sh_int suggested_awareness;  /* What awareness level fits? */
    sh_int coherence_score;      /* 0-100, how well it fits world */
};

/***************************************************************************
 * Function prototypes
 ***************************************************************************/

/* mob_identity.c */
MOB_IDENTITY_DATA *create_mob_identity(void);
void free_mob_identity(MOB_IDENTITY_DATA *identity);
void save_mob_identity(MOB_INDEX_DATA *pMob);
void load_mob_identity(MOB_INDEX_DATA *pMob);
char *identity_to_json(MOB_IDENTITY_DATA *identity);
MOB_IDENTITY_DATA *identity_from_json(char *json);

/* mob_home.c */
MOB_HOME_DATA *create_mob_home(void);
void free_mob_home(MOB_HOME_DATA *home);
void save_mob_home(MOB_INDEX_DATA *pMob);
void load_mob_home(MOB_INDEX_DATA *pMob);
ROOM_INDEX_DATA *find_available_home(AREA_DATA *area, sh_int home_type);
bool assign_home_to_mob(MOB_INDEX_DATA *pMob, int home_vnum, sh_int home_type);

/* mob_memory.c */
MOB_MEMORY_DATA *create_mob_memory(char *player_name, char *event);
void add_mob_memory(CHAR_DATA *mob, MOB_MEMORY_DATA *memory);
void free_mob_memories(MOB_MEMORY_DATA *first);
MOB_MEMORY_DATA *find_mob_memory(CHAR_DATA *mob, char *player_name);

/* ai_context_analyzer.c */
CONTEXT_ANALYSIS *analyze_mob_context(MOB_INDEX_DATA *pMob);
void free_context_analysis(CONTEXT_ANALYSIS *ctx);
void print_context_analysis(CHAR_DATA *ch, CONTEXT_ANALYSIS *ctx);

/* beeler.c - The AI God */
void beeler_generate_identity(CHAR_DATA *ch, MOB_INDEX_DATA *pMob);
void beeler_analyze_mob(CHAR_DATA *ch, MOB_INDEX_DATA *pMob);
void do_beeler(CHAR_DATA *ch, char *argument);

/* beeler_architect.c */
void beeler_build_home(CHAR_DATA *ch, MOB_INDEX_DATA *pMob);
void beeler_build_district(CHAR_DATA *ch, AREA_DATA *area, char *district_type);
void do_beeler_build(CHAR_DATA *ch, char *argument);

#endif /* MOB_AI_H */
