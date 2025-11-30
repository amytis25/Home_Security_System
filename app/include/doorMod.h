#include <stdio.h> // fopen, fprintf, fclose, perror
#include <stdlib.h>  // exit, EXIT_FAILURE, EXIT_SUCCESS
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "hal/HC-SR04.h"
#include "hal/GPIO.h"
#include "hal/timing.h"
#include "hal/StepperMotor.h"

typedef enum {
    LOCKED,
    UNLOCKED,
    OPEN,
    UNKNOWN
} DoorState_t;

typedef struct {
    DoorState_t state;
    int id;
    char door1_name[50];
    // Additional door-related fields can be added here
} Door_t;

// Initialize the door system
bool initializeDoorSystem ();

// Start/stop UDP reporting (notifications + heartbeat)
bool door_reporting_start(const char *hub_ip, uint16_t report_port, uint16_t heartbeat_port,
                          const char *module_id, int heartbeat_ms);
void door_reporting_stop(void);

// Lock the door
Door_t lockDoor (Door_t *door);

/* Public door module API (application level)
 *
 * This header intentionally avoids including HAL-specific headers. The
 * implementation in `app/src/` may include HAL headers privately; the point
 * is to keep this header stable and free of platform-specific dependencies
 * so application consumers and tests don't pull in hardware drivers.
 */
#ifndef APP_DOORMOD_H
#define APP_DOORMOD_H

#include <stdbool.h>
#include <stdint.h>

/* Door state exposed to callers. Keep values small and stable. */
typedef enum {
    LOCKED = 0,
    UNLOCKED = 1,
    OPEN = 2,
    UNKNOWN = 3
} DoorState_t;

/* Minimal door status object. Implementations may have richer internal state
 * but should map to this public shape for consumers (UI, tests, etc.). */
typedef struct {
    DoorState_t state;
    int id;
} Door_t;

/* Initialize hardware and app-level subsystems required by the door module.
 * Returns true on success. The implementation may initialize HAL drivers
 * (HC-SR04, StepperMotor, LEDs) internally; callers shouldn't depend on
 * the specific initialization order. */
bool initializeDoorSystem(void);

/* Start/stop UDP reporting (notifications + heartbeat) at the application
 * level. `report_port` is used for notifications and `heartbeat_port` for
 * heartbeat packets. `heartbeat_ms` is the heartbeat interval in milliseconds.
 */
bool door_reporting_start(const char *hub_ip, uint16_t report_port, uint16_t heartbeat_port,
                          const char *module_id, int heartbeat_ms);
void door_reporting_stop(void);

/* Synchronous door control APIs. These operate on a caller-provided Door_t
 * and return the updated state by value. The implementation must ensure
 * these functions are safe to call from the app thread(s) that use them. */
Door_t lockDoor(Door_t *door);
Door_t unlockDoor(Door_t *door);
Door_t get_door_status(Door_t *door);

#endif /* APP_DOORMOD_H */
