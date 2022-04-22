#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "joybus.hpp"

void print_bytes(const char *prefix, uint8_t *bytes, uint len);

int main(void)
{  
    set_sys_clock_khz(130000, true);

    stdio_init_all();

    uint joybus_pin = 4;

    joybus_port_t port;
    if (joybus_port_init(&port, joybus_pin, pio0) != 0) {
        while (true) {
            printf("Error: Failed to claim unused state machine!\n");
        }
    }

    // Set up LED
    bool led = true;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true) {
        uint8_t response[3];
        // TODO: Experiment with reading just one byte, checking it against
        // known commands, then reading more if it's a command with more length.
        uint response_len = joybus_receive_bytes(&port, response, sizeof(response), 50);

        if (response_len == 1 && response[0] == 0x00) {
            uint8_t status[] = { 0x09, 0x00, 0x03 };
            joybus_send_bytes(&port, status, sizeof(status));
        } else if (response_len == 1 && response[0] == 0x41) {
            uint8_t origin[] = { 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x1F, 0x1F, 0x00, 0x00 };
            joybus_send_bytes(&port, origin, sizeof(origin));
        } else if (response_len == 3 && response[0] == 0x40 && response[1] <= 0x07) {
            uint8_t inputs_report[] = { 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x1F, 0x1F };
            joybus_send_bytes(&port, inputs_report, sizeof(inputs_report));
            // print_bytes("Received ", response, response_len);
            // Toggle LED when we get a poll command
            gpio_put(PICO_DEFAULT_LED_PIN, led);
            led = !led;
        } else {
            print_bytes("Invalid command: ", response, response_len);
            // If we received an invalid command, wait long enough for command
            // to finish, then reset receiving.
            sleep_us(100);
            joybus_reset_receive(&port);
        }
    }
}

void print_bytes(const char *prefix, uint8_t *bytes, uint len) {
    printf(prefix);
    for (int i = 0; i < len; i++) {
        printf("0x%02x ", bytes[i]);
    }
    printf("\n");
}