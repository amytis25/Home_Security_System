#include <stdio.h> // fopen, fprintf, fclose, perror
#include <stdlib.h>  // exit, EXIT_FAILURE, EXIT_SUCCESS
#include <stdbool.h>
#include <time.h>
#include "hal/timing.h"
#include "hal/PWM.h"
#include <unistd.h>

//#define DEBUG 

#define NANOSECONDS_IN_SECOND 1000000000

// Helper function to write to a file
bool PWM_export(void){
    // If the PWM sysfs already exists, consider it exported.
    if (access(PWM_ENABLE_FILE, F_OK) == 0) return true;

    // Try to export the PWM using helper tool. Do not call `sudo` here;
    // the caller should run the program with appropriate privileges.
    int rc = system("beagle-pwm-export --pin GPIO15");
    if (rc != 0) {
        fprintf(stderr, "PWM_export: beagle-pwm-export failed (rc=%d)\n", rc);
        return false;
    }

    // Wait briefly for sysfs entries to appear
    for (int i = 0; i < 20; ++i) {
        if (access(PWM_ENABLE_FILE, F_OK) == 0) return true;
        usleep(100000); // 100 ms
    }
    fprintf(stderr, "PWM_export: timeout waiting for %s\n", PWM_ENABLE_FILE);
    return false;
}
static bool writeToFile(const char* filename, const char* value) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file '%s': ", filename);
        perror(NULL);
        return false;
    }
    
    if (fprintf(file, "%s", value) < 0) {
        fprintf(stderr, "Error writing to file '%s': ", filename);
        perror(NULL);
        fclose(file);
        return false;
    }
    
    fclose(file);
    return true;
}

// PWM helper Functions
bool PWM_setDutyCycle(int dutyCycle){
    #ifdef DEBUG
    printf("Setting duty cycle to %d\n", dutyCycle);
    #endif
    //writeToFile(PWM_DUTY_CYCLE_FILE, "0"); // --- IGNORE ---
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "%d", dutyCycle);
    if (n < 0) return false;
    return writeToFile(PWM_DUTY_CYCLE_FILE, buf);
}

bool PWM_setPeriod(int period){
    #ifdef DEBUG
    printf("Setting period to %d\n", period);
    #endif
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "%d", period);
    if (n < 0) return false;
    return writeToFile(PWM_PERIOD_FILE, buf);
}


bool PWM_setFrequency(int Hz, int dutyCyclePercent)
{
    if (dutyCyclePercent < 0 || dutyCyclePercent > 100) {
        fprintf(stderr, "PWM_setFrequency: invalid duty cycle percentage\n");
        return false;
    }

    // Assignment spec: support 0 - 500 Hz
    if (Hz < 0)   Hz = 0;
    if (Hz > 500) Hz = 500;

    if (Hz == 0) {
        // 0 Hz -> LED off / stop PWM
        return PWM_disable();
    }

    // Use 64-bit to avoid any overflow/rounding surprises
    const unsigned long long NSEC = 1000000000ULL;
    unsigned long long period_ull = NSEC / (unsigned long long)Hz;
    unsigned long long duty_ull   = (period_ull * (unsigned long long)dutyCyclePercent) / 100ULL;

    // SAFEST ORDER:
    // 1) duty=0 -> disable (so period can shrink safely)
    // 2) period=new
    // 3) duty=desired
    // 4) enable
    
    // disable
    if (!PWM_disable()) return false;

    // cast to int because helpers take int
    if (!PWM_setPeriod((int)period_ull)) return false;
    if (!PWM_setDutyCycle((int)duty_ull)) return false;

    // now enable
    PWM_enable();
    return true;
}
bool PWM_enable(){
    #ifdef DEBUG
    printf("Enabling PWM\n");
    #endif
    return writeToFile(PWM_ENABLE_FILE, "1");
}

bool PWM_disable(){
    #ifdef DEBUG
    printf("Disabling PWM\n");
    #endif
    return writeToFile(PWM_ENABLE_FILE, "0");
}