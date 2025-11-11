#include <stdio.h> // fopen, fprintf, fclose, perror
#include <stdlib.h>  // exit, EXIT_FAILURE, EXIT_SUCCESS
#include <stdbool.h>
#include <time.h>
#include "hal/HC-SR04.h"
#include "hal/GPIO.h"
#include "hal/timing.h"
#include "hal/StepperMotor.h"
#include "doorMod.h"


// Initialize the door system
bool initializeDoorSystem (){
    if (!init_hc_sr04 () || !StepperMotor_Init () ) {
            printf("Failed to initialize door module.\n");
            return false;
        }
    return true;
}

// Lock the door
Door_t lockDoor (Door_t *door){
    if (StepperMotor_GetPosition()== 180){
        printf("Door is already locked.\n");
    } else {
        if (get_distance() >= 5) {
            printf("Door is open, cannot lock.\n");
            door->state = OPEN;
        }
        else if (StepperMotor_Rotate(180) && get_distance() < 5) {
            printf("Door locked successfully.\n");
            // Update door state
            door->state = LOCKED;
        } else {
            printf("Failed to lock the door.\n");
        }
    }
    return *door;

}

// Unlock the door
Door_t unlockDoor (Door_t *door){
    if (StepperMotor_GetPosition()== 0){
        printf("Door is already unlocked.\n");
    } else {
        if (get_distance() >= 5) {
            printf("Door is open, cannot unlock.\n");
            door->state = OPEN;
        }
        else if (StepperMotor_Rotate(180)) {
            printf("Door unlocked successfully.\n");
            // Update door state
            door->state = UNLOCKED;
        } else {
            printf("Failed to unlock the door.\n");
        }
    }
    return *door;

}

// Get the current status of the door
Door_t get_door_status (Door_t *door){
    int distance = get_distance();
    if (distance == -1) {
        printf("Error reading distance from ultrasonic sensor.\n");
        return *door;
    }

    if (distance >= 5) {
        door->state = OPEN;
    } else {
        int position = StepperMotor_GetPosition();
        if (position == 0) {
            door->state = UNLOCKED;
        } else if (position == 180) {
            door->state = LOCKED;
        } else {
            door->state = UNKNOWN; // Assuming any other position means closed but not locked
        }
    }

    return *door;

}



 