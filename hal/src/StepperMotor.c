#include "hal/StepperMotor.h"

// #define DEBUG_STEP_MOTOR

// Global position tracking (in degrees)
static int current_position = 0;

// Function to set a single step pattern
static bool set_motor_pins(const int* pattern) {
    if (!write_pin_value(IN1_GPIOCHIP, IN1_GPIO_LINE, pattern[0])) return false;
    if (!write_pin_value(IN2_GPIOCHIP, IN2_GPIO_LINE, pattern[1])) return false;
    if (!write_pin_value(IN3_GPIOCHIP, IN3_GPIO_LINE, pattern[2])) return false;
    if (!write_pin_value(IN4_GPIOCHIP, IN4_GPIO_LINE, pattern[3])) return false;
    return true;
}

bool StepperMotor_Init(void) {
    printf("Initializing Stepper Motor...\n");
    
    // Initialize all pins as outputs
    if (!export_pin(IN1_GPIOCHIP, IN1_GPIO_LINE, "out") ||
        !export_pin(IN2_GPIOCHIP, IN2_GPIO_LINE, "out") ||
        !export_pin(IN3_GPIOCHIP, IN3_GPIO_LINE, "out") ||
        !export_pin(IN4_GPIOCHIP, IN4_GPIO_LINE, "out")) {
        printf("Failed to initialize motor pins\n");
        return false;
    }

    // Set initial position to 0
    current_position = 0;
    
    // Set all pins low initially
    if (!set_motor_pins(zero_pattern)) {
        printf("Failed to set initial pin states\n");
        return false;
    }
    
    printf("Stepper Motor initialized successfully\n");
    return true;
}

bool StepperMotor_Rotate(int target_degrees) {
    if (target_degrees < 0 || target_degrees > 360) {
        printf("Invalid target degrees: %d\n", target_degrees);
        return false;
    }

    // Calculate steps needed
    int total_steps = target_degrees * STEPS_PER_DEGREE;
    int current_step = 0;
    int sequence_index = 0;
    
    printf("Rotating motor to %d degrees (%d steps)...\n", 
           target_degrees, total_steps);
    
    // Perform rotation
    while (current_step < total_steps) {
        // Set the current step pattern
        if (!set_motor_pins(halfstep_sequence[sequence_index])) {
            printf("Failed to set motor pins at step %d\n", current_step);
            return false;
        }

        // Move to next sequence pattern
        sequence_index = (sequence_index + 1) % 8;
        current_step++;

        // Update position tracking (normalize to 0-360)
        current_position = (current_position + (360 / (STEPS_PER_DEGREE * 8))) % 360;

        // Small delay between steps (adjust for speed)
        sleepForMs(2);
    }

    // Hold final position
    printf("Rotation complete. Current position: %d degrees\n", 
           current_position);
    return true;
}

int StepperMotor_GetPosition(void) {
    return current_position;
}

bool StepperMotor_ResetPosition(void) {
    // Reset position to 0 degrees
    if (!set_motor_pins(zero_pattern)) {
        return false;
    }
    current_position = 0;
    return true;
}