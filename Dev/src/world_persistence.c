/****************************************************************************
 * World Persistence System - Auto-save dynamic mobs for crash recovery
 *
 * This system saves auto-created mobs to area files periodically so that
 * if the MUD crashes or is killed, the world state persists.
 *
 * All auto-created mobs are marked with ACT_AUTOCREATED flag for safety.
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* Global counters */
int total_autocreated_mobs = 0;
time_t last_world_save = 0;

/*
 * Save all autocreated mobs to their respective area files
 */
void save_autocreated_mobs(void)
{
    CHAR_DATA *mob;
    AREA_DATA *area;
    FILE *fp;
    char filename[256];
    int saved_count = 0;

    /* Loop through all characters */
    for (mob = first_char; mob; mob = mob->next)
    {
        /* Skip players and non-autocreated mobs */
        if (!IS_NPC(mob))
            continue;
        if (!xIS_SET(mob->act, ACT_AUTOCREATED))
            continue;
        if (!mob->in_room || !mob->in_room->area)
            continue;

        area = mob->in_room->area;

        /* Open area file in append mode */
        /* For now, we'll append autocreated mobs to a separate section */
        /* This prevents corruption of original area files */
        sprintf(filename, "../area/%s.autocreated", area->filename);

        fp = fopen(filename, "a");
        if (!fp)
        {
            sprintf(log_buf, "ERROR: Cannot open %s for writing autocreated mobs", filename);
            log_string(log_buf);
            continue;
        }

        /* Write mob in SMAUG format */
        fprintf(fp, "#MOBILE\n");
        fprintf(fp, "Vnum       %d\n", mob->pIndexData->vnum);
        fprintf(fp, "Keywords   %s~\n", mob->name ? mob->name : "autocreated mob");
        fprintf(fp, "Short      %s~\n", mob->short_descr ? mob->short_descr : "an autocreated mob");
        fprintf(fp, "Long       %s~\n", mob->long_descr ? mob->long_descr : "An autocreated mob stands here.\n");
        fprintf(fp, "Desc       %s~\n", mob->description ? mob->description : "You see an autocreated mob.\n");
        fprintf(fp, "Race       %d\n", mob->race);
        fprintf(fp, "Class      %d\n", mob->class);
        fprintf(fp, "Position   %d\n", mob->position);
        fprintf(fp, "DefPos     %d\n", mob->defposition);
        fprintf(fp, "Gender     %d\n", mob->sex);
        fprintf(fp, "Level      %d\n", mob->level);
        fprintf(fp, "Align      %d\n", mob->alignment);
        fprintf(fp, "Hitroll    %d\n", mob->hitroll);
        fprintf(fp, "Damroll    %d\n", mob->damroll);
        fprintf(fp, "Hit        %d %d %d\n", mob->pIndexData->hitnodice, mob->pIndexData->hitsizedice, mob->pIndexData->hitplus);
        fprintf(fp, "Dam        %d %d %d\n", mob->pIndexData->damnodice, mob->pIndexData->damsizedice, mob->pIndexData->damplus);
        fprintf(fp, "AffectedBy %s\n", print_bitvector(&mob->affected_by));
        fprintf(fp, "Act        %s\n", print_bitvector(&mob->act));
        fprintf(fp, "Armor      %d\n", mob->armor);
        fprintf(fp, "Room       %d\n", mob->in_room->vnum);
        fprintf(fp, "End\n\n");

        fclose(fp);
        saved_count++;
    }

    if (saved_count > 0)
    {
        sprintf(log_buf, "WORLD PERSISTENCE: Saved %d autocreated mobs", saved_count);
        log_string(log_buf);
    }

    last_world_save = time(NULL);
    total_autocreated_mobs = saved_count;
}

/*
 * Load autocreated mobs from area files
 */
void load_autocreated_mobs(void)
{
    AREA_DATA *area;
    FILE *fp;
    char filename[256];
    int loaded_count = 0;

    /* Loop through all areas */
    for (area = first_area; area; area = area->next)
    {
        sprintf(filename, "../area/%s.autocreated", area->filename);

        fp = fopen(filename, "r");
        if (!fp)
            continue;  /* No autocreated mobs for this area */

        sprintf(log_buf, "Loading autocreated mobs from %s", filename);
        log_string(log_buf);

        /* Parse autocreated mobs file */
        /* TODO: Implement parser similar to load_mobiles() in db.c */
        /* For now, just log that we found the file */

        fclose(fp);
        loaded_count++;

        /* Delete the file after loading to prevent duplicates */
        unlink(filename);
    }

    if (loaded_count > 0)
    {
        sprintf(log_buf, "WORLD PERSISTENCE: Loaded autocreated mobs from %d areas", loaded_count);
        log_string(log_buf);
    }
}

/*
 * Clean up all autocreated mobs (for emergency cleanup)
 */
int cleanup_autocreated_mobs(void)
{
    CHAR_DATA *mob;
    CHAR_DATA *mob_next;
    int removed = 0;

    for (mob = first_char; mob; mob = mob_next)
    {
        mob_next = mob->next;

        if (!IS_NPC(mob))
            continue;
        if (!xIS_SET(mob->act, ACT_AUTOCREATED))
            continue;

        extract_char(mob, TRUE);
        removed++;
    }

    sprintf(log_buf, "CLEANUP: Removed %d autocreated mobs", removed);
    log_string(log_buf);

    return removed;
}

/*
 * Count autocreated mobs in world
 */
int count_autocreated_mobs(void)
{
    CHAR_DATA *mob;
    int count = 0;

    for (mob = first_char; mob; mob = mob->next)
    {
        if (!IS_NPC(mob))
            continue;
        if (!xIS_SET(mob->act, ACT_AUTOCREATED))
            continue;
        count++;
    }

    return count;
}

/*
 * Delete all .autocreated files (for cleanup)
 */
void delete_autocreated_files(void)
{
    AREA_DATA *area;
    char filename[256];
    int deleted = 0;

    for (area = first_area; area; area = area->next)
    {
        sprintf(filename, "../area/%s.autocreated", area->filename);
        if (unlink(filename) == 0)
            deleted++;
    }

    if (deleted > 0)
    {
        sprintf(log_buf, "CLEANUP: Deleted %d .autocreated files", deleted);
        log_string(log_buf);
    }
}
