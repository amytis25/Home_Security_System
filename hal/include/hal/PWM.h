// PWM.h
// ENSC 351 Fall 2025
// PWM control interface for GPIO15

#ifndef PWM_H
#define PWM_H
#include <stdbool.h>

// PWM files 
#define PWM_DUTY_CYCLE_FILE "/dev/hat/pwm/GPIO15/duty_cycle"
#define PWM_PERIOD_FILE "/dev/hat/pwm/GPIO15/period"
#define PWM_ENABLE_FILE "/dev/hat/pwm/GPIO15/enable"

// PWM helper Functions
bool PWM_export(void); // Initialization - exports PWM pin on BY-AI
// Set PWM as requested
bool PWM_setDutyCycle(int dutyCycle);
bool PWM_setPeriod(int period);
bool PWM_setFrequency(int Hz, int dutyCyclePercent);
// Enable/disable PWM
bool PWM_enable(void);
bool PWM_disable(void);

#endif