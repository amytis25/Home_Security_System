// door_tcp_client.h
// has heartbeat mehcanism as a stream
// has notification mechanism for events
#ifndef _DOOR_TCP_CLIENT_H_
#define _DOOR_TCP_CLIENT_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    DOOR_REPORT_NOTIFICATION = 1,   // send EVENT on any change
    DOOR_REPORT_HEARTBEAT    = 2,   // periodic HEARTBEAT
    DOOR_REPORT_BOTH         = 3    // notification + heartbeat
} DoorReportMode;

// Initialize TCP connection from module -> host.
// module_id: "M1", "M2", "M3", etc. (used in messages)
// heartbeat_period_ms: how often to send HEARTBEAT (if enabled)
bool door_tcp_init(const char *host_ip, uint16_t port,
                   const char *module_id,
                   DoorReportMode mode,
                   int heartbeat_period_ms);

// Call this regularly with current states for both doors.
// booleans: true = OPEN/LOCKED, false = CLOSED/UNLOCKED.
void door_tcp_update(bool d0_open, bool d0_locked,
                     bool d1_open, bool d1_locked);

// Close socket & reset state
void door_tcp_close(void);

#endif