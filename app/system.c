#include <stdio.h> // fopen, fprintf, fclose, perror
#include <stdlib.h>  // exit, EXIT_FAILURE, EXIT_SUCCESS
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "doorMod.h"

typedef struct {
    Door_t* door[4];
} door_system_t;

int main(){
    if (!initializeDoorSystem ()){
        printf("System initialization failed. Exiting.\n");
        return EXIT_FAILURE;
    } else {
        printf("System initialized successfully.\n");
    }
    
    int control_number;
    bool running = true;
    Door_t* door1;
    while (running) {
        printf("Enter a number to do a task: \n1: Lock Door\n2: Unlock Door\n3: Get Door Status\n");
        scanf("%d", &control_number);
        if (control_number < 1 || control_number > 3) {
            printf("Invalid control number.\n");
            continue;
        } else if (control_number == 1){
            door1 = lockDoor(&door1);
            running = false;
        } else if (control_number == 2){
            door1 = unlockDoor(&door1);
            running = false;
        } else if (control_number == 3){
            door1 = get_door_status(&door1);
            printf("State: %d\n", door1->state);
        }
    }



    return 0;
}