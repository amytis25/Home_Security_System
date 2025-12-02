// PWM.h
// ENSC 351 Fall 2025
// PWM control interface 

#ifndef PWM_H
#define PWM_H
#include <stdbool.h>

// PWM files 
typedef struct {
    const char *duty_cycle;
    const char *period;
    const char *enable;
} LEDt;

/* LEDt instances are defined in a single C file to avoid multiple-definition
   linker errors. Declare them here as `extern` so users of the header can
   reference the same objects. */
extern const LEDt RED_LED;
extern const LEDt GREEN_LED;

#define PWM_DUTY_CYCLE_FILE "/dev/hat/pwm/GPIO15/duty_cycle"
#define PWM_PERIOD_FILE "/dev/hat/pwm/GPIO15/period"
#define PWM_ENABLE_FILE "/dev/hat/pwm/GPIO15/enable"

// PWM helper Functions
bool PWM_export(void); // Initialization - exports PWM pin on BY-AI
// Set PWM as requested
bool PWM_setDutyCycle(LEDt led, int dutyCycle);
bool PWM_setPeriod(LEDt led, int period);
bool PWM_setFrequency(LEDt led, int Hz, int dutyCyclePercent);
// Enable/disable PWM
bool PWM_enable(LEDt led);
bool PWM_disable(LEDt led);

#endif