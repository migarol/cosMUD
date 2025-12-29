/****************************************************************************
 * Family Lineage - Mobs have families, children, lineages
 ****************************************************************************/

#ifndef FAMILY_LINEAGE_H
#define FAMILY_LINEAGE_H

/* Family member record */
typedef struct family_member FAMILY_MEMBER;
struct family_member {
    int mob_vnum;
    char *name;
    CHAR_DATA *mob_ptr;         /* NULL if not currently loaded */

    /* Relationships */
    FAMILY_MEMBER *father;
    FAMILY_MEMBER *mother;
    FAMILY_MEMBER *spouse;
    FAMILY_MEMBER **children;
    int num_children;

    /* Lineage */
    int generation;             /* 1 = founder, 2 = children, etc */
    char *family_name;          /* "House Aldric", "Smith Family" */
    char *bloodline;            /* "Royal Line of DarkHaven" */

    /* Traits inherited */
    int inherited_strength;
    int inherited_intelligence;
    int inherited_charisma;
    char *inherited_profession;

    /* Life events */
    time_t born;
    time_t died;                /* 0 if still alive */
    bool is_alive;

    FAMILY_MEMBER *next_in_family;
    FAMILY_MEMBER *next_global;
};

/* Family tree */
typedef struct family_tree FAMILY_TREE;
struct family_tree {
    char *family_name;
    FAMILY_MEMBER *founder;     /* First generation */
    int total_members;
    int current_generation;
    FAMILY_TREE *next;
};

void init_family_lineage(void);
void load_families(void);
void save_families(void);
FAMILY_MEMBER *create_family_member(char *name, int mob_vnum);
void record_birth(FAMILY_MEMBER *parent1, FAMILY_MEMBER *parent2, CHAR_DATA *child_mob);
void record_death(FAMILY_MEMBER *member);
void establish_dynasty(char *dynasty_name, FAMILY_MEMBER *founder);
FAMILY_TREE *find_family(char *family_name);
void family_lineage_update(void);

#endif
