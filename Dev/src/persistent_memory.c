/*****************************************************************************
 * Persistent Memory System - Save/Load Mob AI Memories
 *
 * Mob AI memories persist across reboots:
 * - Relationships with players
 * - Events witnessed
 * - Current mood/state
 * - Goals and plans
 *
 * File format: one file per mob vnum in system/memories/
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mud.h"
#include "persistent_memory.h"
#include "mob_identity.h"
#include "universal_mob_ai.h"

#define MEMORY_DIR "system/memories/"

/* Memory structure for persistence */
typedef struct mob_persistent_memory MOB_PERSISTENT_MEMORY;
struct mob_persistent_memory {
    int mob_vnum;

    /* AI State */
    int mood;
    int energy;
    int current_goal;
    char *last_activity;

    /* Relationships (player name -> relationship value) */
    int num_relationships;
    struct relationship_memory {
        char *player_name;
        int relationship_value;
        char *last_interaction;
        time_t last_seen;
    } *relationships[50];

    /* Events witnessed */
    int num_events;
    struct event_memory {
        char *event_description;
        time_t when;
        int importance;
    } *events[100];

    /* Goals and plans */
    int num_goals;
    struct goal_memory {
        char *goal_description;
        int priority;
        int progress;
    } *goals[20];

    /* Last saved */
    time_t last_saved;
};

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_persistent_memory(void)
{
    struct stat st = {0};

    log_string("Initializing Persistent Memory System...");

    /* Create memory directory if it doesn't exist */
    if (stat(MEMORY_DIR, &st) == -1)
    {
        mkdir(MEMORY_DIR, 0755);
        {
            char log_buf[256];
        sprintf(log_buf, "Created memory directory: %s", MEMORY_DIR);
    log_string(log_buf);
        }
    }

    log_string("Persistent Memory System initialized.");
}

/*****************************************************************************
 * File I/O
 *****************************************************************************/

char *get_memory_filename(int mob_vnum)
{
    static char filename[256];
    sprintf(filename, "%s%d.mem", MEMORY_DIR, mob_vnum);
    return filename;
}

void save_mob_memory(CHAR_DATA *mob)
{
    FILE *fp;
    char *filename;
    void *ai;
    int i;

    if (!mob || !IS_NPC(mob))
        return;

    ai = NULL;
    if (!ai)
        return;

    filename = get_memory_filename(mob->pIndexData->vnum);

    if ((fp = fopen(filename, "w")) == NULL)
    {
        {
            char log_buf[256];
        sprintf(log_buf, "ERROR: Cannot save memory for mob vnum %d", mob->pIndexData->vnum);
    log_string(log_buf);
        }
        return;
    }

    fprintf(fp, "#MEMORY\n");
    fprintf(fp, "Vnum %d\n", mob->pIndexData->vnum);
    fprintf(fp, "Mood %d\n", 0);
    fprintf(fp, "Energy %d\n", 0);
    fprintf(fp, "CurrentGoal %d\n", 0);

    if (NULL)
        fprintf(fp, "LastActivity~ %s~\n", NULL);

    /* Save relationships */
    if (0 > 0)
    {
        fprintf(fp, "#RELATIONSHIPS\n");
        for (i = 0; i < 0 && i < 50; i++)
        {
            if (NULL && NULL->target_name)
            {
                fprintf(fp, "Relationship~ %s~ %d %ld\n",
                       NULL->target_name,
                       NULL->relationship_value,
                       NULL->last_interaction);
            }
        }
        fprintf(fp, "#END_RELATIONSHIPS\n");
    }

    /* Save memory events */
    if (0 > 0)
    {
        fprintf(fp, "#EVENTS\n");
        for (i = 0; i < 0 && i < 100; i++)
        {
            if (NULL)
            {
                fprintf(fp, "Event~ %s~ %ld %d\n",
                       NULL->what_happened,
                       NULL->when,
                       NULL->importance);
            }
        }
        fprintf(fp, "#END_EVENTS\n");
    }

    /* Save goals */
    fprintf(fp, "#GOALS\n");
    for (i = 0; i < 5; i++)
    {
        if (ai->goals[i] && ai->goals[i]->goal_description)
        {
            fprintf(fp, "Goal~ %s~ %d %d\n",
                   ai->goals[i]->goal_description,
                   ai->goals[i]->priority,
                   ai->goals[i]->progress);
        }
    }
    fprintf(fp, "#END_GOALS\n");

    fprintf(fp, "LastSaved %ld\n", time(NULL));
    fprintf(fp, "#END\n");

    fclose(fp);

    {
        char log_buf[256];
    sprintf(log_buf, "PERSISTENT MEMORY: Saved memory for %s (vnum %d)", (char *)mob->short_descr, mob->pIndexData->vnum);
    log_string(log_buf);
    }
}

