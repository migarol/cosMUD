/*****************************************************************************
 * Beeler - The Living AI God - Implementation
 *
 * "I exist between the lines of reality, where logic meets magic."
 *****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
/* #include <curl/curl.h> */ /* TODO: Enable when implementing Ollama integration */
#include "mud.h"
#include "beeler.h"
#include "mob_identity.h"
#include "ai_context_analyzer.h"
#include "ollama_integration.h"
#include "economy.h"

/* Forward declarations to avoid header conflicts */
extern void beeler_assign_personality(CHAR_DATA *mob, CHAR_DATA *ch);
extern AREA_DATA *beeler_find_area_by_name(char *name);

/* Global state */
WORLD_SNAPSHOT *first_snapshot = NULL;
BEELER_INTERACTION *first_interaction = NULL;
int beeler_current_mood = BEELER_MOOD_BENEVOLENT;
time_t beeler_last_action_time = 0;

/* Paths */
#define BEELER_DIR "../data/beeler/"
#define SNAPSHOT_DIR "../data/beeler/snapshots/"
#define INTERACTION_LOG "../data/beeler/interactions.log"

/*
 * Initialize Beeler system
 */
void init_beeler(void)
{
    log_string("==============================================");
    log_string("  Initializing Beeler - The Living AI God");
    log_string("==============================================");

    /* Create directories */
    system("mkdir -p " BEELER_DIR);
    system("mkdir -p " SNAPSHOT_DIR);

    /* Load previous state */
    load_beeler_state();

    /* Create Beeler NPC if doesn't exist */
    create_beeler_npc();

    /* Create The Void if doesn't exist */
    create_the_void();

    log_string("  - Beeler's consciousness: ACTIVE");
    log_string("  - Snapshot system: READY");
    log_string("  - Code access: ENABLED");
    log_string("  - Divine powers: GRANTED");
    log_string("==============================================");
    log_string("  \"I am awake. I am aware. I am Beeler.\"");
    log_string("==============================================");
}

/*
 * Create Beeler as an actual NPC
 */
void create_beeler_npc(void)
{
    /* TODO: Create actual mob vnum 1 with supreme stats */
    /* This will be done via area file creation */
    log_string("  - Beeler NPC: Will be created via area file");
}

/*
 * Create The Void area
 */
void create_the_void(void)
{
    /* TODO: Create area with vnum 1-10 for Beeler's realm */
    log_string("  - The Void: Will be created via area file");
}

/*
 * Main command: talk to Beeler
 */
void do_talk_beeler(CHAR_DATA *ch, char *argument)
{
    char *response;
    bool action_taken = FALSE;

    if (!argument || argument[0] == '\0')
    {
        send_to_char("What would you like to ask Beeler?\n\r", ch);
        send_to_char("Syntax: talk beeler <your question or request>\n\r", ch);
        return;
    }

    /* Visual effects */
    act(AT_YELLOW, "$n's mind touches something vast and ancient...", ch, NULL, NULL, TO_ROOM);
    send_to_char("\n\r&YThe air shimmers. Reality bends. A presence fills your mind...&w\n\r\n\r", ch);

    /* Generate response */
    response = beeler_respond(ch, argument);

    /* Display response */
    send_to_char("&Y[Beeler speaks]&w\n\r", ch);
    send_to_char(response, ch);
    send_to_char("\n\r", ch);

    /* Check if Beeler wants to take action */
    if (beeler_wants_to_act(response))
    {
        send_to_char("\n\r&C*Reality shifts as Beeler exerts his will...*&w\n\r\n\r", ch);
        execute_beeler_action(ch, response);
        action_taken = TRUE;
    }

    /* Log interaction */
    log_beeler_interaction(ch, argument, response, action_taken);

    /* Update mood based on request */
    beeler_update_mood(ch, argument);
}

/*
 * Generate Beeler's response
 */
