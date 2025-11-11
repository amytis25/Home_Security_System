#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>

/* Initialize a GPIO line for input or output
 * chip: GPIO chip number (e.g., 2 for /dev/gpiochip2)
 * line: Line number within the chip (e.g., 17 for GPIO17)
 * direction: "in" or "out"
 * Returns: true on success, false on error
 */
bool export_pin(int chip, int line, const char* direction);

/* Set GPIO line direction
 * chip: GPIO chip number
 * line: Line number within the chip
 * direction: "in" or "out"
 * Returns: true on success, false on error
 */
bool set_pin_direction(int chip, int line, const char* direction);

/* Write a value to a GPIO line
 * chip: GPIO chip number
 * line: Line number within the chip
 * value: 0 or 1
 * Returns: true on success, false on error
 */
bool write_pin_value(int chip, int line, int value);

/* Read a value from a GPIO line
 * chip: GPIO chip number
 * line: Line number within the chip
 * Returns: 0 or 1 on success, -1 on error
 */
int read_pin_value(int chip, int line);


#endif // GPIO_H