#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "hal/GPIO.h"
#include "hal/timing.h"


#define STEPS_PER_DEGREE (4096 / 360)
// IN1 GPIO 04, IN2 GPIO 22, IN3 GPIO 5, IN4 GPIO 6
#define IN1_GPIOCHIP 1  /* GPIO chip number for IN1 pin */
#define IN1_GPIO_LINE 27 /* GPIO27 - from gpioinfo: gpiochip1 27 "GPIO27" */
#define IN2_GPIOCHIP 1  /* GPIO chip number for IN2 pin */
#define IN2_GPIO_LINE 22 /* GPIO22 - from gpioinfo: gpiochip1 22 "GPIO22" */
#define IN3_GPIOCHIP 1  /* GPIO chip number for IN3 pin */
#define IN3_GPIO_LINE 23 /* GPIO23 - from gpioinfo: gpiochip1 23 "GPIO23" */
#define IN4_GPIOCHIP 1  /* GPIO chip number for IN4 pin */
#define IN4_GPIO_LINE 24 /* GPIO24 - from gpioinfo: gpiochip1 24 "GPIO24" */

// Function declarations
void StepperMotor_Init(void);
bool StepperMotor_Rotate(int target_degrees);
int StepperMotor_GetPosition(void);
bool StepperMotor_ResetPosition(void);

#endif // STEPPER_MOTOR_H