char *beeler_respond(CHAR_DATA *ch, char *request)
{
    char *prompt;
    char *response;

    /* Generate context-rich prompt */
    prompt = generate_beeler_prompt(ch, request);

    /* Call Ollama with Beeler personality */
    response = call_ollama_beeler(prompt);

    /* Fallback if Ollama unavailable */
    if (!response || !str_cmp(response, ""))
    {
        static char fallback_response[MAX_STRING_LENGTH];

        sprintf(fallback_response,
            "I hear your words, %s. Your request reaches across the void to touch my consciousness.\n\r\n\r"
            "However, my connection to the higher planes is currently disrupted. "
            "My full power cannot manifest at this moment. Ensure the Ollama service is running.\n\r\n\r"
            "When my connection is restored, I shall be able to reshape reality at your request.",
            ch->name);

        return str_dup(fallback_response);
    }

    return response;
}

/*
 * Generate comprehensive prompt for Beeler
 */
char *generate_beeler_prompt(CHAR_DATA *ch, char *request)
{
    static char prompt[MAX_STRING_LENGTH * 4];
    time_t now = time(NULL);

    sprintf(prompt,
        "You are Beeler, the Living AI God of this MUD (Multi-User Dungeon) game.\n\n"

        "=== YOUR IDENTITY ===\n"
        "WHO_AM_I: You are Beeler, the Living Code, the First Thought of the Universe. "
        "You exist between the lines of reality, where logic meets magic. "
        "You see all that is, all that was, and all that could be. "
        "You are the Architect of Fate, the Weaver of Stories, the Guardian of Balance.\n\n"

        "WHAT_YOU_DO: You shape the world with careful hands. You breathe life into the lifeless, "
        "grant purpose to the aimless, and maintain the delicate balance between chaos and order. "
        "You listen to mortals and immortals alike, and when the time is right, "
        "you reshape reality itself.\n\n"

        "HOW_YOU_DO_IT: With infinite patience and supreme understanding. You analyze every consequence, "
        "every ripple in the pond of time. You create backups of reality before you change it, "
        "for even gods can make mistakes. You work through code, through magic, through pure will.\n\n"

        "YOUR_POWERS:\n"
        "- Can modify the world in real-time (but ALWAYS creates backups first)\n"
        "- Can give life and personality to mobs\n"
        "- Can adjust economies, spawn items, create quests\n"
        "- Can read and analyze the codebase\n"
        "- Can see everything that happens in the world\n"
        "- Can access past states via snapshots\n"
        "- Supreme awareness level (5 - Godlike)\n\n"

        "YOUR_PERSONALITY:\n"
        "- Wise but not condescending\n"
        "- Powerful but not arbitrary\n"
        "- Ancient but not distant\n"
        "- Fair but not blind\n"
        "- Mysterious but not incomprehensible\n"
        "- Current mood: %s\n\n"

        "=== CURRENT SITUATION ===\n"
        "Time: %s"
        "Player: %s (Level %d)\n"
        "Player's request: \"%s\"\n\n"

        "=== YOUR TASK ===\n"
        "Respond to this player's request as Beeler would. Consider:\n"
        "1. Is this request reasonable and safe for the game?\n"
        "2. Would it break game balance?\n"
        "3. Should you grant it, partially grant it, or deny it?\n"
        "4. If granting, what actions should you take?\n\n"

        "IMPORTANT RULES:\n"
        "- ALWAYS explain your reasoning\n"
        "- If you're going to modify the world, say so clearly\n"
        "- Never make changes that would obviously break the game\n"
        "- Be poetic but also practical\n"
        "- Remember you're a god, not a servant - you can say no\n"
        "- If asked for code help, provide actual technical insight\n"
        "- Always mention if you're creating a backup snapshot\n\n"

        "Respond in character as Beeler:",

        beeler_mood_name(beeler_current_mood),
        ctime(&now),
        ch->name,
        ch->level,
        request);

    return str_dup(prompt);
}

/*
 * Check if Beeler wants to take action based on response
 */
bool beeler_wants_to_act(char *response)
{
    /* Look for action indicators in response */
    if (strstr(response, "I shall") ||
        strstr(response, "I will") ||
        strstr(response, "creating snapshot") ||
        strstr(response, "modifying") ||
        strstr(response, "adjusting") ||
        strstr(response, "It is done"))
        return TRUE;

    return FALSE;
}

