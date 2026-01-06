/*****************************************************************************
 * Universal Mob AI - Intelligence for EVERY Mob
 *
 * NO MORE tontos mobs. TODOS tienen personalidad, memoria, metas, relaciones.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"
#include "universal_mob_ai.h"
#include "ollama_integration.h"

/* Mob AI data storage */
#define MAX_MOB_AI 500
UNIVERSAL_MOB_AI *mob_ai_list[MAX_MOB_AI];
int num_mob_ai = 0;

/* Personality types */
const char *personality_types[] = {
    "friendly", "grumpy", "greedy", "brave", "cautious",
    "wise", "foolish", "aggressive", "passive", "curious",
    "lazy", "hardworking", "honest", "deceptive", "loyal",
    NULL
};

/* Forward declarations */
UNIVERSAL_MOB_AI *get_mob_ai(CHAR_DATA *mob);

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_universal_mob_ai(void)
{
    int i;

    log_string("UNIVERSAL MOB AI: Initializing...");

    for (i = 0; i < MAX_MOB_AI; i++)
        mob_ai_list[i] = NULL;

    num_mob_ai = 0;

    log_string("UNIVERSAL MOB AI: Initialized");
}

void assign_mob_intelligence(CHAR_DATA *mob)
{
    UNIVERSAL_MOB_AI *ai;

    if (!mob || !IS_NPC(mob))
        return;

    /* Check if already has AI */
    if (get_mob_ai(mob))
        return;

    /* Allocate AI */
    ai = (UNIVERSAL_MOB_AI *)calloc(1, sizeof(UNIVERSAL_MOB_AI));
    if (!ai)
    {
        bug("assign_mob_intelligence: Failed to allocate AI");
        return;
    }

    ai->mob = mob;
    ai->intelligence_tier = determine_mob_intelligence_tier(mob);
    ai->power_level = determine_mob_power_level(mob);

    /* Generate personality */
    assign_personality_traits(ai);

    /* Initialize state */
    ai->mood = 50; /* Neutral */
    ai->energy = 70;
    ai->hunger = 50;
    ai->fear = 0;
    ai->curiosity = 30;

    ai->decision_cooldown = 0;
    ai->can_make_decisions = (ai->intelligence_tier >= INTELLIGENCE_SIMPLE);
    ai->can_create_things = (ai->power_level >= 3);

    /* Add to list */
    if (num_mob_ai < MAX_MOB_AI)
    {
        mob_ai_list[num_mob_ai] = ai;
        num_mob_ai++;
    }

    {
        char log_buf[256];
        sprintf(log_buf, "MOB AI: Assigned to %s (tier %d, power %d, %s)",
                mob->short_descr ? mob->short_descr : "unnamed",
                ai->intelligence_tier,
                ai->power_level,
                ai->personality_type ? ai->personality_type : "unknown");
        log_string(log_buf);
    }
}

/*****************************************************************************
 * Intelligence & Power Determination
 *****************************************************************************/

int determine_mob_intelligence_tier(CHAR_DATA *mob)
{
    if (!mob)
        return INTELLIGENCE_MINDLESS;

    /* Special mobs */
    if (mob->pIndexData && mob->pIndexData->vnum == 1200) /* Beeler */
        return INTELLIGENCE_GOD;

    /* By level */
    if (mob->level >= 90)
        return INTELLIGENCE_GENIUS;
    else if (mob->level >= 70)
        return INTELLIGENCE_STRATEGIC;
    else if (mob->level >= 50)
        return INTELLIGENCE_TACTICAL;
    else if (mob->level >= 30)
        return INTELLIGENCE_EDUCATED;
    else if (mob->level >= 15)
        return INTELLIGENCE_SOCIAL;
    else if (mob->level >= 5)
        return INTELLIGENCE_SIMPLE;
    else if (xIS_SET(mob->act, ACT_AGGRESSIVE))
        return INTELLIGENCE_INSTINCT;

    return INTELLIGENCE_ANIMAL;
}

int determine_mob_power_level(CHAR_DATA *mob)
{
    if (!mob)
        return 0;

    /* Beeler is god */
    if (mob->pIndexData && mob->pIndexData->vnum == 1200)
        return 10;

    /* By level and type */
    if (mob->level >= 90)
        return 8; /* Archmages, ancient dragons */
    else if (mob->level >= 70)
        return 7; /* Guild masters */
    else if (mob->level >= 50)
        return 5; /* Captains, merchants */
    else if (mob->level >= 30)
        return 3; /* Skilled NPCs */
    else if (mob->level >= 15)
        return 2; /* Common workers */
    else
        return 1; /* Simple folk */
}

/*****************************************************************************
 * Personality Generation
 *****************************************************************************/

