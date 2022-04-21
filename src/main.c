#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "joybus.h"

void joybus_send_bytes(PIO pio, uint sm, uint offset, uint pin, uint8_t *bytes, uint len);
void joybus_send_byte(PIO pio, uint sm, uint8_t byte, bool stop);
uint8_t joybus_receive_bytes(PIO pio, uint sm, uint offset, uint pin, uint8_t *buf, uint len);
uint8_t joybus_receive_byte(PIO pio, uint sm);

pio_sm_config config;

int main(void)
{  
    set_sys_clock_khz(130000, true);

    stdio_init_all();
    // printf("Hello World\n");

    uint joybus_pin = 4;

    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &joybus_program);

    // LED
    bool led = true;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true) {
        // uint8_t bytes[] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0};
        // joybus_send_bytes(pio, sm, offset, joybus_pin, bytes, sizeof(bytes));

        // LED
        gpio_put(PICO_DEFAULT_LED_PIN, led);
        led = !led;

        // printf("Reading...\n");
        uint8_t buf[1];
        // joybus_program_receive_init(pio, sm, offset, pin, &config);
        joybus_receive_bytes(pio, sm, offset, joybus_pin, buf, sizeof(buf));
        printf("Received 0x%02x\n", buf[0]);
        if (buf[0] == 0x00) {
            uint8_t bytes[] = { 0x09, 0x00, 0x03 };
            joybus_send_bytes(pio, sm, offset, joybus_pin, bytes, sizeof(bytes));
        }
        // joybus_receive_bytes(pio, sm, offset, joybus_pin, buf, sizeof(buf));
        joybus_receive_byte(pio, sm);
        // printf("Received 0x%02x\n", buf[0]);
        // sleep_ms(500);
    }
}