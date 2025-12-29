/*****************************************************************************
 * Periodicos - Smart News/Announcement System
 *
 * Not spam - only newsworthy events get announced. Different channels for
 * different importance levels: newspapers, town criers, rumors.
 *
 * Integration: world_history_tracker.h for event sourcing
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "periodicos.h"
#include "world_history_tracker.h"
#include "ollama_integration.h"

/* Global state */
PERIODICO_PUBLICATION *first_publication = NULL;
int total_publications = 0;

PERIODICO_ARTICLE *recent_articles = NULL;
int total_articles = 0;

ANNOUNCEMENT_FILTER global_filter;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void init_periodicos(void)
{
    log_string("Initializing Periodicos System...");

    first_publication = NULL;
    total_publications = 0;
    recent_articles = NULL;
    total_articles = 0;

    /* Initialize announcement filter */
    global_filter.announcements_last_hour = 0;
    global_filter.last_announcement_time = time(NULL);
    global_filter.max_announcements_per_hour = 6; /* One every 10 min */
    global_filter.min_priority_for_chat = ANNOUNCE_PRIORITY_HIGH;

    memset(global_filter.recent_topics, 0, sizeof(global_filter.recent_topics));

    log_string("Periodicos System initialized.");
}

void load_periodicos(void)
{
    FILE *fp;
    char filename[256];

    sprintf(filename, "%s%s", SYSTEM_DIR, "periodicos.dat");

    if ((fp = fopen(filename, "r")) == NULL)
    {
        log_string("No saved periodicos to load.");
        return;
    }

    log_string("Loading periodicos...");
    /* Load publications and articles */
    fclose(fp);
}

void save_periodicos(void)
{
    FILE *fp;
    char filename[256];
    PERIODICO_ARTICLE *article;

    sprintf(filename, "%s%s", SYSTEM_DIR, "periodicos.dat");

    if ((fp = fopen(filename, "w")) == NULL)
    {
        log_string("ERROR: Cannot save periodicos!");
        return;
    }

    fprintf(fp, "#PERIODICOS\n");

    for (article = recent_articles; article; article = article->next)
    {
        fprintf(fp, "ArticleID %d\n", article->article_id);
        fprintf(fp, "Published %ld\n", article->published_time);
        fprintf(fp, "Category %d\n", article->category);
        fprintf(fp, "Priority %d\n", article->priority);
        fprintf(fp, "Headline~ %s~\n", article->headline);
        fprintf(fp, "Body~ %s~\n", article->body);
        fprintf(fp, "Location~ %s~\n", article->location);
        fprintf(fp, "Author~ %s~\n", article->author);
        fprintf(fp, "Views %d\n", article->views);
        fprintf(fp, "End\n\n");
    }

    fprintf(fp, "#END\n");
    fclose(fp);
}

/*****************************************************************************
 * Publication Management
 *****************************************************************************/

PERIODICO_PUBLICATION *create_publication(char *city_name, int area_vnum, char *publication_name)
{
    PERIODICO_PUBLICATION *pub;

    CREATE(pub, PERIODICO_PUBLICATION, 1);
    pub->publication_name = str_dup(publication_name);
    pub->city_name = str_dup(city_name);
    pub->area_vnum = area_vnum;
    pub->first_article = NULL;
    pub->num_articles = 0;
    pub->editorial_tone = str_dup("formal");
    pub->next = first_publication;
    first_publication = pub;
    total_publications++;

    sprintf(log_buf, "PERIODICOS: Created publication '%s' for %s", publication_name, city_name);
        log_string(log_buf);

    return pub;
}

PERIODICO_PUBLICATION *find_publication_by_city(char *city_name)
{
    PERIODICO_PUBLICATION *pub;

    for (pub = first_publication; pub; pub = pub->next)
    {
        if (!str_cmp(pub->city_name, city_name))
            return pub;
    }

    /* Create default publication if not found */
    return create_publication(city_name, 0, "The City Chronicle");
}

PERIODICO_PUBLICATION *find_nearest_publication(int room_vnum)
{
    /* Find nearest publication based on room location */
    /* For now, return first available */
    if (!first_publication)
        return create_publication("World", 0, "The World News");

    return first_publication;
}

/*****************************************************************************
 * Article Creation
 *****************************************************************************/

PERIODICO_ARTICLE *create_article(char *headline, char *body, int category, int priority, char *location)
{
    PERIODICO_ARTICLE *article;
    static int next_article_id = 1;

    CREATE(article, PERIODICO_ARTICLE, 1);
    article->article_id = next_article_id++;
    article->published_time = time(NULL);
    article->category = category;
    article->priority = priority;
    article->headline = str_dup(headline);
    article->body = str_dup(body);
    article->location = str_dup(location);
    article->author = str_dup("The Chronicle");
    article->announced_in_chat = FALSE;
    article->views = 0;
    article->next = recent_articles;
    recent_articles = article;
    total_articles++;

    sprintf(log_buf, "PERIODICOS: Created article #%d: %s", article->article_id, headline);
        log_string(log_buf);

    return article;
}

