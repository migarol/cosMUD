/*****************************************************************************
 * Mob Identity & Self-Awareness System
 *
 * Each mob knows WHO they are, WHAT they do, WHERE they are, WHERE they go.
 * Persistent memory of players, other mobs, conversations, creations.
 *****************************************************************************/

#ifndef MOB_IDENTITY_H
#define MOB_IDENTITY_H

/* Mob capability flags */
#define MOB_CAN_WRITE_BOOKS     (1 << 0)  /* Can author books */
#define MOB_CAN_CRAFT_ITEMS     (1 << 1)  /* Can create items */
#define MOB_CAN_TRADE           (1 << 2)  /* Can buy/sell dynamically */
#define MOB_CAN_BUILD           (1 << 3)  /* Can construct buildings */
#define MOB_CAN_TEACH           (1 << 4)  /* Can teach skills */
#define MOB_CAN_LEAD            (1 << 5)  /* Can lead groups/factions */

/* Mob self-awareness level */
#define AWARENESS_NONE          0   /* No self-awareness (animals) */
#define AWARENESS_BASIC         1   /* Basic awareness (commoners) */
#define AWARENESS_MODERATE      2   /* Moderate awareness (merchants) */
#define AWARENESS_HIGH          3   /* High awareness (scholars) */
#define AWARENESS_FULL          4   /* Full awareness (leaders, heroes) */

/* Memory types */
#define MEMORY_MET_PLAYER       0
#define MEMORY_MET_MOB          1
#define MEMORY_CONVERSATION     2
#define MEMORY_BOOK_WRITTEN     3
#define MEMORY_ITEM_CRAFTED     4
#define MEMORY_PLACE_VISITED    5
#define MEMORY_EVENT_WITNESSED  6
#define MEMORY_PROMISE_MADE     7
#define MEMORY_PROMISE_KEPT     8
#define MEMORY_PROMISE_BROKEN   9

/* Mob Identity Structure */
typedef struct mob_identity_data {
    int mob_vnum;

    /* Self-awareness: WHO AM I? */
    char *who_am_i;           /* "Tsythia, erudito de Darkhaven" */
    char *what_i_do;          /* "Investigo textos antiguos" */
    char *how_i_do_it;        /* "De noche, meticulosamente, solo" */
    char *where_i_live;       /* "Biblioteca de Darkhaven" */
    char *where_i_go;         /* "A mi estudio, rara vez salgo" */
    char *my_purpose;         /* "Preservar conocimiento antiguo" */

    /* Capabilities */
    int capabilities;         /* Bitfield of MOB_CAN_* flags */
    int awareness_level;      /* 0-4 awareness level */
    int mobility;             /* 0-100 how much this mob moves around */

    /* Writing abilities */
    char *writing_style;      /* "académico, pedante" */
    int books_written;        /* Count of books authored */

    /* Crafting abilities */
    char *craft_specialty;    /* "espadas", "pociones", etc */
    int items_crafted;        /* Count of items created */

    /* Trading */
    int trade_skill;          /* 0-100 haggling skill */
    int gold_earned;          /* Total gold from trading */

    /* Personality AI prompt */
    char *ai_prompt;          /* Full AI personality prompt */

    /* Custom schedule (overrides default) */
    bool has_custom_schedule;
    void *custom_schedule;    /* Pointer to CUSTOM_SCHEDULE */

    struct mob_identity_data *next;
} MOB_IDENTITY;

/* Custom schedule entry */
typedef struct schedule_entry {
    int start_hour;           /* 0-23 */
    int end_hour;             /* 0-23 */
    char *activity;           /* "WRITE", "SLEEP", "PATROL", etc */
    char *location;           /* "library", "study", "quarters" */
    int location_vnum;        /* Optional specific room */
    struct schedule_entry *next;
    struct schedule_entry *prev;
} SCHEDULE_ENTRY;

typedef struct custom_schedule {
    SCHEDULE_ENTRY *first_entry;
    SCHEDULE_ENTRY *last_entry;
} CUSTOM_SCHEDULE;

/* Memory structure - persistent across boots */
typedef struct mob_memory_entry {
    int memory_type;          /* MEMORY_* type */
    time_t timestamp;         /* When this happened */

    /* Who/what */
    char *target_name;        /* Player or mob name */
    int target_vnum;          /* Mob vnum if applicable */

    /* Details */
    char *summary;            /* AI-generated summary */
    char *full_text;          /* Full conversation/details */

    /* Emotional impact */
    int emotional_impact;     /* -100 to +100 */
    int relationship_change;  /* How much relationship changed */

    /* Promises/commitments */
    bool is_promise;
    bool promise_kept;
    time_t promise_deadline;

    /* Location */
    int room_vnum;            /* Where this happened */

    struct mob_memory_entry *next;
    struct mob_memory_entry *prev;
} MEMORY_ENTRY;

typedef struct mob_memory_data {
    int mob_vnum;
    int total_memories;
    MEMORY_ENTRY *first_memory;
    MEMORY_ENTRY *last_memory;
    struct mob_memory_data *next;
} MOB_MEMORY;

/* Creation tracking */
typedef struct mob_creation {
    int creator_vnum;         /* Mob who created it */
    int object_vnum;          /* What was created */
    time_t created_when;      /* When */
    char *creation_name;      /* Name of creation */
    char *creation_desc;      /* AI-generated description */
    bool is_book;             /* TRUE if book, FALSE if item */
    struct mob_creation *next;
} MOB_CREATION;

/* Global lists */
extern MOB_IDENTITY *first_mob_identity;
extern MOB_MEMORY *first_mob_memory;
extern MOB_CREATION *first_mob_creation;

/* Function declarations */
void init_mob_identity_system(void);
MOB_IDENTITY *create_mob_identity(int vnum);
MOB_IDENTITY *get_mob_identity(int vnum);
void save_mob_identity(MOB_IDENTITY *identity);
void load_mob_identity(int vnum);

MOB_MEMORY *get_mob_memory(int vnum);
void add_mob_memory(int vnum, int type, char *target, char *summary, int emotional_impact);
MEMORY_ENTRY *find_memory_about(int vnum, char *target_name);
void save_mob_memory(int vnum);
void load_mob_memory(int vnum);

void mob_write_book(CHAR_DATA *mob, char *topic);
void mob_craft_item(CHAR_DATA *mob, char *item_type);
void track_mob_creation(int creator_vnum, int object_vnum, char *name, bool is_book);

/* Schedule functions */
CUSTOM_SCHEDULE *create_custom_schedule(void);
void add_schedule_entry(CUSTOM_SCHEDULE *sched, int start, int end, char *activity, char *location);
SCHEDULE_ENTRY *get_current_schedule(CUSTOM_SCHEDULE *sched, int hour);
int get_custom_mob_routine(CHAR_DATA *mob);

/* AI generation functions */
void generate_mob_identity(CHAR_DATA *mob);
void generate_mob_schedule(MOB_IDENTITY *identity);
char *generate_conversation_summary(CHAR_DATA *mob, CHAR_DATA *with, char *conversation);

/* Commands */
void do_mobidentity(CHAR_DATA *ch, char *argument);
void do_mobmemory(CHAR_DATA *ch, char *argument);
void do_genpersonality(CHAR_DATA *ch, char *argument);

#endif /* MOB_IDENTITY_H */
