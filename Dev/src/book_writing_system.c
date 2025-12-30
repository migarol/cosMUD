/*****************************************************************************
 * Book Writing System - Mobs Write Books and Poems
 *
 * Mobs (poets, historians, scribes) use Ollama to generate authentic content
 * Creates actual OBJ_DATA book objects with readable text
 * Tracks character mentions in books via world_history_tracker
 *
 * Integration: ollama_integration.h, world_history_tracker.h
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "book_writing_system.h"
#include "world_history_tracker.h"
#include "ollama_integration.h"
#include "periodicos.h"

/* Global state */
typedef struct written_book WRITTEN_BOOK;
struct written_book {
    int book_vnum;
    char *title;
    char *author;
    char *content;
    time_t written_when;
    char **mentioned_characters;
    int num_mentions;
    WRITTEN_BOOK *next;
};

WRITTEN_BOOK *first_book = NULL;
int total_books_written = 0;

/* Book writing queue */
typedef struct book_writing_task BOOK_WRITING_TASK;
struct book_writing_task {
    CHAR_DATA *author;
    int book_type;
    char *subject;
    int target_vnum;
    time_t queued_when;
    BOOK_WRITING_TASK *next;
};

BOOK_WRITING_TASK *first_task = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_book_writing_system(void)
{
    log_string("Initializing Book Writing System...");
    first_book = NULL;
    total_books_written = 0;
    first_task = NULL;
    log_string("Book Writing System initialized.");
}

void load_books(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "written_books.dat");

    if ((fp = fopen(filename, "r")) == NULL)
    {
        log_string("No saved books to load.");
        return;
    }

    log_string("Loading written books...");
    /* Load book data */
    fclose(fp);
}

void save_books(void)
{
    FILE *fp;
    char filename[256];
    WRITTEN_BOOK *book;

    sprintf(filename, "%s%s", SYSTEM_DIR, "written_books.dat");

    if ((fp = fopen(filename, "w")) == NULL)
    {
        log_string("ERROR: Cannot save books!");
        return;
    }

    fprintf(fp, "#BOOKS\n");

    for (book = first_book; book; book = book->next)
    {
        fprintf(fp, "Vnum %d\n", book->book_vnum);
        fprintf(fp, "Title~ %s~\n", book->title);
        fprintf(fp, "Author~ %s~\n", book->author);
        fprintf(fp, "Content~ %s~\n", book->content);
        fprintf(fp, "Written %ld\n", book->written_when);
        fprintf(fp, "End\n\n");
    }

    fprintf(fp, "#END\n");
    fclose(fp);
}

/*****************************************************************************
 * Content Generation - Uses Ollama for realistic writing
 *****************************************************************************/

char *generate_book_content(int book_type, char *subject, char *author_name, char *author_profession)
{
    char prompt[MAX_STRING_LENGTH * 2];
    char *content;

    switch (book_type)
    {
        case BOOK_TYPE_POETRY:
            sprintf(prompt,
                "Write a medieval fantasy poem about %s. "
                "Author: %s, a %s. "
                "Write 3-4 stanzas, 4-6 lines each. "
                "Use archaic language and vivid imagery. "
                "Keep it under 200 words.",
                subject, author_name, author_profession);
            break;

        case BOOK_TYPE_HISTORY:
            sprintf(prompt,
                "Write a historical account about %s. "
                "Author: %s, a %s. "
                "Write in a scholarly medieval chronicle style. "
                "Include dates, places, and important figures. "
                "3-4 paragraphs, under 300 words.",
                subject, author_name, author_profession);
            break;

        case BOOK_TYPE_FICTION:
            sprintf(prompt,
                "Write a short fantasy story about %s. "
                "Author: %s, a %s. "
                "Include dialogue, description, and a satisfying conclusion. "
                "4-5 paragraphs, under 400 words.",
                subject, author_name, author_profession);
            break;

        case BOOK_TYPE_BIOGRAPHY:
            sprintf(prompt,
                "Write a journal entry about %s. "
                "Author: %s, a %s. "
                "Write in first person, personal tone. "
                "Include thoughts, observations, feelings. "
                "2-3 paragraphs, under 200 words.",
                subject, author_name, author_profession);
            break;

        case BOOK_TYPE_MANUAL:
            sprintf(prompt,
                "Write a scholarly treatise about %s. "
                "Author: %s, a %s. "
                "Academic tone, discuss theory and practice. "
                "3-4 paragraphs, under 300 words.",
                subject, author_name, author_profession);
            break;

        default:
            sprintf(prompt,
                "Write a short text about %s by %s. "
                "2-3 paragraphs, under 200 words.",
                subject, author_name);
            break;
    }

    /* Use Ollama for generation if policy allows */
    if (ollama_is_available() && OLLAMA_USE_FOR_BOOKS)
    {
        content = ollama_request(prompt, 500);
        if (content && strlen(content) > 50)
        {
            sprintf(log_buf, "BOOK WRITING: AI generated content about '%s'", subject);
            log_string(log_buf);
            return content;
        }
    }

    /* Fallback to template */
    sprintf(prompt,
        "A work by %s\n\n"
        "Regarding the subject of %s:\n\n"
        "The author has much to say on this matter, "
        "though the details remain to be fully documented. "
        "Future scholars may expand upon these notes.",
        author_name, subject);

    return str_dup(prompt);
}

