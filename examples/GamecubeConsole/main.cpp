#include "GamecubeConsole.hpp"
#include "gamecube_definitions.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>

GamecubeConsole *gc;

int main(void) {
    set_sys_clock_khz(130'000, true);

    uint joybus_pin = 1;

    gc = new GamecubeConsole(joybus_pin, pio0);
    gc_report_t report = default_gc_report;

    // Set up LED
    bool led = true;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true) {
        gc->WaitForPoll();
        gc->SendReport(&report);

        // Toggle LED
        led = !led;
        gpio_put(PICO_DEFAULT_LED_PIN, led);
    }
}
