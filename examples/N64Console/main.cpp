#include "N64Console.hpp"
#include "n64_definitions.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>

N64Console *console;

int main(void) {
    set_sys_clock_khz(130'000, true);

    uint joybus_pin = 1;

    console = new N64Console(joybus_pin, pio0);
    n64_report_t report = default_n64_report;

    // Set up LED
    bool led = true;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true) {
        console->WaitForPoll();
        console->SendReport(&report);

        // Toggle LED
        led = !led;
        gpio_put(PICO_DEFAULT_LED_PIN, led);
    }
}
