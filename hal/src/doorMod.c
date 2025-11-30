#include <stdio.h> // fopen, fprintf, fclose, perror
#include <stdlib.h>  // exit, EXIT_FAILURE, EXIT_SUCCESS
#include <stdbool.h>
#include <time.h>
#include "hal/HC-SR04.h"
#include "hal/GPIO.h"
#include "hal/timing.h"
#include "hal/StepperMotor.h"
#include "hal/led.h"
#include "hal/led_worker.h"
#include "include/doorMod.h"
#include "hal/door_udp_client.h"

// Small helper to map Door_t -> UDP booleans
static void report_door_state_udp(Door_t *door)
{
    // We only send well-defined states; UNKNOWN leaves last value on hub.
    bool d0_open = false;
    bool d0_locked = false;

    switch (door->state) {
        case OPEN:
            d0_open = true;
            d0_locked = false;
            break;
        case LOCKED:
            d0_open = false;
            d0_locked = true;
            break;
        case UNLOCKED:
            d0_open = false;
            d0_locked = false;
            break;
        case UNKNOWN:
        default:
            // Don't send anything; we don't have a clean mapping.
            return;
    }

    // This module only has one physical door, so D1 is unused.
    // D1 is reported as CLOSED & UNLOCKED (placeholder).
    door_udp_update(d0_open, d0_locked, false, false);
}


// Initialize the door system
bool initializeDoorSystem (){
    if (!init_hc_sr04 () || !StepperMotor_Init () ) {
        printf("Failed to initialize door module.\n");
        return false;
    }

    // Initialize LEDs (best-effort)
    if (!LED_init()) {
        fprintf(stderr, "Warning: LED_init failed (continuing)\n");
    }

    return true;
}

long long avgDistanceSample (void){
    long long totalDistance = 0;
    int sample_count = 0;
    long long start_sampling_time = getTimeInMs();
    while ((getTimeInMs() - start_sampling_time) < 100){
        long long distance = get_distance();
        if (distance != -1){
            totalDistance += distance;
            sample_count++;
        }
        sleepForMs(5); // Small delay between samples
    }
    if (sample_count == 0){
        return -1; // Indicate error if no valid samples
    }

    return totalDistance / sample_count;
}


// Lock the door
Door_t lockDoor (Door_t *door){
    long long dist = avgDistanceSample();
    if (StepperMotor_GetPosition() == 180){
        printf("Door is already locked.\n");
    } else {
        if (dist >= 10) {
            printf("Door is open, cannot lock.\n");
            door->state = OPEN;
            LED_enqueue_lock_failure();
        }
        else if (dist == -1) {
            printf("Ultrasonic sensor error, cannot lock door.\n");
            door->state = UNKNOWN;
            LED_enqueue_status_door_error();
        }
        else if (dist < 10) {
            if (StepperMotor_Rotate(180)){
                printf("Door locked successfully.\n");
                // Update door state
                door->state = LOCKED;
                LED_enqueue_lock_success();
            } else {
                printf("Failed to lock the door.\n");
                LED_enqueue_lock_failure();
            }
        } else {
            printf("Failed to lock the door.\n");
            LED_enqueue_lock_failure();
        }
    }
    // Report new state to hub (if UDP initialized)
    report_door_state_udp(door);
    return *door;

}

// Unlock the door
Door_t unlockDoor (Door_t *door){
    if (StepperMotor_GetPosition() == 0){
        printf("Door is already unlocked.\n");
    } else {
        long long distance = get_distance();
        if (distance >= 5) {
            printf("Door is open, cannot unlock.\n");
            door->state = OPEN;
            LED_enqueue_unlock_failure();
        }
        else if (StepperMotor_Rotate(180)) {
            printf("Door unlocked successfully.\n");
            // Update door state
            door->state = UNLOCKED;
            LED_enqueue_unlock_success();
        } else {
            printf("Failed to unlock the door.\n");
            LED_enqueue_unlock_failure();
        }
    }
    // Report new state to hub
    report_door_state_udp(door);
    return *door;

}

// Get the current status of the door
Door_t get_door_status (Door_t *door){
    long long distance = get_distance();
    if (StepperMotor_GetPosition() == 180){
        door->state = LOCKED;
        printf("Door is LOCKED.\n");
    } 
    else if (distance < 10 && distance != -1) {
        door->state = UNLOCKED;
        printf("Door is CLOSED and UNLOCKED.\n");
    }
    else if (distance == -1) {
        printf("Error reading distance from ultrasonic sensor.\n");
        LED_enqueue_status_door_error();
        // Report UNKNOWN (helper will skip sending) and return
        report_door_state_udp(door);
        return *door;
    }
    else if (distance >= 10) {
        door->state = OPEN;
        printf("Door is OPEN.\n");
    } 
    else {        
        door->state = UNKNOWN; // Assuming any other position means closed but not locked
        printf("Door is in an UNKNOWN state.\n");
    }
    
    // Report current state to hub
    report_door_state_udp(door);
    return *door;

}
