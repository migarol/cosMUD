/****************************************************************************
 * Book Writing System - Mobs write books, poems, lore
 * Integrates with world_history_tracker for mentions
 ****************************************************************************/

#ifndef BOOK_WRITING_SYSTEM_H
#define BOOK_WRITING_SYSTEM_H

/* Book types */
typedef enum {
    BOOK_TYPE_HISTORY,    /* Historical account */
    BOOK_TYPE_POETRY,     /* Poems and verses */
    BOOK_TYPE_LORE,       /* World lore */
    BOOK_TYPE_BIOGRAPHY,  /* Life story */
    BOOK_TYPE_MANUAL,     /* How-to guide */
    BOOK_TYPE_FICTION     /* Stories */
} BOOK_TYPE;

/* Functions */
void init_book_writing_system(void);
void mob_write_book(CHAR_DATA *author, int book_type, char *subject);
void mob_write_poem(CHAR_DATA *poet, char *subject);
char *generate_book_content(int book_type, char *subject, char *author_name, char *author_profession);
char *generate_poem_content(CHAR_DATA *poet, char *subject);
OBJ_DATA *create_book_object(char *title, char *author, char *content, int *assigned_vnum);
void book_mention_character(OBJ_DATA *book, char *character_name);
void book_writing_update(void);
void queue_book_writing(CHAR_DATA *author, int book_type, char *subject);

#endif
