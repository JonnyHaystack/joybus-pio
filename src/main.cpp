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
        // Toggle LED
        gpio_put(PICO_DEFAULT_LED_PIN, led);
        led = !led;

        uint8_t response1_buf[8];
        uint8_t response2_buf[8];
        uint8_t response3_buf[8];
        uint response1_len = joybus_send_receive(&port, nullptr, 0, response1_buf, 1, 50);
        if (response1_buf[0] == 0x00) {
            uint8_t status[] = { 0x09, 0x00, 0x03 };
            uint response2_len = joybus_send_receive(&port, status, sizeof(status), response2_buf, 1, 50);
            if (response2_buf[0] == 0x41) {
                uint8_t origin[] = { 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x1F, 0x1F, 0x00, 0x00 };
                uint response3_len = joybus_send_receive(&port, origin, sizeof(origin), response3_buf, 1, 50);
                print_bytes("Received ", response1_buf, response1_len);
                print_bytes("Sent ", status, sizeof(status));
                print_bytes("Received ", response2_buf, response2_len);
                print_bytes("Sent ", origin, sizeof(origin));
                print_bytes("Received ", response3_buf, response3_len);
                printf("\n");
            }
        }
        sleep_ms(100);
    }
}

void print_bytes(const char *prefix, uint8_t *bytes, uint len) {
    printf(prefix);
    for (int i = 0; i < len; i++) {
        printf("0x%02x ", bytes[i]);
    }
    printf("\n");
}