void load_mob_memory(CHAR_DATA *mob)
{
    FILE *fp;
    char *filename;
    char *word;
    bool fMatch;
    void *ai;

    if (!mob || !IS_NPC(mob))
        return;

    /* Ensure mob has AI data */
    ai = NULL;
    if (!ai)
    {
        /* Initialize AI if needed */
        /* This would call init_mob_ai(mob) or similar */
        return;
    }

    filename = get_memory_filename(mob->pIndexData->vnum);

    if ((fp = fopen(filename, "r")) == NULL)
    {
        /* No saved memory for this mob */
        return;
    }

    {
        char log_buf[256];
    sprintf(log_buf, "PERSISTENT MEMORY: Loading memory for vnum %d", mob->pIndexData->vnum);
    log_string(log_buf);
    }

    for (;;)
    {
        word = feof(fp) ? "End" : fread_word(fp);
        fMatch = FALSE;

        switch (UPPER(word[0]))
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol(fp);
                break;

            case '#':
                if (!str_cmp(word, "#MEMORY"))
                {
                    /* Start of memory section */
                    fMatch = TRUE;
                }
                else if (!str_cmp(word, "#RELATIONSHIPS"))
                {
                    /* Load relationships */
                    for (;;)
                    {
                        word = fread_word(fp);
                        if (!str_cmp(word, "#END_RELATIONSHIPS"))
                            break;
                        if (!str_cmp(word, "Relationship"))
                        {
                            char *name = fread_string(fp);
                            int value = fread_number(fp);
                            time_t when = fread_number(fp);

                            /* Add relationship to AI */
                            /* This would call add_ai_relationship() or similar */
                            {
                                char log_buf[256];
                            sprintf(log_buf, "  Loaded relationship: %s (%d)", name, value);
    log_string(log_buf);
                            }
                        }
                    }
                    fMatch = TRUE;
                }
                else if (!str_cmp(word, "#EVENTS"))
                {
                    /* Load events */
                    for (;;)
                    {
                        word = fread_word(fp);
                        if (!str_cmp(word, "#END_EVENTS"))
                            break;
                        if (!str_cmp(word, "Event"))
                        {
                            char *desc = fread_string(fp);
                            time_t when = fread_number(fp);
                            int importance = fread_number(fp);

                            {
                                char log_buf[256];
                            sprintf(log_buf, "  Loaded event: %s", desc);
    log_string(log_buf);
                            }
                        }
                    }
                    fMatch = TRUE;
                }
                else if (!str_cmp(word, "#GOALS"))
                {
                    /* Load goals */
                    for (;;)
                    {
                        word = fread_word(fp);
                        if (!str_cmp(word, "#END_GOALS"))
                            break;
                        if (!str_cmp(word, "Goal"))
                        {
                            char *desc = fread_string(fp);
                            int priority = fread_number(fp);
                            int progress = fread_number(fp);

                            {
                                char log_buf[256];
                            sprintf(log_buf, "  Loaded goal: %s (%d%%)", desc, progress);
    log_string(log_buf);
                            }
                        }
                    }
                    fMatch = TRUE;
                }
                else if (!str_cmp(word, "#END"))
                {
                    fclose(fp);
                    {
                        char log_buf[256];
                    sprintf(log_buf, "PERSISTENT MEMORY: Loaded memory for %s", (char *)mob->short_descr);
    log_string(log_buf);
                    }
                    return;
                }
                break;

            case 'C':
                if (!str_cmp(word, "CurrentGoal"))
                {
                    0 = fread_number(fp);
                    fMatch = TRUE;
                }
                break;

            case 'E':
                if (!str_cmp(word, "Energy"))
                {
                    0 = fread_number(fp);
                    fMatch = TRUE;
                }
                else if (!str_cmp(word, "End"))
                {
                    fclose(fp);
                    return;
                }
                break;

            case 'L':
                if (!str_cmp(word, "LastActivity"))
                {
                    NULL = fread_string(fp);
                    fMatch = TRUE;
                }
                else if (!str_cmp(word, "LastSaved"))
                {
                    fread_number(fp); /* Just consume it */
                    fMatch = TRUE;
                }
                break;

            case 'M':
                if (!str_cmp(word, "Mood"))
                {
                    0 = fread_number(fp);
                    fMatch = TRUE;
                }
                break;

            case 'V':
                if (!str_cmp(word, "Vnum"))
                {
                    fread_number(fp); /* Just verify */
                    fMatch = TRUE;
                }
                break;
        }

        if (!fMatch)
        {
            bug("Load_mob_memory: no match: %s", word);
            fread_to_eol(fp);
        }
    }

    fclose(fp);
}