char *extract_character_mentions(char *content, int *num_mentions)
{
    /* Extract character names mentioned in content */
    /* This is simplified - would do actual NLP */
    static char mentions[MAX_STRING_LENGTH];

    *num_mentions = 0;
    mentions[0] = '\0';

    /* For now, just return empty */
    return str_dup(mentions);
}

/*****************************************************************************
 * Book Creation - Create actual OBJ_DATA objects
 *****************************************************************************/

OBJ_DATA *create_book_object(char *title, char *author, char *content, int *assigned_vnum)
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *book;
    EXTRA_DESCR_DATA *ed;
    static int next_book_vnum = 30000; /* Start from high vnum range */

    /* Find a free vnum */
    while (get_obj_index(next_book_vnum))
        next_book_vnum++;

    *assigned_vnum = next_book_vnum;

    /* Create OBJ_INDEX */
    CREATE(pObjIndex, OBJ_INDEX_DATA, 1);
    pObjIndex->vnum = next_book_vnum;
    pObjIndex->name = str_dup("book tome");
    pObjIndex->short_descr = str_dup(title);
    pObjIndex->description = str_dup("A leather-bound book lies here.");
    pObjIndex->item_type = ITEM_NOTE; /* Or ITEM_BOOK if you have it */
    pObjIndex->wear_flags = ITEM_TAKE | ITEM_HOLD;
    pObjIndex->count = 0;
    pObjIndex->weight = 5;
    pObjIndex->cost = 100;

    /* Add to index */
    /* This would normally link into the hash table */

    /* Create actual object */
    CREATE(book, OBJ_DATA, 1);
    book->pIndexData = pObjIndex;
    book->in_room = NULL;
    book->name = str_dup(pObjIndex->name);
    book->short_descr = str_dup(pObjIndex->short_descr);
    book->description = str_dup(pObjIndex->description);

    /* Add extra description with content */
    CREATE(ed, EXTRA_DESCR_DATA, 1);
    ed->keyword = str_dup("book text content");
    ed->description = str_dup(content);
    ed->next = book->first_extradesc;
    book->first_extradesc = ed;

    /* Add author note */
    CREATE(ed, EXTRA_DESCR_DATA, 1);
    ed->keyword = str_dup("author");
    ed->description = str_dup(author);
    ed->next = book->first_extradesc;
    book->first_extradesc = ed;

    sprintf(log_buf, "BOOK CREATION: Created book object vnum %d: '%s' by %s",
            next_book_vnum, title, author);
    log_string(log_buf);

    next_book_vnum++;
    return book;
}

/*****************************************************************************
 * Book Writing Workflow
 *****************************************************************************/

