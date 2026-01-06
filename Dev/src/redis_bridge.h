/**
 * Redis Bridge - Publish events from MUD to Redis
 *
 * This module handles all communication with Redis for event publishing,
 * allowing the MUD engine to broadcast events to the web dashboard in real-time.
 */

#ifndef REDIS_BRIDGE_H
#define REDIS_BRIDGE_H

#include <hiredis/hiredis.h>

/* Redis connection state */
extern redisContext *redis_ctx;

/* Initialize Redis connection */
void init_redis_bridge(void);

/* Shutdown Redis connection */
void close_redis_bridge(void);

/* Publish event to Redis channel */
void redis_publish(const char *channel, const char *message);

/* Helper functions to publish specific event types */
void redis_publish_world_event(const char *event_type, const char *json_data);
void redis_publish_ai_decision(const char *leader_name, const char *decision, const char *reasoning);
void redis_publish_economy_update(const char *area_name, const char *resource, int change);
void redis_publish_player_action(const char *player_name, const char *action, int importance);
void redis_publish_system_update(const char *update_type, const char *details);

/* Check if Redis is connected */
bool is_redis_connected(void);

#endif /* REDIS_BRIDGE_H */