void assign_personality_traits(UNIVERSAL_MOB_AI *ai)
{
    int i, num_traits;

    if (!ai)
        return;

    /* Random personality type */
    i = number_range(0, 14); /* 15 types */
    ai->personality_type = strdup(personality_types[i]);

    /* Multiple traits */
    num_traits = number_range(2, 5);
    for (i = 0; i < num_traits && i < 10; i++)
    {
        ai->personality_traits[i] = number_range(0, 14);
    }

    /* AI-generated full personality if high intelligence */
    if (ai->intelligence_tier >= INTELLIGENCE_SOCIAL && ollama_is_available())
    {
        ai->ai_personality_desc = ai_generate_mob_personality(ai->mob);
    }
}

char *ai_generate_mob_personality(CHAR_DATA *mob)
{
    char prompt[MAX_STRING_LENGTH];
    char *response;

    if (!mob)
        return strdup("ordinary");

    sprintf(prompt,
        "Generate a brief personality description (2-3 traits) for:\n"
        "NPC: %s\n"
        "Level: %d\n"
        "Give personality in 1 sentence.",
        mob->short_descr ? mob->short_descr : "an NPC",
        mob->level
    );

    response = ollama_request(prompt, 100);

    if (!response || response[0] == '\0')
        return strdup("ordinary personality");

    return response;
}

/*****************************************************************************
 * Memory System
 *****************************************************************************/

void mob_remember_event(CHAR_DATA *mob, char *event, int emotional_impact)
{
    UNIVERSAL_MOB_AI *ai;
    int i;

    if (!mob || !event)
        return;

    ai = get_mob_ai(mob);
    if (!ai)
        return;

    /* Find empty slot */
    for (i = 0; i < 50; i++)
    {
        if (!ai->memories[i])
        {
            ai->memories[i] = (struct mob_memory *)calloc(1, sizeof(struct mob_memory));
            if (ai->memories[i])
            {
                ai->memories[i]->event_description = strdup(event);
                ai->memories[i]->when = time(NULL);
                ai->memories[i]->emotional_impact = emotional_impact;
                ai->num_memories++;
            }
            return;
        }
    }

    /* If full, forget oldest memory */
    if (ai->memories[0])
    {
        free(ai->memories[0]->event_description);
        free(ai->memories[0]);
    }

    /* Shift memories */
    for (i = 0; i < 49; i++)
    {
        ai->memories[i] = ai->memories[i + 1];
    }

    /* Add new at end */
    ai->memories[49] = (struct mob_memory *)calloc(1, sizeof(struct mob_memory));
    if (ai->memories[49])
    {
        ai->memories[49]->event_description = strdup(event);
        ai->memories[49]->when = time(NULL);
        ai->memories[49]->emotional_impact = emotional_impact;
    }
}

bool mob_remembers(CHAR_DATA *mob, CHAR_DATA *other)
{
    UNIVERSAL_MOB_AI *ai;
    int i, j;

    if (!mob || !other)
        return FALSE;

    ai = get_mob_ai(mob);
    if (!ai)
        return FALSE;

    /* Check relationships */
    for (i = 0; i < ai->num_relationships; i++)
    {
        if (ai->relationships[i] && ai->relationships[i]->other == other)
            return TRUE;
    }

    return FALSE;
}

/*****************************************************************************
 * Relationship System
 *****************************************************************************/

void mob_update_relationship(CHAR_DATA *mob, CHAR_DATA *other, int change)
{
    UNIVERSAL_MOB_AI *ai;
    int i;

    if (!mob || !other)
        return;

    ai = get_mob_ai(mob);
    if (!ai)
        return;

    /* Find existing relationship */
    for (i = 0; i < ai->num_relationships; i++)
    {
        if (ai->relationships[i] && ai->relationships[i]->other == other)
        {
            ai->relationships[i]->relationship_value += change;

            /* Clamp */
            if (ai->relationships[i]->relationship_value > 100)
                ai->relationships[i]->relationship_value = 100;
            if (ai->relationships[i]->relationship_value < -100)
                ai->relationships[i]->relationship_value = -100;

            return;
        }
    }

    /* Create new relationship */
    if (ai->num_relationships < 20)
    {
        ai->relationships[ai->num_relationships] =
            (struct mob_relationship *)calloc(1, sizeof(struct mob_relationship));
        if (ai->relationships[ai->num_relationships])
        {
            ai->relationships[ai->num_relationships]->other = other;
            ai->relationships[ai->num_relationships]->relationship_value = change;
            ai->relationships[ai->num_relationships]->relationship_type = strdup("acquaintance");
            ai->num_relationships++;
        }
    }
}

