#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "joybus.pio.h"

uint8_t joybus_read_bytes(PIO pio, uint sm, uint offset, uint pin, uint8_t *buf, uint len);
uint8_t joybus_read_byte(PIO pio, uint sm);
void joybus_write_bytes(PIO pio, uint sm, uint offset, uint pin, uint8_t *bytes, uint len);
void joybus_write_byte(PIO pio, uint sm, uint8_t byte, bool stop);

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
    config = joybus_program_init(pio, sm, offset, joybus_pin);

    // LED
    bool led = true;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true) {
        // uint8_t bytes[] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xf0};
        // joybus_write_bytes(pio, sm, offset, joybus_pin, bytes, sizeof(bytes));

        // LED
        gpio_put(PICO_DEFAULT_LED_PIN, led);
        led = !led;

        // printf("Reading...\n");
        uint8_t buf[1];
        joybus_read_bytes(pio, sm, offset, joybus_pin, buf, sizeof(buf));
        // printf("Received 0x%02x\n", buf[0]);
        if (buf[0] == 0x00) {
            uint8_t bytes[] = { 0x09, 0x00, 0x03 };
            joybus_write_bytes(pio, sm, offset, joybus_pin, bytes, sizeof(bytes));
        }
        // joybus_read_bytes(pio, sm, offset, joybus_pin, buf, sizeof(buf));
        joybus_read_byte(pio, sm);
        // printf("Received 0x%02x\n", buf[0]);
        // sleep_ms(500);
    }
}

void joybus_write_bytes(PIO pio, uint sm, uint offset, uint pin, uint8_t *bytes, uint len) {
    joybus_program_write_init(pio, sm, offset, pin, &config);

    for (int i = 0; i < len; i++) {
        joybus_write_byte(pio, sm, bytes[i], i == len - 1);
    }
}

void joybus_write_byte(PIO pio, uint sm, uint8_t byte, bool stop) {
    uint32_t data_shifted = (byte << 24) | (stop << 23);
    pio_sm_put_blocking(pio, sm, data_shifted);
}

uint8_t joybus_read_bytes(PIO pio, uint sm, uint offset, uint pin, uint8_t *buf, uint len) {
    // Have to do this unconditionally otherwise ISR and RX FIFO might not be clean
    // if (pio_sm_get_pc(pio, sm) != joybus_offset_read) {
    // joybus_program_read_init(pio, sm, offset, pin);
    // }
    joybus_program_read_init(pio, sm, offset, pin, &config);

    int i;
    for (i = 0; i < len; i++) {
        buf[i] = joybus_read_byte(pio, sm);
    }

    return i;
}

uint8_t joybus_read_byte(PIO pio, uint sm) {
    return (uint8_t) pio_sm_get_blocking(pio, sm);
}