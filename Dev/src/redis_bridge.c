/**
 * Redis Bridge - Publish events from MUD to Redis
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "redis_bridge.h"

#ifdef HAVE_HIREDIS
#include <hiredis/hiredis.h>

/* Global Redis connection */
redisContext *redis_ctx = NULL;
#endif

/* Configuration - TODO: move to config file */
#define REDIS_HOST "localhost"
#define REDIS_PORT 6379
#define REDIS_PASSWORD "cosmud_redis_password"

/**
 * Initialize Redis connection
 */
void init_redis_bridge(void)
{
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    redis_ctx = redisConnectWithTimeout(REDIS_HOST, REDIS_PORT, timeout);

    if (redis_ctx == NULL || redis_ctx->err) {
        if (redis_ctx) {
            printf("Redis connection error: %s\n", redis_ctx->errstr);
            redisFree(redis_ctx);
            redis_ctx = NULL;
        } else {
            printf("Redis connection error: can't allocate redis context\n");
        }
        printf("WARNING: Redis not available - events will not be published to dashboard\n");
        return;
    }

    /* Authenticate if password is set */
    if (REDIS_PASSWORD && strlen(REDIS_PASSWORD) > 0) {
        redisReply *reply = (redisReply *)redisCommand(redis_ctx, "AUTH %s", REDIS_PASSWORD);
        if (reply == NULL) {
            printf("Redis AUTH failed: %s\n", redis_ctx->errstr);
            redisFree(redis_ctx);
            redis_ctx = NULL;
            return;
        }
        freeReplyObject(reply);
    }

    printf("✅ Redis bridge initialized - connected to %s:%d\n", REDIS_HOST, REDIS_PORT);
}

/**
 * Shutdown Redis connection
 */
void close_redis_bridge(void)
{
    if (redis_ctx) {
        redisFree(redis_ctx);
        redis_ctx = NULL;
        printf("Redis bridge closed\n");
    }
}

/**
 * Check if Redis is connected
 */
bool is_redis_connected(void)
{
    return (redis_ctx != NULL && !redis_ctx->err);
}

/**
 * Publish a message to a Redis channel
 */
void redis_publish(const char *channel, const char *message)
{
    if (!is_redis_connected()) {
        return; // Silently fail if not connected
    }

    redisReply *reply = (redisReply *)redisCommand(redis_ctx, "PUBLISH %s %s", channel, message);

    if (reply == NULL) {
        printf("Redis PUBLISH failed: %s\n", redis_ctx->errstr);
        return;
    }

    freeReplyObject(reply);
}

/**
 * Publish world event
 * Format: {"type":"kill","killer":"Thorin","victim":"Dragon",...}
 */
void redis_publish_world_event(const char *event_type, const char *json_data)
{
    char buffer[MSL];

    snprintf(buffer, sizeof(buffer),
             "{\"type\":\"%s\",\"timestamp\":\"%ld\",\"data\":%s}",
             event_type, (long)time(NULL), json_data);

    redis_publish("world.events", buffer);
}

/**
 * Publish AI decision
 */
void redis_publish_ai_decision(const char *leader_name, const char *decision, const char *reasoning)
{
    char buffer[MSL];
    char escaped_reasoning[MSL];

    /* Escape quotes in reasoning */
    int i, j = 0;
    for (i = 0; reasoning[i] && j < MSL - 2; i++) {
        if (reasoning[i] == '"') {
            escaped_reasoning[j++] = '\\';
        }
        escaped_reasoning[j++] = reasoning[i];
    }
    escaped_reasoning[j] = '\0';

    snprintf(buffer, sizeof(buffer),
             "{\"leader\":\"%s\",\"decision\":\"%s\",\"reasoning\":\"%s\",\"timestamp\":%ld}",
             leader_name, decision, escaped_reasoning, (long)time(NULL));

    redis_publish("ai.decisions", buffer);
}

/**
 * Publish economy update
 */
void redis_publish_economy_update(const char *area_name, const char *resource, int change)
{
    char buffer[MSL];

    snprintf(buffer, sizeof(buffer),
             "{\"area\":\"%s\",\"resource\":\"%s\",\"change\":%d,\"timestamp\":%ld}",
             area_name, resource, change, (long)time(NULL));

    redis_publish("economy.updates", buffer);
}

/**
 * Publish player action
 */
void redis_publish_player_action(const char *player_name, const char *action, int importance)
{
    char buffer[MSL];
    char escaped_action[MSL];

    /* Escape quotes in action */
    int i, j = 0;
    for (i = 0; action[i] && j < MSL - 2; i++) {
        if (action[i] == '"') {
            escaped_action[j++] = '\\';
        }
        escaped_action[j++] = action[i];
    }
    escaped_action[j] = '\0';

    snprintf(buffer, sizeof(buffer),
             "{\"player\":\"%s\",\"action\":\"%s\",\"importance\":%d,\"timestamp\":%ld}",
             player_name, escaped_action, importance, (long)time(NULL));

    redis_publish("player.actions", buffer);
}

/**
 * Publish system update
 */
void redis_publish_system_update(const char *update_type, const char *details)
{
    char buffer[MSL];
    char escaped_details[MSL];

    /* Escape quotes in details */
    int i, j = 0;
    for (i = 0; details[i] && j < MSL - 2; i++) {
        if (details[i] == '"') {
            escaped_details[j++] = '\\';
        }
        escaped_details[j++] = details[i];
    }
    escaped_details[j] = '\0';

    snprintf(buffer, sizeof(buffer),
             "{\"type\":\"%s\",\"details\":\"%s\",\"timestamp\":%ld}",
             update_type, escaped_details, (long)time(NULL));

    redis_publish("system.updates", buffer);
}

#else /* !HAVE_HIREDIS */

/* Stub implementations when Redis is not available */
void init_redis_bridge(void) {
    /* Redis support disabled at compile time */
}

void shutdown_redis_bridge(void) {
    /* No-op */
}

void redis_publish_world_event(const char *event_type, const char *json_data) {
    /* No-op */
}

void redis_publish_leader_decision(const char *leader_name, const char *decision, const char *reasoning) {
    /* No-op */
}

void redis_publish_ai_decision(const char *leader_name, const char *decision, const char *reasoning) {
    /* No-op */
}

void redis_publish_player_action(const char *player_name, const char *action, int importance) {
    /* No-op */
}

void redis_publish_system_update(const char *update_type, const char *details) {
    /* No-op */
}

#endif /* HAVE_HIREDIS */