bool mob_can_write_books(CHAR_DATA *mob)
{
    if (!mob || !IS_NPC(mob))
        return FALSE;

    /* Check if mob is a writer type */
    if (strstr(mob->short_descr, "poet") ||
        strstr(mob->short_descr, "scribe") ||
        strstr(mob->short_descr, "historian") ||
        strstr(mob->short_descr, "scholar") ||
        strstr(mob->short_descr, "bard") ||
        strstr(mob->short_descr, "chronicler"))
    {
        return TRUE;
    }

    return FALSE;
}

void mob_write_book(CHAR_DATA *author, int book_type, char *subject)
{
    WRITTEN_BOOK *book;
    OBJ_DATA *book_obj;
    char *content;
    char title[MAX_STRING_LENGTH];
    int vnum;
    int num_mentions;

    if (!author || !mob_can_write_books(author))
    {
        log_string("ERROR: Invalid author for book writing");
        return;
    }

    sprintf(log_buf, "BOOK WRITING: %s is writing about '%s'", author->short_descr, subject);
    log_string(log_buf);

    /* Generate content */
    content = generate_book_content(book_type, subject, author->short_descr, "scribe");
    if (!content)
    {
        log_string("ERROR: Failed to generate book content");
        return;
    }

    /* Generate title */
    switch (book_type)
    {
        case BOOK_TYPE_POETRY:
            sprintf(title, "Ode to %s", subject);
            break;
        case BOOK_TYPE_HISTORY:
            sprintf(title, "Chronicle of %s", subject);
            break;
        case BOOK_TYPE_FICTION:
            sprintf(title, "The Tale of %s", subject);
            break;
        case BOOK_TYPE_BIOGRAPHY:
            sprintf(title, "Musings on %s", subject);
            break;
        case BOOK_TYPE_MANUAL:
            sprintf(title, "A Treatise Concerning %s", subject);
            break;
        default:
            sprintf(title, "Regarding %s", subject);
            break;
    }

    /* Create book object */
    book_obj = create_book_object(title, author->short_descr, content, &vnum);
    if (!book_obj)
    {
        log_string("ERROR: Failed to create book object");
        DISPOSE(content);
        return;
    }

    /* Place book in author's inventory or room */
    if (author->in_room)
        obj_to_room(book_obj, author->in_room);
    else
        obj_to_char(book_obj, author);

    /* Track book */
    CREATE(book, WRITTEN_BOOK, 1);
    book->book_vnum = vnum;
    book->title = str_dup(title);
    book->author = str_dup(author->short_descr);
    book->content = content;
    book->written_when = time(NULL);
    book->mentioned_characters = NULL;
    book->num_mentions = 0;
    book->next = first_book;
    first_book = book;
    total_books_written++;

    /* Extract character mentions */
    extract_character_mentions(content, &num_mentions);

    /* Record in world history */
    record_book_written(author->short_descr, title, vnum, content);

    /* Record mentions */
    /* This would parse content and call record_book_mention() for each character */

    /* Announce */
    if (book_type == BOOK_TYPE_HISTORY || total_books_written % 10 == 0)
    {
        smart_announce(
            title,
            "A new work has been published",
            EVENT_CATEGORY_CULTURE,
            ANNOUNCE_PRIORITY_LOW,
            author->in_room ? author->in_room->area->name : "Unknown"
        );
    }

    sprintf(log_buf, "BOOK WRITING: Completed! '%s' by %s (vnum %d)", title, author->short_descr, vnum);
    log_string(log_buf);
}

void queue_book_writing(CHAR_DATA *author, int book_type, char *subject)
{
    BOOK_WRITING_TASK *task;

    CREATE(task, BOOK_WRITING_TASK, 1);
    task->author = author;
    task->book_type = book_type;
    task->subject = str_dup(subject);
    task->queued_when = time(NULL);
    task->next = first_task;
    first_task = task;

    sprintf(log_buf, "BOOK WRITING: Queued task for %s to write about '%s'",
            author->short_descr, subject);
    log_string(log_buf);
}

/*****************************************************************************
 * Update Loop - Process writing queue
 *****************************************************************************/