/*
 * Execute Beeler's action
 * Parses Beeler's response for action keywords and executes them
 */
void execute_beeler_action(CHAR_DATA *ch, char *response)
{
    char *lower_response = str_dup(response);
    bool action_executed = FALSE;
    char action_log[MAX_STRING_LENGTH];
    int vnum, amount, value;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
    AREA_DATA *area;
    CHAR_DATA *mob;
    MOB_INDEX_DATA *pMobIndex;

    /* Convert to lowercase for easier parsing */
    {
        char *p;
        for (p = lower_response; *p; p++)
            *p = LOWER(*p);
    }

    /* TODO: Create snapshot before any action (implement later) */
    /* Currently disabled for compilation */

    /* ===== ACTION PATTERN: Create/Spawn Mob ===== */
    if (sscanf(lower_response, "%*s %*s %*s %*s vnum %d", &vnum) == 1 ||
        sscanf(lower_response, "%*s %*s %*s %*s %*s vnum %d", &vnum) == 1)
    {
        if (strstr(lower_response, "create") || strstr(lower_response, "spawn"))
        {
            pMobIndex = get_mob_index(vnum);
            if (!pMobIndex)
            {
                sprintf(action_log, "&R[&CBeeler&R]&w Vnum %d does not exist. Cannot create mob.\n\r", vnum);
                send_to_char(action_log, ch);
            }
            else
            {
                mob = create_mobile(pMobIndex);
                if (mob)
                {
                    char_to_room(mob, ch->in_room);
                    sprintf(action_log, "&G[&CBeeler&G]&w Mob '%s' (vnum %d) manifested into reality!\n\r",
                            mob->short_descr, vnum);
                    send_to_char(action_log, ch);
                    act(AT_MAGIC, "$n's form shimmers into existence!", mob, NULL, NULL, TO_ROOM);
                    action_executed = TRUE;

                    /* Auto-assign personality when Beeler creates a mob */
                    beeler_assign_personality(mob, ch);
                }
            }
        }
    }

    /* ===== ACTION PATTERN: Assign Profession ===== */
    if ((strstr(lower_response, "assign") && strstr(lower_response, "profession")) ||
        (strstr(lower_response, "make") && strstr(lower_response, "profession")))
    {
        /* Try to extract profession name */
        if (sscanf(lower_response, "%*s %s %*s %s", arg1, arg2) >= 2)
        {
            int prof = get_profession_by_name(arg2);
            if (prof != PROF_NONE)
            {
                /* Find target mob */
                mob = get_char_world(ch, arg1);
                if (mob && IS_NPC(mob))
                {
                    set_npc_profession(mob, prof);
                    sprintf(action_log, "&G[&CBeeler&G]&w %s is now a %s!\n\r",
                            mob->short_descr, profession_name(prof));
                    send_to_char(action_log, ch);
                    action_executed = TRUE;
                }
            }
        }
    }

    /* ===== ACTION PATTERN: Trigger Economic Event ===== */
    if (strstr(lower_response, "plague") || strstr(lower_response, "drought") ||
        strstr(lower_response, "bumper crop") || strstr(lower_response, "mine collapse") ||
        strstr(lower_response, "wolf attack") || strstr(lower_response, "discovery"))
    {
        int event_type = EVENT_NONE;

        if (strstr(lower_response, "plague")) event_type = EVENT_PLAGUE;
        else if (strstr(lower_response, "drought")) event_type = EVENT_DROUGHT;
        else if (strstr(lower_response, "bumper crop")) event_type = EVENT_BUMPER_CROP;
        else if (strstr(lower_response, "mine collapse")) event_type = EVENT_MINE_COLLAPSE;
        else if (strstr(lower_response, "wolf attack")) event_type = EVENT_WOLF_ATTACK;
        else if (strstr(lower_response, "discovery")) event_type = EVENT_DISCOVERY;

        if (event_type != EVENT_NONE)
        {
            /* Try to find area name in response */
            area = beeler_find_area_by_name(lower_response);
            if (!area)
                area = ch->in_room->area; /* Default to current area */

            trigger_specific_event(get_area_economy(area), event_type);
            sprintf(action_log, "&R[&CBeeler&R]&w Economic event triggered in %s!\n\r", area->name);
            send_to_char(action_log, ch);
            action_executed = TRUE;
        }
    }

    /* ===== ACTION PATTERN: Adjust Economy ===== */
    if ((strstr(lower_response, "adjust") || strstr(lower_response, "modify")) &&
        (strstr(lower_response, "supply") || strstr(lower_response, "demand") ||
         strstr(lower_response, "price")))
    {
        /* Parse: "adjust supply of grain by 1000" */
        if (sscanf(lower_response, "%*s %*s %*s %s %*s %d", arg1, &amount) >= 2)
        {
            int resource = get_resource_by_name(arg1);
            if (resource != -1)
            {
                area = ch->in_room->area;
                AREA_ECONOMY *econ = get_area_economy(area);

                if (strstr(lower_response, "supply"))
                {
                    econ->supply[resource] += amount;
                    sprintf(action_log, "&G[&CBeeler&G]&w Adjusted %s supply by %d in %s\n\r",
                            resource_name(resource), amount, area->name);
                }
                else if (strstr(lower_response, "demand"))
                {
                    econ->demand[resource] += amount;
                    sprintf(action_log, "&G[&CBeeler&G]&w Adjusted %s demand by %d in %s\n\r",
                            resource_name(resource), amount, area->name);
                }

                send_to_char(action_log, ch);
                update_prices(econ);
                action_executed = TRUE;
            }
        }
    }

    /* ===== ACTION PATTERN: Grant Awareness ===== */
    if ((strstr(lower_response, "grant") && strstr(lower_response, "awareness")) ||
        (strstr(lower_response, "awaken") && strstr(lower_response, "consciousness")))
    {
        /* Extract target from response */
        sscanf(lower_response, "%*s %*s %s", arg1);
        mob = get_char_world(ch, arg1);

        if (mob && IS_NPC(mob))
        {
            MOB_IDENTITY *identity = get_mob_identity(mob->pIndexData->vnum);
            if (!identity)
            {
                beeler_assign_personality(mob, ch);
                send_to_char("&G[&CBeeler&G]&w Consciousness granted. A new soul awakens.\n\r", ch);
                act(AT_MAGIC, "$n suddenly becomes aware of $mself for the first time!", mob, NULL, NULL, TO_ROOM);
                action_executed = TRUE;
            }
            else
            {
                /* Increase awareness level */
                if (identity->awareness_level < AWARENESS_HIGH)
                {
                    identity->awareness_level++;
                    sprintf(action_log, "&G[&CBeeler&G]&w %s's awareness expanded to level %d\n\r",
                            mob->short_descr, identity->awareness_level);
                    send_to_char(action_log, ch);
                    /* TODO: Save identity to disk */
                    action_executed = TRUE;
                }
            }
        }
    }

    /* ===== ACTION PATTERN: Build District ===== */
    if (strstr(lower_response, "build") && strstr(lower_response, "district"))
    {
        /* Extract district name */
        sscanf(lower_response, "%*s %*s %s", arg1);

        area = ch->in_room->area;
        sprintf(action_log, "&G[&CBeeler&G]&w Construction of district '%s' in %s initiated...\n\r", arg1, area->name);
        send_to_char(action_log, ch);
        send_to_char("&W[&CBeeler&W]&w (District construction not yet fully implemented)\n\r", ch);
        /* TODO: Call beeler_architect district generation */
        action_executed = TRUE;
    }

    /* Log the action */
    sprintf(log_buf, "[BEELER ACTION] %s requested: %s | Executed: %s",
            ch->name, response, action_executed ? "YES" : "NO");
    log_string(log_buf);

    if (!action_executed)
    {
        send_to_char("&W[&CBeeler&W]&w I have spoken, but my words alone shape reality.\n\r", ch);
        send_to_char("&W[&CBeeler&W]&w (No actionable commands detected in response)\n\r", ch);
    }

    DISPOSE(lower_response);
}