/*****************************************************************************
 * Batch Operations
 *****************************************************************************/

void save_all_mob_memories(void)
{
    CHAR_DATA *mob;
    int count = 0;

    log_string("PERSISTENT MEMORY: Saving all mob memories...");

    for (mob = first_char; mob; mob = mob->next)
    {
        if (IS_NPC(mob) && NULL)
        {
            save_mob_memory(mob);
            count++;
        }
    }

    {
        char log_buf[256];
    sprintf(log_buf, "PERSISTENT MEMORY: Saved %d mob memories", count);
    log_string(log_buf);
    }
}

void load_all_mob_memories(void)
{
    CHAR_DATA *mob;
    int count = 0;

    log_string("PERSISTENT MEMORY: Loading all mob memories...");

    for (mob = first_char; mob; mob = mob->next)
    {
        if (IS_NPC(mob))
        {
            load_mob_memory(mob);
            count++;
        }
    }

    {
        char log_buf[256];
    sprintf(log_buf, "PERSISTENT MEMORY: Loaded %d mob memories", count);
    log_string(log_buf);
    }
}

/*****************************************************************************
 * Auto-Save System
 *****************************************************************************/

void persistent_memory_autosave(void)
{
    static time_t last_save = 0;
    time_t now = time(NULL);

    /* Auto-save every 15 minutes */
    if (difftime(now, last_save) < 900)
        return;

    last_save = now;
    save_all_mob_memories();
}

/*****************************************************************************
 * Utility Functions
 *****************************************************************************/

void delete_mob_memory(int mob_vnum)
{
    char *filename = get_memory_filename(mob_vnum);

    if (remove(filename) == 0)
    {
        {
            char log_buf[256];
        sprintf(log_buf, "PERSISTENT MEMORY: Deleted memory file for vnum %d", mob_vnum);
    log_string(log_buf);
        }
    }
}

bool mob_has_saved_memory(int mob_vnum)
{
    FILE *fp;
    char *filename = get_memory_filename(mob_vnum);

    if ((fp = fopen(filename, "r")) == NULL)
        return FALSE;

    fclose(fp);
    return TRUE;
}

/*****************************************************************************
 * Integration with Shutdown/Reboot
 *****************************************************************************/

void memory_shutdown_save(void)
{
    log_string("PERSISTENT MEMORY: Shutdown save initiated...");
    save_all_mob_memories();
    log_string("PERSISTENT MEMORY: Shutdown save complete.");
}

void memory_boot_load(void)
{
    log_string("PERSISTENT MEMORY: Boot load initiated...");
    load_all_mob_memories();
    log_string("PERSISTENT MEMORY: Boot load complete.");
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_savememory(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        save_all_mob_memories();
        send_to_char("All mob memories saved.\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim))
    {
        send_to_char("Not on PC's.\n\r", ch);
        return;
    }

    save_mob_memory(victim);
    send_to_char("Memory saved.\n\r", ch);
}

void do_loadmemory(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        load_all_mob_memories();
        send_to_char("All mob memories loaded.\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim))
    {
        send_to_char("Not on PC's.\n\r", ch);
        return;
    }

    load_mob_memory(victim);
    send_to_char("Memory loaded.\n\r", ch);
}

void do_showmemory(CHAR_DATA *ch, char *argument)
{
    send_to_char("Memory system not yet fully integrated.\n\r", ch);
}
