// hal/src/button.c
// Minimal GPIO button via /dev/gpiochip2 (v1 API). No sysfs.

#include "hal/button.h"
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <stdio.h>

#define GPIOCHIP_PATH "/dev/gpiochip2"
#define BUTTON_LINE   8     // from `gpioinfo`: gpiochip2 8 "GPIO17"
#define ACTIVE_LOW    1     // pressed pulls line LOW (typical with pull-up)

static int line_fd = -1;

bool Button_init(void) {
    int chip_fd = open(GPIOCHIP_PATH, O_RDONLY | O_CLOEXEC);
    if (chip_fd < 0) { perror("open gpiochip"); return false; }

    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = BUTTON_LINE;
    req.lines = 1;
    req.flags = GPIOHANDLE_REQUEST_INPUT;
#ifdef GPIOHANDLE_REQUEST_BIAS_PULL_UP
    req.flags |= GPIOHANDLE_REQUEST_BIAS_PULL_UP; // ok if unsupported; kernel ignores
#endif

    if (ioctl(chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        perror("GPIO_GET_LINEHANDLE_IOCTL");
        close(chip_fd);
        return false;
    }
    close(chip_fd);

    line_fd = req.fd; // handle used for reads
    return (line_fd >= 0);
}

bool Button_read(bool *pressed) {
    if (line_fd < 0 || !pressed) return false;

    struct gpiohandle_data data;
    memset(&data, 0, sizeof(data));

    if (ioctl(line_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) {
        perror("GPIOHANDLE_GET_LINE_VALUES_IOCTL");
        return false;
    }

#if ACTIVE_LOW
    *pressed = (data.values[0] == 0);  // pressed = logic low
#else
    *pressed = (data.values[0] == 1);  // pressed = logic high
#endif
    return true;
}

void Button_close(void) {
    if (line_fd >= 0) { close(line_fd); line_fd = -1; }
}