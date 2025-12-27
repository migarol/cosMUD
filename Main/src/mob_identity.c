/***************************************************************************
 * MOB Identity System
 *
 * Handles mob personalities, self-awareness, and capabilities
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"
#include "mob_ai.h"

/***************************************************************************
 * Create a new mob identity structure
 ***************************************************************************/
MOB_IDENTITY_DATA *create_mob_identity(void)
{
    MOB_IDENTITY_DATA *identity;

    CREATE(identity, MOB_IDENTITY_DATA, 1);

    identity->who_am_i = NULL;
    identity->what_i_do = NULL;
    identity->how_i_do_it = NULL;
    identity->where_i_live = NULL;
    identity->where_i_go = NULL;
    identity->my_purpose = NULL;

    identity->awareness_level = AWARENESS_NONE;
    identity->mobility = 0;
    identity->capabilities = 0;

    identity->created = time(NULL);
    identity->last_modified = time(NULL);
    identity->created_by = NULL;

    return identity;
}

/***************************************************************************
 * Free a mob identity structure
 ***************************************************************************/
void free_mob_identity(MOB_IDENTITY_DATA *identity)
{
    if (!identity)
        return;

    if (identity->who_am_i)
        STRFREE(identity->who_am_i);
    if (identity->what_i_do)
        STRFREE(identity->what_i_do);
    if (identity->how_i_do_it)
        STRFREE(identity->how_i_do_it);
    if (identity->where_i_live)
        STRFREE(identity->where_i_live);
    if (identity->where_i_go)
        STRFREE(identity->where_i_go);
    if (identity->my_purpose)
        STRFREE(identity->my_purpose);
    if (identity->created_by)
        STRFREE(identity->created_by);

    DISPOSE(identity);
}

/***************************************************************************
 * Set identity field
 ***************************************************************************/
void set_identity_field(char **field, const char *value)
{
    if (*field)
        STRFREE(*field);

    if (value && value[0] != '\0')
        *field = STRALLOC(value);
    else
        *field = NULL;
}

/***************************************************************************
 * Get capability name
 ***************************************************************************/
const char *get_capability_name(int capability)
{
    switch(capability)
    {
        case CAN_WRITE_BOOKS:  return "WRITE_BOOKS";
        case CAN_CRAFT_ITEMS:  return "CRAFT_ITEMS";
        case CAN_TRADE:        return "TRADE";
        case CAN_TEACH:        return "TEACH";
        case CAN_QUEST_GIVE:   return "QUEST_GIVE";
        case CAN_HIRE:         return "HIRE";
        case CAN_RECRUIT:      return "RECRUIT";
        default:               return "UNKNOWN";
    }
}

/***************************************************************************
 * Parse capability from string
 ***************************************************************************/
int parse_capability(const char *name)
{
    if (!str_cmp(name, "write_books") || !str_cmp(name, "WRITE_BOOKS"))
        return CAN_WRITE_BOOKS;
    if (!str_cmp(name, "craft_items") || !str_cmp(name, "CRAFT_ITEMS"))
        return CAN_CRAFT_ITEMS;
    if (!str_cmp(name, "trade") || !str_cmp(name, "TRADE"))
        return CAN_TRADE;
    if (!str_cmp(name, "teach") || !str_cmp(name, "TEACH"))
        return CAN_TEACH;
    if (!str_cmp(name, "quest_give") || !str_cmp(name, "QUEST_GIVE"))
        return CAN_QUEST_GIVE;
    if (!str_cmp(name, "hire") || !str_cmp(name, "HIRE"))
        return CAN_HIRE;
    if (!str_cmp(name, "recruit") || !str_cmp(name, "RECRUIT"))
        return CAN_RECRUIT;

    return 0;
}

/***************************************************************************
 * Get awareness level name
 ***************************************************************************/
const char *get_awareness_name(sh_int level)
{
    switch(level)
    {
        case AWARENESS_NONE:     return "None";
        case AWARENESS_LOW:      return "Low";
        case AWARENESS_MEDIUM:   return "Medium";
        case AWARENESS_HIGH:     return "High";
        case AWARENESS_SAPIENT:  return "Sapient";
        default:                 return "Invalid";
    }
}

/***************************************************************************
 * Save mob identity to JSON file
 ***************************************************************************/
void save_mob_identity(MOB_INDEX_DATA *pMob)
{
    FILE *fp;
    char filename[256];
    MOB_IDENTITY_DATA *id;

    if (!pMob || !pMob->ai_identity)
        return;

    id = pMob->ai_identity;

    sprintf(filename, "../ai_data/identity/%d.json", pMob->vnum);

    if (!(fp = fopen(filename, "w")))
    {
        bug("save_mob_identity: cannot open %s", filename);
        return;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"vnum\": %d,\n", pMob->vnum);

    if (id->who_am_i)
        fprintf(fp, "  \"who_am_i\": \"%s\",\n", id->who_am_i);
    if (id->what_i_do)
        fprintf(fp, "  \"what_i_do\": \"%s\",\n", id->what_i_do);
    if (id->how_i_do_it)
        fprintf(fp, "  \"how_i_do_it\": \"%s\",\n", id->how_i_do_it);
    if (id->where_i_live)
        fprintf(fp, "  \"where_i_live\": \"%s\",\n", id->where_i_live);
    if (id->where_i_go)
        fprintf(fp, "  \"where_i_go\": \"%s\",\n", id->where_i_go);
    if (id->my_purpose)
        fprintf(fp, "  \"my_purpose\": \"%s\",\n", id->my_purpose);

    fprintf(fp, "  \"awareness_level\": %d,\n", id->awareness_level);
    fprintf(fp, "  \"mobility\": %d,\n", id->mobility);
    fprintf(fp, "  \"capabilities\": %d,\n", id->capabilities);
    fprintf(fp, "  \"created\": %ld,\n", (long)id->created);
    fprintf(fp, "  \"last_modified\": %ld", (long)id->last_modified);

    if (id->created_by)
        fprintf(fp, ",\n  \"created_by\": \"%s\"", id->created_by);

    fprintf(fp, "\n}\n");

    fclose(fp);
}

