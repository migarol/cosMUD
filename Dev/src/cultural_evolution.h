/****************************************************************************
 * Cultural Evolution - Areas develop unique cultures, traditions, festivals
 ****************************************************************************/

#ifndef CULTURAL_EVOLUTION_H
#define CULTURAL_EVOLUTION_H

typedef struct area_culture AREA_CULTURE;
struct area_culture {
    AREA_DATA *area;

    /* Core identity */
    int primary_culture_type;      /* CULTURE_MEDIEVAL, etc */
    char *cultural_name;           /* "The Coastal Folk", "Mountain Clans" */
    char *cultural_identity;       /* AI-generated description */

    /* Values and beliefs */
    int values[10];                /* What this culture values */
    char *core_belief;             /* Primary belief system */

    /* Traditions */
    int num_traditions;
    struct tradition {
        char *name;
        char *description;
        int age_in_years;          /* How old is this tradition */
        int importance;            /* 1-10 */
    } *traditions[20];

    /* Festivals */
    int num_festivals;
    struct festival {
        char *name;
        char *description;
        int month;                 /* When celebrated */
        int day;
        bool active;
    } *festivals[10];

    /* Cultural practices */
    char *greeting_style;          /* How people greet */
    char *architecture_style;      /* Building style */
    char *art_style;               /* Artistic preferences */
    char *music_style;             /* Musical preferences */
    char *food_culture;            /* Culinary traditions */

    /* Cultural drift */
    int similarity_to_neighbors;   /* 0-100 */
    int cultural_isolation;        /* 0-100 (higher = more unique) */

    /* Evolution tracking */
    time_t last_evolution;
    int evolution_rate;            /* How fast culture changes */
    int cultural_age;

    AREA_CULTURE *next;
};

void init_cultural_evolution(void);
void area_develop_culture(AREA_DATA *area);
void create_tradition(AREA_CULTURE *culture, char *name, char *description, int importance);
void create_festival(AREA_CULTURE *culture, char *name, char *description, int month, int day);
void evolve_culture(AREA_CULTURE *culture);
void cultural_evolution_update(void);
AREA_CULTURE *find_or_create_culture(AREA_DATA *area);

#endif