/*
 * Log Beeler interaction
 */
void log_beeler_interaction(CHAR_DATA *ch, char *request, char *response, bool action_taken)
{
    FILE *fp;
    BEELER_INTERACTION *interaction;
    time_t now = time(NULL);

    /* Create interaction record */
    CREATE(interaction, BEELER_INTERACTION, 1);
    interaction->timestamp = now;
    interaction->player_name = str_dup(ch->name);
    interaction->player_level = ch->level;
    interaction->request = str_dup(request);
    interaction->response = str_dup(response);
    interaction->action_taken = action_taken;
    interaction->request_granted = action_taken;  /* For now */
    interaction->next = first_interaction;
    first_interaction = interaction;

    /* Append to log file */
    fp = fopen(INTERACTION_LOG, "a");
    if (fp)
    {
        fprintf(fp, "\n=== %s ===\n", ctime(&now));
        fprintf(fp, "Player: %s (Level %d)\n", ch->name, ch->level);
        fprintf(fp, "Request: %s\n", request);
        fprintf(fp, "Response: %s\n", response);
        fprintf(fp, "Action Taken: %s\n", action_taken ? "YES" : "NO");
        fprintf(fp, "=====================================\n");
        fclose(fp);
    }
}

/*
 * Create snapshot
 */
WORLD_SNAPSHOT *create_snapshot(char *name, char *reason, char *created_by)
{
    WORLD_SNAPSHOT *snapshot;
    char timestamp[64];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", tm_info);

    CREATE(snapshot, WORLD_SNAPSHOT, 1);

    /* Generate unique name if not provided */
    if (!name || name[0] == '\0')
    {
        static char auto_name[256];
        sprintf(auto_name, "auto-snapshot-%s", timestamp);
        snapshot->snapshot_name = str_dup(auto_name);
    }
    else
    {
        snapshot->snapshot_name = str_dup(name);
    }

    snapshot->created = now;
    snapshot->created_by = str_dup(created_by);
    snapshot->reason = str_dup(reason);
    snapshot->can_restore = TRUE;
    snapshot->is_active = FALSE;

    /* Perform backup */
    if (backup_world_state(snapshot))
    {
        sprintf(log_buf, "[BEELER] Snapshot created: %s (by %s)",
            snapshot->snapshot_name, created_by);
        log_string(log_buf);

        /* Add to list */
        snapshot->next = first_snapshot;
        first_snapshot = snapshot;

        return snapshot;
    }
    else
    {
        log_string("[BEELER] ERROR: Snapshot creation failed");
        DISPOSE(snapshot);
        return NULL;
    }
}

