#ifndef JOYBUS_H
#define JOYBUS_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "joybus.pio.h"

typedef struct {
    PIO pio;
    uint sm;
    uint offset;
    uint pin;
} joybus_port_t;

int joybus_port_init(joybus_port_t *port, PIO pio);
int joybus_port_init(joybus_port_t *port, PIO pio, uint sm);
int joybus_port_init(joybus_port_t *port, PIO pio, uint sm, uint offset);
uint8_t joybus_send_receive(
    joybus_port_t *port,
    uint8_t *message,
    uint message_len,
    uint8_t *response_buf,
    uint response_len,
    uint read_timeout_us
);
void joybus_send_bytes(joybus_port_t *port, uint8_t *bytes, uint len);
void joybus_send_byte(PIO pio, uint sm, uint8_t byte, bool stop);
uint8_t joybus_receive_bytes(
    PIO pio,
    uint sm,
    uint offset,
    uint8_t *buf,
    uint len,
    uint64_t timeout_us
);
bool joybus_receive_byte(PIO pio, uint sm, uint8_t *byte);

#endif