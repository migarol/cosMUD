/*****************************************************************************
 * AI God Context Analyzer
 *
 * Deep analysis of mob context before generating personality.
 * Analyzes: location, items, lore, relationships, area, fame, etc.
 *
 * Prevents breaking the world by understanding existing context.
 *****************************************************************************/

#ifndef AI_CONTEXT_ANALYZER_H
#define AI_CONTEXT_ANALYZER_H

/* Forward declarations */
struct mob_identity_data;
typedef struct mob_identity_data MOB_IDENTITY;

/* Context analysis result */
typedef struct mob_context_analysis {
    /* Basic info */
    int mob_vnum;
    char *mob_name;
    char *short_desc;
    char *long_desc;

    /* Location analysis */
    char *area_name;
    int area_vnum;
    char *area_type;         /* "city", "dungeon", "forest", "castle" */
    char *room_name;
    char *room_desc;
    int room_vnum;

    /* Importance analysis */
    bool is_unique;          /* Only one in the world */
    bool is_boss;            /* Boss mob */
    bool is_shopkeeper;      /* Runs a shop */
    bool is_quest_giver;     /* Gives quests */
    bool is_guard;           /* Guard/sentinel */
    int fame_level;          /* 0-100 how famous/important */

    /* Items analysis */
    int num_items;
    char **item_names;       /* Items the mob carries */
    bool has_unique_items;   /* Has unique/legendary items */
    char *why_these_items;   /* AI-generated reason for items */

    /* Lore analysis */
    bool mentioned_in_books; /* Is this mob mentioned in any books? */
    char **book_mentions;    /* Books that mention this mob */
    int num_mentions;
    char *existing_lore;     /* Lore from room/area descriptions */

    /* Relationship analysis */
    int nearby_mobs_count;
    char **nearby_mob_names;
    char *mob_group_type;    /* "guards", "merchants", "scholars", etc */

    /* Behavioral hints */
    bool is_aggressive;
    bool is_friendly;
    bool stays_in_place;     /* ACT_SENTINEL */
    bool wanders;

    /* Coherence score */
    int coherence_score;     /* 0-100 how much lore exists */
    char *warnings;          /* Warnings about potential issues */

} MOB_CONTEXT;

/* Function declarations */
MOB_CONTEXT *analyze_mob_context(CHAR_DATA *mob);
void free_mob_context(MOB_CONTEXT *ctx);
char *generate_context_aware_prompt(MOB_CONTEXT *ctx);
bool validate_personality_fits_context(MOB_CONTEXT *ctx, MOB_IDENTITY *identity);
void search_books_for_mob(char *mob_name, MOB_CONTEXT *ctx);
void analyze_mob_items(CHAR_DATA *mob, MOB_CONTEXT *ctx);
void analyze_nearby_mobs(CHAR_DATA *mob, MOB_CONTEXT *ctx);
void determine_mob_importance(CHAR_DATA *mob, MOB_CONTEXT *ctx);
char *infer_area_type(AREA_DATA *area);

#endif /* AI_CONTEXT_ANALYZER_H */