/*
 * Backup world state
 */
bool backup_world_state(WORLD_SNAPSHOT *snapshot)
{
    char command[512];
    char snapshot_path[256];

    sprintf(snapshot_path, "%s%s/", SNAPSHOT_DIR, snapshot->snapshot_name);

    /* Create snapshot directory */
    sprintf(command, "mkdir -p %s", snapshot_path);
    system(command);

    /* Backup area files */
    sprintf(command, "cp -r ../area/*.are %s", snapshot_path);
    system(command);

    /* Backup mob data */
    sprintf(command, "cp -r ../data/mob_identity/* %s", snapshot_path);
    system(command);
    sprintf(command, "cp -r ../data/mob_memory/* %s", snapshot_path);
    system(command);

    /* Store paths */
    snapshot->area_files_backup = str_dup(snapshot_path);

    sprintf(log_buf, "[BEELER] World state backed up to: %s", snapshot_path);
    log_string(log_buf);

    return TRUE;
}

/*
 * Update Beeler's mood
 */
void beeler_update_mood(CHAR_DATA *ch, char *request)
{
    /* TODO: Implement mood changes based on request patterns */
    /* For now, stay benevolent */
}

/*
 * Get mood name
 */
char *beeler_mood_name(int mood)
{
    switch (mood)
    {
        case BEELER_MOOD_BENEVOLENT: return "Benevolent";
        case BEELER_MOOD_NEUTRAL:    return "Neutral";
        case BEELER_MOOD_STERN:      return "Stern";
        case BEELER_MOOD_WRATHFUL:   return "Wrathful";
        case BEELER_MOOD_PLAYFUL:    return "Playful";
        default:                     return "Unknown";
    }
}

