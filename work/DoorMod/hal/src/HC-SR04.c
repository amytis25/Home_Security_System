/* HC-SR04 driver implementation using project's GPIO helpers and timing utilities */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "hal/HC-SR04.h"
#include "hal/GPIO.h"
#include "hal/timing.h"

bool init_hc_sr04(){
    /* Initialize trigger pin as output */
    if(!export_pin(TRIG_GPIOCHIP, TRIG_GPIO_LINE, "out")) {
        perror("Failed to export trigger pin");
        return false;
    }
    
    /* Initialize echo pin as input */
    if(!export_pin(ECHO_GPIOCHIP, ECHO_GPIO_LINE, "in")) {
        perror("Failed to export echo pin");
        return false;
    }

    /* Ensure trigger starts low */
    if(!write_pin_value(TRIG_GPIOCHIP, TRIG_GPIO_LINE, 0)) {
        perror("Failed to set initial trigger state");
        return false;
    }
    return true;
}


long long get_distance(){
    /* Trigger pulse sequence */
    if(!write_pin_value(TRIG_GPIOCHIP, TRIG_GPIO_LINE, 0)) {
        perror("Failed to set trigger low");
        return -1;
    }
    sleepForMs(1); /* At least 2us, we use 1ms since that's our timing resolution */
    
    if(!write_pin_value(TRIG_GPIOCHIP, TRIG_GPIO_LINE, 1)) {
        perror("Failed to set trigger high");
        return -1;
    }
    sleepForMs(1); /* At least 10us, we use 1ms */
    
    if(!write_pin_value(TRIG_GPIOCHIP, TRIG_GPIO_LINE, 0)) {
        perror("Failed to set trigger low");
        return -1;
    }

    const long long timeout_ms = 60; /* 60 ms timeout */
    long long wait_start = getTimeInMs();

    /* Wait for rising edge */
    while(true) {
        int v = read_pin_value(ECHO_GPIOCHIP, ECHO_GPIO_LINE);
        if(v < 0) {
            perror("Failed to read echo pin value");
            return -1;
        }
        if(v == 1) {
            break;
        }
        if((getTimeInMs() - wait_start) > timeout_ms) {
            perror("Timeout waiting for echo pin to go high");
            return -1;
        }
        sleepForMs(1); /* Poll every 1ms */
    }

    long long t_start = getTimeInMs();

    /* Wait for falling edge */
    while(true) {
        int v = read_pin_value(ECHO_GPIOCHIP, ECHO_GPIO_LINE);
        if(v < 0) {
            perror("Failed to read echo pin value");
            return -1;
        }
        if(v == 0) {
            break;
        }
        if((getTimeInMs() - t_start) > timeout_ms) {
            perror("Timeout waiting for echo pin to go low");
            return -1;
        }
        sleepForMs(1); /* Poll every 1ms */
    }

    long long t_end = getTimeInMs();
    long long pulse_ms = t_end - t_start;
    
    /* Distance calculation:
     * Sound speed = 343.2 m/s
     * Distance = (pulse_time * sound_speed) / 2
     * For centimeters: multiply by 100
     * pulse_ms is in milliseconds, so multiply by 1000 for microseconds
     * Final formula: distance_cm = pulse_us * (343.2 * 100 / 2 / 1000000)
     * Simplified: distance_cm = pulse_us * 0.01716
     */
    return (long long)(pulse_ms * 1000LL * 0.01716);
}