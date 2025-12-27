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
#define OLLAMA_MODEL "llama3.2:latest"
#define OLLAMA_TIMEOUT 5  /* seconds */

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
char *ollama_chat(char *system_prompt, char *user_prompt, int max_tokens);

/* Utility */
char *ollama_escape_json(char *str);
bool ollama_is_available(void);
char *ollama_get_status(void);

/* Commands */
void do_ollama(CHAR_DATA *ch, char *argument);

#endif /* OLLAMA_INTEGRATION_H */
