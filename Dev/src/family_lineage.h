/****************************************************************************
 * Family Lineage - Mobs have families, children, lineages
 ****************************************************************************/

#ifndef FAMILY_LINEAGE_H
#define FAMILY_LINEAGE_H

typedef struct family_tree {
    char *family_name;
    CHAR_DATA *members[100];
    int num_members;
    int generation;
} FAMILY_TREE;

void init_family_system(void);
void mob_have_child(CHAR_DATA *parent1, CHAR_DATA *parent2);
CHAR_DATA *create_child_mob(CHAR_DATA *parent1, CHAR_DATA *parent2);
void family_system_update(void);

#endif