int mob_get_relationship(CHAR_DATA *mob, CHAR_DATA *other)
{
    UNIVERSAL_MOB_AI *ai;
    int i;

    if (!mob || !other)
        return 0;

    ai = get_mob_ai(mob);
    if (!ai)
        return 0;

    for (i = 0; i < ai->num_relationships; i++)
    {
        if (ai->relationships[i] && ai->relationships[i]->other == other)
            return ai->relationships[i]->relationship_value;
    }

    return 0; /* Neutral */
}

/*****************************************************************************
 * AI Decision Making
 *****************************************************************************/

void mob_ai_think(CHAR_DATA *mob)
{
    UNIVERSAL_MOB_AI *ai;

    if (!mob || !IS_NPC(mob))
        return;

    ai = get_mob_ai(mob);
    if (!ai)
    {
        /* Assign AI if doesn't have one */
        assign_mob_intelligence(mob);
        return;
    }

    /* Decrement cooldown */
    if (ai->decision_cooldown > 0)
    {
        ai->decision_cooldown--;
        return;
    }

    /* Can this mob make decisions? */
    if (!ai->can_make_decisions)
        return;

    /* Make a decision */
    char *decision = mob_ai_decide_action(ai);
    if (decision)
    {
        mob_execute_decision(mob, decision);
        free(decision);
    }

    /* Reset cooldown */
    ai->decision_cooldown = number_range(20, 60); /* 20-60 ticks */
}

char *mob_ai_decide_action(UNIVERSAL_MOB_AI *ai)
{
    /* Simple decision for now */
    /* TODO: AI-powered decisions */

    if (!ai || !ai->mob)
        return NULL;

    /* Different decisions based on intelligence */
    if (ai->intelligence_tier >= INTELLIGENCE_SOCIAL)
    {
        /* Can decide complex actions */
        return strdup("wander"); /* Simple for now */
    }
    else if (ai->intelligence_tier >= INTELLIGENCE_ANIMAL)
    {
        /* Basic instincts */
        if (ai->hunger > 70)
            return strdup("find_food");
        if (ai->fear > 50)
            return strdup("flee");
    }

    return NULL;
}

void mob_execute_decision(CHAR_DATA *mob, char *decision)
{
    /* Execute the decision */
    /* TODO: Implement actual decision execution */

    if (!mob || !decision)
        return;

    /* For now just log */
    {
        char log_buf[256];
        sprintf(log_buf, "MOB AI: %s decided to %s",
                mob->short_descr ? mob->short_descr : "mob",
                decision);
        log_string(log_buf);
    }
}

/*****************************************************************************
 * Dynamic Conversation
 *****************************************************************************/

char *mob_ai_respond_to_speech(CHAR_DATA *mob, CHAR_DATA *speaker, char *what_said)
{
    UNIVERSAL_MOB_AI *ai;
    char prompt[MAX_STRING_LENGTH * 2];
    char *response;

    if (!mob || !speaker || !what_said)
        return NULL;

    ai = get_mob_ai(mob);
    if (!ai)
        return NULL;

    /* Need at least SIMPLE intelligence to converse */
    if (ai->intelligence_tier < INTELLIGENCE_SIMPLE)
        return NULL;

    /* Build conversation context */
    sprintf(prompt,
        "You are %s, a %s.\n"
        "Personality: %s\n"
        "Mood: %d/100 (0=miserable, 100=happy)\n"
        "%s says to you: \"%s\"\n\n"
        "Respond in character (1-2 sentences):",
        mob->short_descr ? mob->short_descr : "someone",
        ai->personality_type ? ai->personality_type : "ordinary person",
        ai->ai_personality_desc ? ai->ai_personality_desc : "ordinary",
        ai->mood,
        speaker->name ? speaker->name : "Someone",
        what_said
    );

    response = ollama_request(prompt, 150);

    if (!response || response[0] == '\0')
        return NULL;

    /* Remember this conversation */
    char memory[MAX_STRING_LENGTH];
    sprintf(memory, "Spoke with %s about %s",
            speaker->name ? speaker->name : "someone",
            what_said);
    mob_remember_event(mob, memory, 10); /* Mild positive */

    /* Update relationship */
    mob_update_relationship(mob, speaker, 5); /* Talking improves relationship */

    return response;
}

/*****************************************************************************
 * Update Loop
 *****************************************************************************/

void universal_mob_ai_update(void)
{
    CHAR_DATA *mob;

    /* Update all mobs in world */
    for (mob = first_char; mob; mob = mob->next)
    {
        if (IS_NPC(mob))
        {
            mob_ai_think(mob);
        }
    }
}

/*****************************************************************************
 * Utility Functions
 *****************************************************************************/

UNIVERSAL_MOB_AI *get_mob_ai(CHAR_DATA *mob)
{
    int i;

    if (!mob)
        return NULL;

    for (i = 0; i < num_mob_ai; i++)
    {
        if (mob_ai_list[i] && mob_ai_list[i]->mob == mob)
            return mob_ai_list[i];
    }

    return NULL;
}
