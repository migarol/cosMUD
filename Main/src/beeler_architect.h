/*****************************************************************************
 * Beeler - World Architect System
 *
 * Beeler can CREATE rooms, modify areas, and build residential districts.
 * He doesn't just assign homes - he BUILDS them.
 *
 * "I am not merely an observer. I am the builder of worlds."
 *****************************************************************************/

#ifndef BEELER_ARCHITECT_H
#define BEELER_ARCHITECT_H

/* Area modification capabilities */
typedef struct beeler_area_mod {
    int area_vnum;
    char *area_name;
    char *modification_type;  /* "add_room", "add_district", "add_inn" */

    /* Rooms to add */
    int num_rooms_to_add;
    int *new_room_vnums;

    /* Backup info */
    char *original_area_file;
    char *snapshot_name;

    time_t created;
    bool applied;

    struct beeler_area_mod *next;
} BEELER_AREA_MOD;

/* Room generation data */
typedef struct room_generation {
    int vnum;
    char *name;
    char *description;
    int sector_type;
    int room_flags;

    /* Exits */
    int num_exits;
    int *exit_dirs;      /* Direction (NORTH, SOUTH, etc) */
    int *exit_to_vnums;  /* Where they lead */

    /* Extra descriptions */
    int num_extras;
    char **extra_keywords;
    char **extra_descriptions;

    /* AI-generated content */
    bool ai_generated;
    char *generation_prompt;

} ROOM_GENERATION;

/* Residential district generation */
typedef struct district_generation {
    char *district_name;
    int area_vnum;
    int starting_vnum;   /* First room vnum */
    int num_rooms;       /* How many rooms in district */

    /* District layout */
    char *layout_type;   /* "grid", "street", "circular" */
    int home_type;       /* HOME_TYPE_* */

    /* Generated rooms */
    ROOM_GENERATION **rooms;

    /* Connection point */
    int connect_to_vnum; /* Existing room to connect to */
    int connect_dir;     /* Direction of connection */

} DISTRICT_GENERATION;

/* Context engine for fast lookups */
typedef struct beeler_context {
    /* World state cache */
    int total_mobs;
    int total_areas;
    int total_rooms;
    int total_players_online;

    /* Quick lookups */
    void *mob_index_hash;      /* Fast mob lookup */
    void *room_index_hash;     /* Fast room lookup */
    void *area_index_hash;     /* Fast area lookup */

    /* Recent activity */
    char *recent_kills[10];
    char *recent_deaths[10];
    char *recent_logins[10];

    /* Economic data */
    int total_gold_in_world;
    int avg_player_gold;

    /* Cached queries */
    time_t last_cache_update;

} BEELER_CONTEXT;

/* Function declarations */

/* Initialization */
void init_beeler_architect(void);
void beeler_scan_all_areas(void);
void beeler_build_context_cache(void);

/* Area analysis */
int beeler_find_available_vnums(int area_vnum, int count);
bool beeler_can_add_rooms_to_area(int area_vnum, int count);
int beeler_find_connection_point(int area_vnum, char *district_type);
AREA_DATA *beeler_find_area_by_name(char *name);

/* Room generation */
ROOM_GENERATION *beeler_generate_room(int vnum, int home_type, char *mob_name);
char *beeler_generate_room_description(int home_type, char *mob_name, char *mob_personality);
void beeler_add_room_to_area(int area_vnum, ROOM_GENERATION *room);
bool beeler_create_exit(int from_vnum, int to_vnum, int direction);

/* District generation */
DISTRICT_GENERATION *beeler_generate_residential_district(char *area_name, int home_type, int num_homes);
bool beeler_build_district(DISTRICT_GENERATION *district);
void beeler_connect_district_to_city(DISTRICT_GENERATION *district, int city_vnum);

/* Inn generation */
void beeler_generate_inn(char *area_name, char *inn_name, int num_rooms);
ROOM_GENERATION *beeler_generate_inn_room(int vnum, char *inn_name, int room_number);
ROOM_GENERATION *beeler_generate_inn_common_room(int vnum, char *inn_name);

/* Area file modification */
bool beeler_read_area_file(char *filename, char **content);
bool beeler_write_area_file(char *filename, char *content);
bool beeler_insert_room_in_area_file(char *area_file, ROOM_GENERATION *room);
bool beeler_backup_area_file(char *filename);

/* Smart assignment */
int beeler_assign_home_intelligently(CHAR_DATA *mob);
char *beeler_determine_home_city(CHAR_DATA *mob);
int beeler_find_or_create_home(CHAR_DATA *mob, char *city_name, int home_type);

/* Context tools */
void beeler_update_context(void);
char *beeler_quick_stat(char *query);  /* "mobs in darkhaven", "gold in economy" */
CHAR_DATA *beeler_find_mob_by_name(char *name);
ROOM_INDEX_DATA *beeler_find_room_by_name(char *name);
int beeler_count_mobs_in_area(int area_vnum);

/* Immortal command interface */
void beeler_execute_immortal_command(char *command, char *arguments);
bool beeler_has_permission_for(char *command);
char *beeler_analyze_command_result(char *command, char *output);

/* AI-powered generation */
char *beeler_ai_generate_district(char *city_name, int home_type, int num_homes);
char *beeler_ai_generate_inn(char *location, char *inn_style);
char *beeler_ai_generate_room_desc(char *room_type, char *owner_name, char *context);

/* Safety checks */
bool beeler_vnum_range_safe(int start_vnum, int count);
bool beeler_modification_safe(BEELER_AREA_MOD *mod);
void beeler_validate_generated_area(DISTRICT_GENERATION *district);

/* Persistence */
void save_beeler_modifications(void);
void load_beeler_modifications(void);

/* Commands */
void do_beeler_build(CHAR_DATA *ch, char *argument);
void do_beeler_analyze(CHAR_DATA *ch, char *argument);
void do_beeler_context(CHAR_DATA *ch, char *argument);

#endif /* BEELER_ARCHITECT_H */
