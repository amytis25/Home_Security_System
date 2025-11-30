// hub_webhook.h
// Thin asynchronous wrapper around hal DiscordAlert functions.

#ifndef HUB_WEBHOOK_H
#define HUB_WEBHOOK_H

#include <stdbool.h>

// Initialize webhook worker. Pass NULL to skip initialization.
// Returns true on success.
bool hub_webhook_init(const char *webhook_url);

// Shutdown worker and cleanup resources.
void hub_webhook_shutdown(void);

// Enqueue a message to be sent to the webhook (non-blocking).
// The message will be copied; caller may free the buffer after return.
void hub_webhook_send(const char *msg);

#endif // HUB_WEBHOOK_H
