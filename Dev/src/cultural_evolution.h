/****************************************************************************
 * Cultural Evolution - Areas develop unique cultures, traditions, festivals
 ****************************************************************************/

#ifndef CULTURAL_EVOLUTION_H
#define CULTURAL_EVOLUTION_H

typedef struct area_culture {
    AREA_DATA *area;
    char *cultural_identity;
    char *traditions[20];
    int num_traditions;
    char *festivals[10];
    int num_festivals;
    int cultural_age;
} AREA_CULTURE;

void init_cultural_evolution(void);
void area_develop_culture(AREA_DATA *area);
void create_tradition(AREA_DATA *area, char *tradition);
void cultural_evolution_update(void);

#endif
