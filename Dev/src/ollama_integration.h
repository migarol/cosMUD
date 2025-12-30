/*****************************************************************************
 * Ollama AI Integration
 *
 * Beeler uses Ollama (local LLM) to generate rich, context-aware descriptions.
 * When Ollama is unavailable, falls back to template-based generation.
 *
 * "Let the machines dream of rooms..."
 *****************************************************************************/

#ifndef OLLAMA_INTEGRATION_H
#define OLLAMA_INTEGRATION_H

/* Ollama configuration */
#define OLLAMA_HOST "http://localhost"
#define OLLAMA_PORT 11434

/* Multi-Tier AI System - Different models for different needs */

/* TIER 0: Beeler (God AI) - World Architecture */
#define OLLAMA_MODEL_BEELER      "qwen2.5:7b"    /* Quality > Speed, background only */
#define OLLAMA_TIMEOUT_BEELER    30              /* Can wait - it's background work */

/* TIER 1: Leader Thinking - Strategic Decisions */
#define OLLAMA_MODEL_THINKING    "phi3:mini"     /* Fast reasoning, background */
#define OLLAMA_TIMEOUT_THINKING  3               /* Target <100ms in practice */

/* TIER 2: Leader Speaking - Real-time Dialogue */
#define OLLAMA_MODEL_SPEAKING    "tinyllama"     /* Ultra fast, <30ms */
#define OLLAMA_TIMEOUT_SPEAKING  1               /* Must be instant */

/* TIER 3: Normal NPCs - Simple dialogue */
#define OLLAMA_MODEL_NPC         "tinyllama"     /* Same as speaking */
#define OLLAMA_TIMEOUT_NPC       1

/* Default model (for backward compatibility) */
#define OLLAMA_MODEL             OLLAMA_MODEL_SPEAKING
#define OLLAMA_TIMEOUT           OLLAMA_TIMEOUT_SPEAKING

/* NPC AI Tiers - determines which model to use */
#define NPC_AI_TIER_GOD          0  /* Beeler - world creation, background */
#define NPC_AI_TIER_LEADER       1  /* Leaders - thinking + speaking */
#define NPC_AI_TIER_IMPORTANT    2  /* Merchants, quest givers - speaking only */
#define NPC_AI_TIER_NORMAL       3  /* Residents - templates with AI fallback */
#define NPC_AI_TIER_SIMPLE       4  /* Guards, vendors - templates only */

/* AI Usage Policy - when to use AI vs templates */
#define OLLAMA_USE_FOR_NPCS       TRUE   /* NOW enabled - we have fast models */
#define OLLAMA_USE_FOR_ROOMS      FALSE  /* Templates are faster for gameplay */
#define OLLAMA_USE_FOR_BOOKS      TRUE   /* Books can wait, use AI for quality */
#define OLLAMA_USE_FOR_CULTURE    TRUE   /* Background evolution can use AI */
#define OLLAMA_USE_FOR_HISTORY    TRUE   /* Historical content can use AI */

/* Generation types */
#define OLLAMA_GEN_ROOM_DESC      0
#define OLLAMA_GEN_HOME_DESC      1
#define OLLAMA_GEN_MOB_PERSONALITY 2
#define OLLAMA_GEN_DISTRICT_DESC  3
#define OLLAMA_GEN_INN_DESC       4

/* Ollama status */
extern bool ollama_enabled;
extern char *ollama_last_error;

/* Function declarations */

/* Initialization */
void init_ollama(void);
bool ollama_test_connection(void);
void ollama_shutdown(void);

/* Generation functions */
char *ollama_generate_room_description(int home_type, char *owner_name, char *context);
char *ollama_generate_mob_personality(char *mob_name, char *race, int level, char *role);
char *ollama_generate_district_description(char *district_name, int home_type, char *city_name);
char *ollama_generate_inn_description(char *inn_name, char *location, int quality);

/* Low-level API */
char *ollama_request(char *prompt, int max_tokens);
char *ollama_request_with_model(char *prompt, int max_tokens, char *model, int timeout);
char *ollama_chat(char *system_prompt, char *user_prompt, int max_tokens);

/* Multi-tier NPC AI */
char *ollama_generate_npc_dialogue(CHAR_DATA *npc, char *player_message);
void leader_think_strategically(CHAR_DATA *leader);

/* Utility */
char *ollama_escape_json(char *str);
bool ollama_is_available(void);
char *ollama_get_status(void);

/* Commands */
void do_ollama(CHAR_DATA *ch, char *argument);

#endif /* OLLAMA_INTEGRATION_H */
