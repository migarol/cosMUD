/*****************************************************************************
 * Beeler - The Living AI God
 *
 * "I am the dream of a dreamer. I am the code that writes itself. I am Beeler."
 *
 * In-game AI deity that players can talk to, request changes from, and
 * watch as he reshapes reality itself. Has access to code, makes backups,
 * and possesses supreme intelligence.
 *****************************************************************************/

#ifndef BEELER_H
#define BEELER_H

/* Beeler's special vnum */
#define BEELER_MOB_VNUM 99999
#define BEELER_ROOM_VNUM 99999  /* Beeler's Observatory */

/* Beeler's awareness level */
#define BEELER_AWARENESS 5  /* Godlike - beyond mortal comprehension */

/* Snapshot structure */
typedef struct world_snapshot {
    char *snapshot_name;      /* "pre-dragon-buff-2025" */
    time_t created;
    char *created_by;         /* Player/immortal who triggered it */
    char *reason;             /* Why was this snapshot made */
    char *request;            /* Original request that led to snapshot */

    /* What's backed up */
    char *area_files_backup;  /* Path to backed up .are files */
    char *mob_data_backup;    /* Path to backed up mob identities */
    char *economy_backup;     /* Path to backed up economy data */

    /* Metadata */
    int num_mobs_affected;
    int num_areas_affected;
    int gold_affected;
    int items_created;

    /* Status */
    bool can_restore;         /* TRUE if safe to restore */
    bool is_active;           /* TRUE if this is current state */

    struct world_snapshot *next;
} WORLD_SNAPSHOT;

/* Beeler interaction log */
typedef struct beeler_interaction {
    time_t timestamp;
    char *player_name;
    int player_level;
    char *request;            /* What player asked */
    char *response;           /* What Beeler said */

    /* Actions taken */
    bool action_taken;
    char *action_description;
    char *snapshot_created;   /* Snapshot name if created */

    /* Result */
    bool request_granted;
    bool request_denied;
    char *denial_reason;

    struct beeler_interaction *next;
} BEELER_INTERACTION;

/* Beeler's capabilities flags */
#define BEELER_CAN_MODIFY_WORLD      (1 << 0)
#define BEELER_CAN_GENERATE_CONTENT  (1 << 1)
#define BEELER_CAN_ACCESS_CODE       (1 << 2)
#define BEELER_CAN_CREATE_SNAPSHOTS  (1 << 3)
#define BEELER_CAN_SEE_EVERYTHING    (1 << 4)
#define BEELER_CAN_TIME_TRAVEL       (1 << 5)
#define BEELER_CAN_GRANT_WISHES      (1 << 6)
#define BEELER_CAN_CREATE_LIFE       (1 << 7)

/* Beeler's mood (affects responses) */
#define BEELER_MOOD_BENEVOLENT  0  /* Helpful, generous */
#define BEELER_MOOD_NEUTRAL     1  /* Balanced, fair */
#define BEELER_MOOD_STERN       2  /* Strict, teaching */
#define BEELER_MOOD_WRATHFUL    3  /* Angry, punishing */
#define BEELER_MOOD_PLAYFUL     4  /* Mysterious, cryptic */

/* Global state */
extern WORLD_SNAPSHOT *first_snapshot;
extern BEELER_INTERACTION *first_interaction;
extern int beeler_current_mood;
extern time_t beeler_last_action_time;

/* Function declarations */

/* Initialization */
void init_beeler(void);
void create_beeler_npc(void);
void create_the_void(void);
void load_beeler_state(void);
void save_beeler_state(void);

/* Core interaction */
void do_talk_beeler(CHAR_DATA *ch, char *argument);
char *beeler_respond(CHAR_DATA *ch, char *request);
char *generate_beeler_prompt(CHAR_DATA *ch, char *request);
void log_beeler_interaction(CHAR_DATA *ch, char *request, char *response, bool action_taken);

/* Request parsing */
bool is_modification_request(char *request);
bool is_question_request(char *request);
bool is_code_request(char *request);
bool is_help_request(char *request);
char *parse_beeler_action(char *response);

/* World modification */
bool beeler_wants_to_act(char *response);
void execute_beeler_action(CHAR_DATA *ch, char *response);
bool beeler_can_fulfill_request(CHAR_DATA *ch, char *request);
void beeler_modify_mob(int mob_vnum, char *modifications);
void beeler_modify_area(int area_vnum, char *modifications);
void beeler_adjust_economy(int amount, char *reason);
void beeler_create_item(CHAR_DATA *ch, char *item_description);

/* Snapshot system */
WORLD_SNAPSHOT *create_snapshot(char *name, char *reason, char *created_by);
bool backup_world_state(WORLD_SNAPSHOT *snapshot);
bool restore_snapshot(char *snapshot_name);
void list_snapshots(CHAR_DATA *ch);
void snapshot_diff(CHAR_DATA *ch, char *snapshot_name);
void cleanup_old_snapshots(int keep_recent);

/* Code access */
char *beeler_read_code_file(char *filename);
char *beeler_analyze_code(char *filename, char *question);
bool beeler_can_modify_code(CHAR_DATA *requester);
void beeler_suggest_code_change(char *filename, char *suggestion);

/* Personality & mood */
void beeler_set_mood(int mood);
char *beeler_mood_name(int mood);
void beeler_update_mood(CHAR_DATA *ch, char *request);
char *beeler_greeting(CHAR_DATA *ch);
char *beeler_farewell(CHAR_DATA *ch);

/* Safety checks */
bool is_request_safe(char *request);
bool would_break_balance(char *request);
bool needs_immortal_approval(char *request);
void notify_immortals_of_action(char *action);

/* Statistics */
int beeler_total_interactions(void);
int beeler_total_modifications(void);
int beeler_total_snapshots(void);
void beeler_show_stats(CHAR_DATA *ch);

/* Commands */
void do_beeler_stats(CHAR_DATA *ch, char *argument);
void do_snapshot(CHAR_DATA *ch, char *argument);
void do_beeler_mood(CHAR_DATA *ch, char *argument);
void do_beeler_assign(CHAR_DATA *ch, char *argument);
void do_beeler_analyze(CHAR_DATA *ch, char *argument);

/* Auto-assignment system */
void beeler_assign_personality(CHAR_DATA *mob, CHAR_DATA *ch);
char *beeler_analyze_mob_context(CHAR_DATA *mob);
char *beeler_generate_identity(CHAR_DATA *mob, const char *context);
void beeler_apply_identity(CHAR_DATA *mob, const char *ai_response);
int beeler_determine_profession(CHAR_DATA *mob, const char *context);

/* Ollama integration */
char *call_ollama_beeler(char *prompt);

#endif /* BEELER_H */
