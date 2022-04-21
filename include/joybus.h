#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "joybus.pio.h"

void joybus_send_bytes(PIO pio, uint sm, uint offset, uint pin, uint8_t *bytes, uint len);
void joybus_send_byte(PIO pio, uint sm, uint8_t byte, bool stop);
uint8_t joybus_receive_bytes(PIO pio, uint sm, uint offset, uint pin, uint8_t *buf, uint len);
uint8_t joybus_receive_byte(PIO pio, uint sm);

typedef struct {
    PIO pio;
    uint sm;
    uint offset;
    uint pin;
} joybus_port_t;