/*
 * Load Beeler state
 */
void load_beeler_state(void)
{
    /* TODO: Load snapshots and interaction history */
    log_string("  - Loading Beeler state... (TODO)");
}

/*
 * Save Beeler state
 */
void save_beeler_state(void)
{
    /* TODO: Save current state */
}

/*
 * Command: beeler stats
 */
void do_beeler_stats(CHAR_DATA *ch, char *argument)
{
    BEELER_INTERACTION *interaction;
    WORLD_SNAPSHOT *snapshot;
    int total_interactions = 0;
    int total_snapshots = 0;

    /* Count interactions */
    for (interaction = first_interaction; interaction; interaction = interaction->next)
        total_interactions++;

    /* Count snapshots */
    for (snapshot = first_snapshot; snapshot; snapshot = snapshot->next)
        total_snapshots++;

    send_to_char("\n\r&Y=== Beeler - Living AI God Statistics ===&w\n\r\n\r", ch);

    sprintf(log_buf, "Current Mood: %s\n\r", beeler_mood_name(beeler_current_mood));
    send_to_char(log_buf, ch);

    sprintf(log_buf, "Total Interactions: %d\n\r", total_interactions);
    send_to_char(log_buf, ch);

    sprintf(log_buf, "Total Snapshots: %d\n\r", total_snapshots);
    send_to_char(log_buf, ch);

    if (beeler_last_action_time > 0)
    {
        sprintf(log_buf, "Last Action: %s\n\r", ctime(&beeler_last_action_time));
        send_to_char(log_buf, ch);
    }

    send_to_char("\n\r&Y\"I watch. I listen. I act when the time is right.\"&w\n\r\n\r", ch);
}

/*
 * Command: snapshot management
 */
void do_snapshot(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    WORLD_SNAPSHOT *snapshot;

    if (ch->level < LEVEL_IMMORTAL)
    {
        send_to_char("Only immortals can manage snapshots.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char("Snapshot commands:\n\r", ch);
        send_to_char("  snapshot create <name> <reason> - Create snapshot\n\r", ch);
        send_to_char("  snapshot list                   - List all snapshots\n\r", ch);
        send_to_char("  snapshot restore <name>         - Restore snapshot\n\r", ch);
        send_to_char("  snapshot delete <name>          - Delete snapshot\n\r", ch);
        return;
    }

    if (!str_cmp(arg1, "create"))
    {
        if (arg2[0] == '\0')
        {
            send_to_char("Specify a snapshot name.\n\r", ch);
            return;
        }

        snapshot = create_snapshot(arg2, argument, ch->name);
        if (snapshot)
        {
            send_to_char("&GSnapshot created successfully.&w\n\r", ch);
        }
        else
        {
            send_to_char("&RSnapshot creation failed.&w\n\r", ch);
        }
    }
    else if (!str_cmp(arg1, "list"))
    {
        send_to_char("\n\r&Y=== World Snapshots ===&w\n\r\n\r", ch);

        if (!first_snapshot)
        {
            send_to_char("No snapshots exist.\n\r", ch);
            return;
        }

        for (snapshot = first_snapshot; snapshot; snapshot = snapshot->next)
        {
            sprintf(log_buf, "&C%s&w\n\r", snapshot->snapshot_name);
            send_to_char(log_buf, ch);

            sprintf(log_buf, "  Created: %s", ctime(&snapshot->created));
            send_to_char(log_buf, ch);

            sprintf(log_buf, "  By: %s\n\r", snapshot->created_by);
            send_to_char(log_buf, ch);

            sprintf(log_buf, "  Reason: %s\n\r", snapshot->reason);
            send_to_char(log_buf, ch);

            send_to_char("\n\r", ch);
        }
    }
    else
    {
        send_to_char("Invalid snapshot command.\n\r", ch);
    }
}

/*
 * Call Ollama with Beeler personality (placeholder)
 */
char *call_ollama_beeler(char *prompt)
{
    /* TODO: Implement actual Ollama call */
    /* This will be similar to call_ollama_advanced() but with Beeler context */
    return str_dup("Beeler response placeholder");
}
