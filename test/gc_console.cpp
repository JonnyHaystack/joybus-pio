#include "GamecubeConsole.hpp"
#include "gamecube_definitions.h"

#include "hardware/pio.h"
#include "pico/stdlib.h"

#include <stdio.h>

void print_bytes(const char *prefix, uint8_t *bytes, uint len);

GamecubeConsole *gc;

int main(void) {
    set_sys_clock_khz(130000, true);

    stdio_init_all();

    uint joybus_pin = 1;

    gc = new GamecubeConsole(joybus_pin, pio0);
    gc_report_t gc_report = default_gc_report;

    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);
    gpio_put(2, 0);

    // Set up LED
    bool led = true;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true) {
        gc->WaitForPoll();
        gc->SendReport(&gc_report);

        // Toggle LED
        led = !led;
        gpio_put(PICO_DEFAULT_LED_PIN, led);
    }
}

void print_bytes(const char *prefix, uint8_t *bytes, uint len) {
    printf("%s", prefix);
    for (int i = 0; i < len; i++) {
        printf("0x%02x ", bytes[i]);
    }
    printf("\n");
}