void publish_article(PERIODICO_PUBLICATION *pub, PERIODICO_ARTICLE *article)
{
    if (!pub || !article)
        return;

    /* Add article to publication */
    article->author = str_dup(pub->publication_name);
    pub->num_articles++;

    sprintf(log_buf, "PERIODICOS: Published article in %s: %s", pub->publication_name, article->headline);
        log_string(log_buf);
}

/*****************************************************************************
 * Smart Announcement System - THE CORE FEATURE
 *****************************************************************************/

bool should_announce_in_chat(PERIODICO_ARTICLE *article)
{
    time_t now = time(NULL);
    int i;

    /* CRITICAL events ALWAYS go to chat */
    if (article->priority == ANNOUNCE_PRIORITY_CRITICAL)
        return TRUE;

    /* SILENT events NEVER go to chat */
    if (article->priority >= ANNOUNCE_PRIORITY_SILENT)
        return FALSE;

    /* Check rate limiting */
    if (difftime(now, global_filter.last_announcement_time) > 3600)
    {
        /* Reset hourly counter */
        global_filter.announcements_last_hour = 0;
        global_filter.last_announcement_time = now;
    }

    /* Too many announcements this hour? */
    if (global_filter.announcements_last_hour >= global_filter.max_announcements_per_hour)
    {
        sprintf(log_buf, "PERIODICOS: Rate limit - no chat announcement for: %s", article->headline);
        log_string(log_buf);
        return FALSE;
    }

    /* Check priority threshold */
    if (article->priority > global_filter.min_priority_for_chat)
    {
        return FALSE;
    }

    /* Check for duplicate topics */
    for (i = 0; i < 10; i++)
    {
        if (global_filter.recent_topics[i] &&
            strstr(article->headline, global_filter.recent_topics[i]))
        {
            sprintf(log_buf, "PERIODICOS: Duplicate topic - no chat announcement: %s", article->headline);
        log_string(log_buf);
            return FALSE;
        }
    }

    /* Passed all checks */
    return TRUE;
}

char *format_announcement_for_chat(PERIODICO_ARTICLE *article)
{
    static char formatted[MAX_STRING_LENGTH];
    char *category_name = "NEWS";

    switch (article->category)
    {
        case EVENT_CATEGORY_WAR:         category_name = "WAR"; break;
        case EVENT_CATEGORY_DIPLOMACY:   category_name = "DIPLOMACY"; break;
        case EVENT_CATEGORY_CONSTRUCTION:category_name = "CONSTRUCTION"; break;
        case EVENT_CATEGORY_TRADE:       category_name = "TRADE"; break;
        case EVENT_CATEGORY_CULTURE:     category_name = "CULTURE"; break;
        case EVENT_CATEGORY_ECONOMY:     category_name = "ECONOMY"; break;
        case EVENT_CATEGORY_DISASTER:    category_name = "DISASTER"; break;
        case EVENT_CATEGORY_DISCOVERY:   category_name = "DISCOVERY"; break;
    }

    if (article->priority == ANNOUNCE_PRIORITY_CRITICAL)
    {
        sprintf(formatted, "&R[!!! %s !!!]&w &Y%s&w", category_name, article->headline);
    }
    else if (article->priority == ANNOUNCE_PRIORITY_HIGH)
    {
        sprintf(formatted, "&G[%s]&w %s", category_name, article->headline);
    }
    else
    {
        sprintf(formatted, "&c[%s]&w %s", category_name, article->headline);
    }

    return formatted;
}

void announce_article(PERIODICO_ARTICLE *article)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int i;

    if (!article)
        return;

    /* Decide if this goes to chat */
    if (should_announce_in_chat(article))
    {
        /* Format for chat */
        strcpy(buf, format_announcement_for_chat(article));

        /* Send to all players */
        for (d = first_descriptor; d; d = d->next)
        {
            if (d->connected == CON_PLAYING && d->character)
            {
                send_to_char(buf, d->character);
                send_to_char("\r\n", d->character);
            }
        }

        /* Update rate limit */
        global_filter.announcements_last_hour++;
        article->announced_in_chat = TRUE;

        /* Add to recent topics (prevent spam) */
        for (i = 9; i > 0; i--)
            global_filter.recent_topics[i] = global_filter.recent_topics[i-1];
        global_filter.recent_topics[0] = str_dup(article->headline);

        sprintf(log_buf, "PERIODICOS: Announced in chat: %s", article->headline);
        log_string(log_buf);
    }
    else
    {
        /* Only in periodicos, no chat */
        sprintf(log_buf, "PERIODICOS: Periodicos only (no chat): %s", article->headline);
        log_string(log_buf);
    }
}

void smart_announce(char *headline, char *body, int category, int priority, char *location)
{
    PERIODICO_ARTICLE *article;
    PERIODICO_PUBLICATION *pub;

    /* Create article */
    article = create_article(headline, body, category, priority, location);

    /* Find appropriate publication */
    pub = find_publication_by_city(location);
    if (pub)
        publish_article(pub, article);

    /* Smart announcement decision */
    announce_article(article);

    /* Record in world history */
    record_world_event(EVENT_PLAYER_ACTION, headline, priority <= ANNOUNCE_PRIORITY_HIGH ? 8 : 5);
}

