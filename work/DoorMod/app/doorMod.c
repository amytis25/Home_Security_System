#include <stdio.h> // fopen, fprintf, fclose, perror
#include <stdlib.h>  // exit, EXIT_FAILURE, EXIT_SUCCESS
#include <stdbool.h>
#include <time.h>
#include "hal/HC-SR04.h"
#include "hal/GPIO.h"
#include "hal/timing.h"


int main() {
    printf("DoorMod application started.\n");
    // Application logic goes here
    if (init_hc_sr04()){
        printf("HC-SR04 initialized successfully.\n");
    } else {
        printf("Failed to initialize HC-SR04.\n");
        return EXIT_FAILURE;
    }
    while(1){
        long long distance = get_distance();
        if (distance != -1) {
            printf("Distance measured: %lld cm\n", distance);
        } else {
            printf("Failed to measure distance.\n");
        }
        sleepForMs(1000); // Wait for 1 second before next measurement
    }
  

    return 0;
}