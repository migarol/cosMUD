/*****************************************************************************
 * Mob Housing System
 *
 * Every sentient mob has a home. Personalized apartments, houses, manors.
 * Inns for travelers. Mobs sleep in their beds, not random rooms.
 *
 * "Home is where the heart is... and where the AI God placed your bed."
 *****************************************************************************/

#ifndef MOB_HOME_H
#define MOB_HOME_H

/* Home types */
#define HOME_TYPE_HOVEL      0  /* Poor, dirty, cramped */
#define HOME_TYPE_APARTMENT  1  /* Basic, clean, functional */
#define HOME_TYPE_HOUSE      2  /* Nice, comfortable, spacious */
#define HOME_TYPE_MANOR      3  /* Luxurious, impressive, grand */
#define HOME_TYPE_PALACE     4  /* Royal, magnificent, legendary */
#define HOME_TYPE_INN_ROOM   5  /* Temporary, rented */
#define HOME_TYPE_BARRACKS   6  /* Military, shared, utilitarian */
#define HOME_TYPE_DORMITORY  7  /* Academic, shared, studious */

/* Mob home structure */
typedef struct mob_home_data {
    int mob_vnum;
    int home_vnum;           /* Room where they live */
    int home_type;           /* HOME_TYPE_* */
    char *home_name;         /* "Tsythia's Study", "Guard Captain's Quarters" */
    char *home_description;  /* AI-generated personalized description */

    /* Ownership */
    bool owned;              /* TRUE if owned, FALSE if renting */
    int rent_cost;           /* Gold per week if renting */
    time_t rent_paid_until;  /* When rent expires */

    /* Furnishings (AI-generated based on personality) */
    int num_furnishings;
    char **furnishing_descriptions;

    /* Location context */
    char *district;          /* "Residential Quarter", "Scholar's Row" */
    char *building_name;     /* "The Ivory Apartments", "Guard Tower" */

    /* Neighbors */
    int num_neighbors;
    int *neighbor_vnums;

    /* Statistics */
    time_t last_visited;     /* Last time mob was home */
    int times_visited;       /* How often they go home */

    struct mob_home_data *next;
} MOB_HOME;

/* Inn structure */
typedef struct inn_data {
    int inn_vnum;            /* Inn's main room vnum */
    char *inn_name;          /* "The Prancing Pony Inn" */
    char *inn_keeper_name;   /* NPC who runs it */
    int inn_keeper_vnum;

    /* Pricing */
    int room_cost_per_night; /* Gold per night */
    int meal_cost;           /* Cost of food/drink */
    int stable_cost;         /* Cost to stable mount */

    /* Rooms */
    int num_rooms;           /* Number of rentable rooms */
    int *room_vnums;         /* Array of room vnums */
    bool *room_occupied;     /* Which rooms are rented */
    int *room_occupants;     /* Mob vnum in each room */
    time_t *checkout_times;  /* When they must leave */

    /* Location */
    bool on_road;            /* TRUE if on major travel route */
    bool in_city;            /* TRUE if in city */
    char *city_name;         /* Which city (if in_city) */
    char *road_name;         /* Which road (if on_road) */

    /* Amenities */
    bool has_stables;
    bool has_tavern;
    bool has_bath;
    bool has_secure_storage;

    /* Reputation */
    int quality;             /* 0-100 how nice it is */
    int safety;              /* 0-100 how safe it is */
    char *reputation;        /* "renowned", "decent", "questionable" */

    struct inn_data *next;
} INN_DATA;

/* Residential district structure */
typedef struct district_data {
    char *district_name;     /* "Noble Quarter", "Slums" */
    int area_vnum;           /* Which area it's in */
    int min_room_vnum;       /* Range of rooms */
    int max_room_vnum;

    /* District type */
    int district_type;       /* HOME_TYPE_* for area quality */
    int avg_rent;            /* Average rent in this district */

    /* Available housing */
    int num_homes;
    int *home_vnums;         /* Rooms that can be homes */
    bool *home_occupied;     /* Which are currently occupied */

    struct district_data *next;
} DISTRICT_DATA;

/* Global lists */
extern MOB_HOME *first_mob_home;
extern INN_DATA *first_inn;
extern DISTRICT_DATA *first_district;

/* Function declarations */

/* Initialization */
void init_housing_system(void);
void load_all_homes(void);
void load_all_inns(void);
void load_all_districts(void);

/* Game loop update */
void housing_system_update(void);  /* Called from update.c */

/* Home management */
MOB_HOME *create_mob_home(int mob_vnum, int home_vnum, int home_type);
MOB_HOME *get_mob_home(int mob_vnum);
void save_mob_home(MOB_HOME *home);
void assign_home_to_mob(CHAR_DATA *mob);
void mob_go_home(CHAR_DATA *mob);
bool is_mob_at_home(CHAR_DATA *mob);

/* Home generation */
char *generate_home_description(CHAR_DATA *mob, int home_type);
char *generate_home_name(CHAR_DATA *mob, int home_type);
void generate_furnishings(MOB_HOME *home, CHAR_DATA *mob);

/* Inn management */
INN_DATA *create_inn(int inn_vnum, char *name);
INN_DATA *get_inn(int inn_vnum);
void save_inn(INN_DATA *inn);
bool inn_has_vacancy(INN_DATA *inn);
int inn_rent_room(INN_DATA *inn, CHAR_DATA *mob, int nights);
void inn_checkout(INN_DATA *inn, CHAR_DATA *mob);
INN_DATA *find_nearest_inn(CHAR_DATA *mob);

/* District management */
DISTRICT_DATA *create_district(char *name, int area_vnum, int type);
DISTRICT_DATA *get_district_for_room(int room_vnum);
int find_available_home_in_district(DISTRICT_DATA *district);

/* Utility */
char *home_type_name(int home_type);
int determine_appropriate_home_type(CHAR_DATA *mob);
int calculate_home_rent(int home_type, DISTRICT_DATA *district);

/* Commands */
void do_home(CHAR_DATA *ch, char *argument);
void do_rent(CHAR_DATA *ch, char *argument);
void do_innlist(CHAR_DATA *ch, char *argument);

/* Admin commands */
void do_homeslist(CHAR_DATA *ch, char *argument);
void do_homeinfo(CHAR_DATA *ch, char *argument);
void do_homeassign(CHAR_DATA *ch, char *argument);
void do_homeunassign(CHAR_DATA *ch, char *argument);

#endif /* MOB_HOME_H */