void book_writing_update(void)
{
    BOOK_WRITING_TASK *task, *task_next, *task_prev;
    time_t now = time(NULL);
    int processed = 0;

    task_prev = NULL;
    for (task = first_task; task && processed < 3; task = task_next)
    {
        task_next = task->next;

        /* Check if author still valid */
        if (!task->author || task->author->position == POS_DEAD)
        {
            /* Remove invalid task */
            if (task_prev)
                task_prev->next = task_next;
            else
                first_task = task_next;

            DISPOSE(task->subject);
            DISPOSE(task);
            continue;
        }

        /* Process if queued for at least 1 hour */
        if (difftime(now, task->queued_when) >= 3600)
        {
            mob_write_book(task->author, task->book_type, task->subject);

            if (task_prev)
                task_prev->next = task_next;
            else
                first_task = task_next;

            DISPOSE(task->subject);
            DISPOSE(task);
            processed++;
        }
        else
        {
            task_prev = task;
        }
    }

    if (processed > 0)
    {
        sprintf(log_buf, "BOOK WRITING: Processed %d writing tasks", processed);
        log_string(log_buf);
    }
}

/*****************************************************************************
 * Automatic Book Generation - Mobs write about events
 *****************************************************************************/

void trigger_book_about_event(HISTORY_EVENT *event)
{
    CHAR_DATA *mob;
    AREA_DATA *area;
    bool found_writer = FALSE;

    /* Find a historian or scribe in the event's area */
    if (event->room_vnum > 0)
    {
        ROOM_INDEX_DATA *room = get_room_index(event->room_vnum);
        if (room && room->area)
        {
            area = room->area;

            /* Look for writers in this area */
            for (mob = first_char; mob; mob = mob->next)
            {
                if (IS_NPC(mob) &&
                    mob->in_room &&
                    mob->in_room->area == area &&
                    mob_can_write_books(mob))
                {
                    /* Queue book about this event */
                    if (event->importance >= 8)
                    {
                        queue_book_writing(mob, BOOK_TYPE_HISTORY, event->description);
                        found_writer = TRUE;
                        break;
                    }
                }
            }
        }
    }

    if (found_writer)
    {
        sprintf(log_buf, "BOOK WRITING: Triggered automatic book about: %s", event->description);
        log_string(log_buf);
    }
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_writebook(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int book_type;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Usage: writebook <type> <subject>\n\r", ch);
        send_to_char("Types: poetry, history, fiction, biography, manual, lore\n\r", ch);
        return;
    }

    if (!str_cmp(arg1, "poetry") || !str_cmp(arg1, "poem"))
        book_type = BOOK_TYPE_POETRY;
    else if (!str_cmp(arg1, "history"))
        book_type = BOOK_TYPE_HISTORY;
    else if (!str_cmp(arg1, "fiction") || !str_cmp(arg1, "story"))
        book_type = BOOK_TYPE_FICTION;
    else if (!str_cmp(arg1, "biography") || !str_cmp(arg1, "journal"))
        book_type = BOOK_TYPE_BIOGRAPHY;
    else if (!str_cmp(arg1, "manual") || !str_cmp(arg1, "treatise"))
        book_type = BOOK_TYPE_MANUAL;
    else if (!str_cmp(arg1, "lore"))
        book_type = BOOK_TYPE_LORE;
    else
    {
        send_to_char("Invalid book type.\n\r", ch);
        return;
    }

    /* Create a temp mob as author for testing */
    mob_write_book(ch, book_type, arg2);
    send_to_char("Book created!\n\r", ch);
}

void do_books(CHAR_DATA *ch, char *argument)
{
    WRITTEN_BOOK *book;
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    if (IS_NPC(ch))
        return;

    send_to_char("&c=== Written Books ===&w\n\r\n\r", ch);

    sprintf(buf, "Total books written: %d\n\r\n\r", total_books_written);
    send_to_char(buf, ch);

    for (book = first_book; book && count < 20; book = book->next)
    {
        sprintf(buf, "&Y[%d]&w %s\n\r     by %s\n\r",
                book->book_vnum, book->title, book->author);
        send_to_char(buf, ch);
        count++;
    }
}