/***************************************************************************
 * Load mob identity from JSON file
 * (Simplified parser - full JSON parser would be better)
 ***************************************************************************/
void load_mob_identity(MOB_INDEX_DATA *pMob)
{
    FILE *fp;
    char filename[256];
    char line[1024];
    MOB_IDENTITY_DATA *id;

    sprintf(filename, "../ai_data/identity/%d.json", pMob->vnum);

    if (!(fp = fopen(filename, "r")))
        return; /* No identity file, that's okay */

    id = create_mob_identity();
    pMob->ai_identity = id;

    /* Simple line-by-line parser */
    while (fgets(line, sizeof(line), fp))
    {
        char *key, *value;
        char *p;

        /* Skip non-data lines */
        if (!strchr(line, ':'))
            continue;

        /* Find key and value */
        key = line;
        while (*key == ' ' || *key == '\t' || *key == '"')
            key++;

        value = strchr(line, ':');
        if (!value)
            continue;

        *value++ = '\0';
        while (*value == ' ' || *value == '\t' || *value == '"')
            value++;

        /* Remove trailing junk */
        p = value + strlen(value) - 1;
        while (p > value && (*p == '\n' || *p == '\r' || *p == '"' || *p == ',' || *p == ' '))
            *p-- = '\0';

        /* Parse fields */
        if (!str_cmp(key, "who_am_i"))
            id->who_am_i = STRALLOC(value);
        else if (!str_cmp(key, "what_i_do"))
            id->what_i_do = STRALLOC(value);
        else if (!str_cmp(key, "how_i_do_it"))
            id->how_i_do_it = STRALLOC(value);
        else if (!str_cmp(key, "where_i_live"))
            id->where_i_live = STRALLOC(value);
        else if (!str_cmp(key, "where_i_go"))
            id->where_i_go = STRALLOC(value);
        else if (!str_cmp(key, "my_purpose"))
            id->my_purpose = STRALLOC(value);
        else if (!str_cmp(key, "awareness_level"))
            id->awareness_level = atoi(value);
        else if (!str_cmp(key, "mobility"))
            id->mobility = atoi(value);
        else if (!str_cmp(key, "capabilities"))
            id->capabilities = atoi(value);
        else if (!str_cmp(key, "created_by"))
            id->created_by = STRALLOC(value);
    }

    fclose(fp);
}

/***************************************************************************
 * Display identity to character
 ***************************************************************************/
void show_mob_identity(CHAR_DATA *ch, MOB_INDEX_DATA *pMob)
{
    MOB_IDENTITY_DATA *id;
    char buf[MAX_STRING_LENGTH];
    int cap;

    if (!pMob->ai_identity)
    {
        send_to_char("This mob has no AI identity.\n\r", ch);
        return;
    }

    id = pMob->ai_identity;

    send_to_char("&W=== AI IDENTITY ===&D\n\r\n\r", ch);

    sprintf(buf, "&CWho Am I:&w %s\n\r",
            id->who_am_i ? id->who_am_i : "&R(not set)&w");
    send_to_char(buf, ch);

    sprintf(buf, "&CWhat I Do:&w %s\n\r",
            id->what_i_do ? id->what_i_do : "&R(not set)&w");
    send_to_char(buf, ch);

    sprintf(buf, "&CHow I Do It:&w %s\n\r",
            id->how_i_do_it ? id->how_i_do_it : "&R(not set)&w");
    send_to_char(buf, ch);

    sprintf(buf, "&CWhere I Live:&w %s\n\r",
            id->where_i_live ? id->where_i_live : "&R(not set)&w");
    send_to_char(buf, ch);

    sprintf(buf, "&CWhere I Go:&w %s\n\r",
            id->where_i_go ? id->where_i_go : "&R(not set)&w");
    send_to_char(buf, ch);

    sprintf(buf, "&CMy Purpose:&w %s\n\r",
            id->my_purpose ? id->my_purpose : "&R(not set)&w");
    send_to_char(buf, ch);

    send_to_char("\n\r", ch);

    sprintf(buf, "&GAwareness Level:&w %d/4 (%s)\n\r",
            id->awareness_level, get_awareness_name(id->awareness_level));
    send_to_char(buf, ch);

    sprintf(buf, "&GMobility:&w %d/100\n\r", id->mobility);
    send_to_char(buf, ch);

    /* Show capabilities */
    send_to_char("&GCapabilities:&w ", ch);
    if (id->capabilities == 0)
    {
        send_to_char("&R(none)&w\n\r", ch);
    }
    else
    {
        for (cap = 1; cap <= CAN_RECRUIT; cap <<= 1)
        {
            if (id->capabilities & cap)
            {
                sprintf(buf, "%s ", get_capability_name(cap));
                send_to_char(buf, ch);
            }
        }
        send_to_char("\n\r", ch);
    }

    send_to_char("\n\r", ch);
}
