/*****************************************************************************
 * Periodicos - In-Game Newspaper System
 *
 * Not everything needs to be announced in real-time chat. Some events are
 * published in periodicos (newspapers) that players can read at their leisure.
 *
 * "Not all news is urgent. Some stories unfold slowly, like dawn."
 *****************************************************************************/

#ifndef PERIODICOS_H
#define PERIODICOS_H

/* Announcement priority levels */
#define ANNOUNCE_PRIORITY_CRITICAL   0  /* World chat immediately (wars, disasters) */
#define ANNOUNCE_PRIORITY_HIGH       1  /* World chat + periodicos (major construction, trade) */
#define ANNOUNCE_PRIORITY_MEDIUM     2  /* Periodicos only (minor events, NPC marriages) */
#define ANNOUNCE_PRIORITY_LOW        3  /* Periodicos archive only (daily routine) */
#define ANNOUNCE_PRIORITY_SILENT     4  /* Never announced (internal bookkeeping) */

/* Event categories */
#define EVENT_CATEGORY_WAR           0
#define EVENT_CATEGORY_DIPLOMACY     1
#define EVENT_CATEGORY_CONSTRUCTION  2
#define EVENT_CATEGORY_TRADE         3
#define EVENT_CATEGORY_CULTURE       4
#define EVENT_CATEGORY_ECONOMY       5
#define EVENT_CATEGORY_DISASTER      6
#define EVENT_CATEGORY_DISCOVERY     7
#define EVENT_CATEGORY_BIRTH_DEATH   8
#define EVENT_CATEGORY_MISC          9

/* Newspaper article */
typedef struct periodico_article {
    int article_id;
    time_t published_time;
    int category;
    int priority;

    char *headline;         /* "King Aldric Orders Construction of Ironforge Fortress" */
    char *body;            /* Full article text */
    char *location;        /* "DarkHaven" */
    char *author;          /* "The DarkHaven Chronicle" */

    bool announced_in_chat;  /* Was this also in world chat? */
    int views;              /* How many players read it */

    struct periodico_article *next;
} PERIODICO_ARTICLE;

/* Newspaper publication (like "The DarkHaven Chronicle") */
typedef struct periodico_publication {
    char *publication_name;  /* "The DarkHaven Chronicle" */
    char *city_name;         /* "DarkHaven" */
    int area_vnum;

    /* Recent articles (last 30 days) */
    PERIODICO_ARTICLE *first_article;
    int num_articles;

    /* Publication style */
    char *editorial_tone;    /* "formal", "casual", "propaganda" */

    struct periodico_publication *next;
} PERIODICO_PUBLICATION;

/* Global announcement filtering */
typedef struct announcement_filter {
    /* Spam prevention */
    int announcements_last_hour;
    time_t last_announcement_time;
    int max_announcements_per_hour;  /* Default: 6 (one every 10 min) */

    /* Priority thresholds */
    int min_priority_for_chat;       /* Default: ANNOUNCE_PRIORITY_HIGH */

    /* Recent announcements (to avoid duplicates) */
    char *recent_topics[10];

} ANNOUNCEMENT_FILTER;

/* Function declarations */

/* Initialization */
void init_periodicos(void);
void load_periodicos(void);
void save_periodicos(void);

/* Publication management */
PERIODICO_PUBLICATION *create_publication(char *city_name, int area_vnum, char *publication_name);
PERIODICO_PUBLICATION *find_publication_by_city(char *city_name);
PERIODICO_PUBLICATION *find_nearest_publication(int room_vnum);

/* Article creation */
PERIODICO_ARTICLE *create_article(char *headline, char *body, int category, int priority, char *location);
void publish_article(PERIODICO_PUBLICATION *pub, PERIODICO_ARTICLE *article);
void announce_article(PERIODICO_ARTICLE *article);  /* Intelligently announce based on priority */

/* Smart announcement system */
bool should_announce_in_chat(PERIODICO_ARTICLE *article);
void smart_announce(char *headline, char *body, int category, int priority, char *location);
char *format_announcement_for_chat(PERIODICO_ARTICLE *article);

/* Player reading interface */
void show_recent_news(CHAR_DATA *ch, int num_articles);
void show_news_by_category(CHAR_DATA *ch, int category);
void show_publication_list(CHAR_DATA *ch);
void read_article(CHAR_DATA *ch, int article_id);

/* AI-generated content */
char *ai_generate_article_body(char *headline, char *context, char *editorial_tone);
char *ai_generate_editorial_response(char *event, char *city_name, char *leader_personality);

/* Announcement example functions - implemented in periodicos.c */
void announce_war_declaration(char *aggressor, char *target);
void announce_construction(char *city, char *project_name, char *leader_name);
void announce_npc_marriage(char *npc1, char *npc2, char *city);
void log_market_prices(char *city, int resource_id, int price);

/* Commands */
void do_news(CHAR_DATA *ch, char *argument);      /* Read recent news */
void do_periodicos(CHAR_DATA *ch, char *argument); /* Browse newspapers */

#endif /* PERIODICOS_H */
