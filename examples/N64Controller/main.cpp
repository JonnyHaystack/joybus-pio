#include "N64Controller.hpp"
#include "gamecube_definitions.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>
#include <stdio.h>

void print_bytes(const char *prefix, uint8_t *bytes, uint len);

N64Controller *controller;

int main(void) {
    set_sys_clock_khz(130'000, true);

    stdio_init_all();

    uint joybus_pin = 1;

    controller = new N64Controller(joybus_pin, 120, pio0);
    n64_report_t report = default_n64_report;

    // Set up LED
    bool led = true;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true) {
        controller->Poll(&report, 0);

        printf("A: %d\n", report.a);
        printf("B: %d\n", report.b);
        printf("C-Left: %d\n", report.c_left);
        printf("C-Right: %d\n", report.c_right);
        printf("C-Down: %d\n", report.c_down);
        printf("C-Up: %d\n", report.c_up);
        printf("L: %d\n", report.l);
        printf("R: %d\n", report.r);
        printf("Z: %d\n", report.z);
        printf("Start: %d\n", report.start);
        printf("D-Pad Left: %d\n", report.dpad_left);
        printf("D-Pad Right: %d\n", report.dpad_right);
        printf("D-Pad Down: %d\n", report.dpad_down);
        printf("D-Pad Up: %d\n", report.dpad_up);
        printf("Stick X-Axis: %d\n", report.stick_x);
        printf("Stick Y-Axis: %d\n", report.stick_y);

        // Toggle LED
        led = !led;
        gpio_put(PICO_DEFAULT_LED_PIN, led);
    }
}
