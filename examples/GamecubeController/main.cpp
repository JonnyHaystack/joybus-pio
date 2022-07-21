#include "GamecubeController.hpp"
#include "gamecube_definitions.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>
#include <stdio.h>

void print_bytes(const char *prefix, uint8_t *bytes, uint len);

GamecubeController *gcc;

int main(void) {
    set_sys_clock_khz(130'000, true);

    stdio_init_all();

    uint joybus_pin = 1;

    gcc = new GamecubeController(joybus_pin, 120, pio0);
    gc_report_t report = default_gc_report;

    // Set up LED
    bool led = true;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true) {
        gcc->Poll(&report, 0);

        printf("A: %d\n", report.a);
        printf("B: %d\n", report.b);
        printf("X: %d\n", report.x);
        printf("Y: %d\n", report.y);
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
        printf("C-Stick X-Axis: %d\n", report.cstick_x);
        printf("C-Stick Y-Axis: %d\n", report.cstick_y);
        printf("L-Analog: %d\n", report.l_analog);
        printf("R-Analog: %d\n", report.r_analog);

        // Toggle LED
        led = !led;
        gpio_put(PICO_DEFAULT_LED_PIN, led);
    }
}
