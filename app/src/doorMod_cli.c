/*
 * doorMod_cli.c
 * Small CLI wrapper to run door module logic as a standalone executable.
 * Usage: ./doorMod_cli [MODULE_ID]
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include "doorMod.h"
#include "hal/door_udp.h"
#include "hal/led.h"
#include "hal/led_worker.h"

#define DEBUG

int main(int argc, char *argv[])
{
    const char *module_id = (argc > 1) ? argv[1] : "D1";
    const char *hub_ip    = "192.168.8.108"; // fixed hub IP
    bool reporting_running = false;

    if (!initializeDoorSystem()) {
        fprintf(stderr, "Failed to initialize door system\n");
        return EXIT_FAILURE;
    }

    // Try to start reporting (notifications + separate heartbeat port)
    if (!door_reporting_start(hub_ip, 12345, 12346, module_id, 1000)) {
        fprintf(stderr, "WARNING: reporting init failed; continuing without UDP reporting\n");
        // Indicate network error via LED
        LED_status_network_error();
    } else {
        reporting_running = true;
    }

    Door_t door = { .state = UNKNOWN };

    printf("doorMod CLI started. Commands: l(lock), u(unlock), s(status), q(quit)\n");
    fflush(stdout);
    char line[128];

    while (1) {
        printf("door> ");
        if (!fgets(line, sizeof(line), stdin)) break;

        /* strip trailing newline/carriage return */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
            len--;
        }

        /* skip leading whitespace */
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;

        /* first-char shortcuts (case-insensitive) */
        char c = (char)tolower((unsigned char)p[0]);
        if (c == 'q') break;
        if (c == 'l') {
        
        #ifdef DEBUG        
        printf("DEBUG: calling lockDoor()\n"); 
        fflush(stdout);
        #endif
        door = lockDoor(&door); 
        #ifdef DEBUG
         printf("DEBUG: returned from lockDoor(), state=%d\n", door.state);
         fflush(stdout);
        #endif
        continue; }

        if (c == 'u') { door = unlockDoor(&door); continue; }
        if (c == 's') { door = get_door_status(&door); continue; }

        /* full-word commands (case-insensitive) */
        if (strncasecmp(p, "lock", 4) == 0) { door = lockDoor(&door); continue; }
        if (strncasecmp(p, "unlock", 6) == 0) { door = unlockDoor(&door); continue; }
        if (strncasecmp(p, "status", 6) == 0) { door = get_door_status(&door); continue; }

        printf("Unknown command\n");
    }

    if (reporting_running) door_reporting_stop();

    printf("Exiting doorMod CLI\n");
    return 0;
}