/*****************************************************************************
 * AI-Generated Content
 *****************************************************************************/

char *ai_generate_article_body(char *headline, char *context, char *editorial_tone)
{
    char prompt[MAX_STRING_LENGTH * 2];
    char *result;

    sprintf(prompt,
        "Write a newspaper article with headline: '%s'. "
        "Context: %s. "
        "Editorial tone: %s. "
        "Write 2-3 paragraphs in a medieval fantasy newspaper style. "
        "Keep it under 300 words.",
        headline, context, editorial_tone);

    if (ollama_is_available())
    {
        result = ollama_request(prompt, 300);
        if (result)
            return result;
    }

    /* Fallback */
    return str_dup(context);
}

char *ai_generate_editorial_response(char *event, char *city_name, char *leader_personality)
{
    char prompt[MAX_STRING_LENGTH * 2];
    char *result;

    sprintf(prompt,
        "Write a brief editorial response to this event: '%s' "
        "from the perspective of %s's newspaper. "
        "The local leader is: %s. "
        "Write 1-2 paragraphs expressing local sentiment.",
        event, city_name, leader_personality);

    if (ollama_is_available())
    {
        result = ollama_request(prompt, 200);
        if (result)
            return result;
    }

    return str_dup("The citizens await further developments.");
}

/*****************************************************************************
 * Player Reading Interface
 *****************************************************************************/

void show_recent_news(CHAR_DATA *ch, int num_articles)
{
    PERIODICO_ARTICLE *article;
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    send_to_char("&c=== Recent News ===&w\n\r\n\r", ch);

    for (article = recent_articles; article && count < num_articles; article = article->next)
    {
        sprintf(buf, "&Y[%d]&w %s\n\r     &c%s&w - %d views\n\r\n\r",
                article->article_id,
                article->headline,
                article->location,
                article->views);
        send_to_char(buf, ch);
        count++;
    }

    if (count == 0)
        send_to_char("No recent news available.\n\r", ch);
}

void show_news_by_category(CHAR_DATA *ch, int category)
{
    PERIODICO_ARTICLE *article;
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    send_to_char("&c=== News by Category ===&w\n\r\n\r", ch);

    for (article = recent_articles; article; article = article->next)
    {
        if (article->category == category)
        {
            sprintf(buf, "&Y[%d]&w %s\n\r", article->article_id, article->headline);
            send_to_char(buf, ch);
            count++;
        }
    }

    if (count == 0)
        send_to_char("No news in this category.\n\r", ch);
}

void read_article(CHAR_DATA *ch, int article_id)
{
    PERIODICO_ARTICLE *article;
    char buf[MAX_STRING_LENGTH];

    for (article = recent_articles; article; article = article->next)
    {
        if (article->article_id == article_id)
        {
            sprintf(buf, "&B=====================================&w\n\r");
            send_to_char(buf, ch);
            sprintf(buf, "&Y%s&w\n\r", article->headline);
            send_to_char(buf, ch);
            sprintf(buf, "&c%s - %s&w\n\r", article->location, article->author);
            send_to_char(buf, ch);
            sprintf(buf, "&B=====================================&w\n\r\n\r");
            send_to_char(buf, ch);
            sprintf(buf, "%s\n\r", article->body);
            send_to_char(buf, ch);

            article->views++;
            return;
        }
    }

    send_to_char("Article not found.\n\r", ch);
}

/*****************************************************************************
 * Commands
 *****************************************************************************/

void do_periodnews(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        show_recent_news(ch, 10);
        return;
    }

    if (is_number(arg))
    {
        read_article(ch, atoi(arg));
        return;
    }

    send_to_char("Usage: news [article_id]\n\r", ch);
}

void do_periodicos(CHAR_DATA *ch, char *argument)
{
    PERIODICO_PUBLICATION *pub;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    if (!IS_IMMORTAL(ch))
    {
        do_periodnews(ch, argument);
        return;
    }

    send_to_char("&c=== Periodicos System Status ===&w\n\r\n\r", ch);

    sprintf(buf, "Total publications: %d\n\r", total_publications);
    send_to_char(buf, ch);
    sprintf(buf, "Total articles: %d\n\r", total_articles);
    send_to_char(buf, ch);
    sprintf(buf, "Announcements this hour: %d/%d\n\r\n\r",
            global_filter.announcements_last_hour,
            global_filter.max_announcements_per_hour);
    send_to_char(buf, ch);

    send_to_char("&GPublications:&w\n\r", ch);
    for (pub = first_publication; pub; pub = pub->next)
    {
        sprintf(buf, "  %s (%s) - %d articles\n\r",
                pub->publication_name, pub->city_name, pub->num_articles);
        send_to_char(buf, ch);
    }
}
