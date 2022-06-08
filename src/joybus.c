#include "joybus.h"

#include "joybus.pio.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>

uint joybus_port_init(joybus_port_t *port, uint pin, PIO pio, int sm, int offset) {
    if (sm < 0) {
        sm = pio_claim_unused_sm(pio, true);
    } else {
        pio_sm_claim(pio, sm);
    }

    if (offset < 0) {
        offset = pio_add_program(pio, &joybus_program);
    }

    port->pin = pin;
    port->pio = pio;
    port->sm = sm;
    port->offset = offset;
    port->config = joybus_program_get_config(pio, sm, offset, pin);

    joybus_port_reset(port);

    return offset;
}

void joybus_port_terminate(joybus_port_t *port) {
    pio_sm_set_enabled(port->pio, port->sm, false);
    pio_sm_unclaim(port->pio, port->sm);
    pio_remove_program(port->pio, &joybus_program, port->offset);
}

void joybus_port_reset(joybus_port_t *port) {
    joybus_program_receive_init(port->pio, port->sm, port->offset, port->pin, &port->config);
}

uint __no_inline_not_in_flash_func(joybus_send_receive)(
    joybus_port_t *port,
    uint8_t *message,
    uint message_len,
    uint8_t *response_buf,
    uint response_len,
    uint read_timeout_us
) {
    // If the message has length zero, we send nothing and manually init
    // the state machine for receiving.
    if (message_len > 0) {
        joybus_send_bytes(port, message, message_len);
    } else {
        joybus_port_reset(port);
    }

    return joybus_receive_bytes(port, response_buf, response_len, read_timeout_us, false);
}

void __no_inline_not_in_flash_func(joybus_send_bytes)(
    joybus_port_t *port,
    uint8_t *bytes,
    uint len
) {
    // Wait for line to be high before sending anything.
    while (!gpio_get(port->pin)) {
        tight_loop_contents();
    }

    joybus_program_send_init(port->pio, port->sm, port->offset, port->pin, &port->config);

    for (int i = 0; i < len; i++) {
        joybus_send_byte(port, bytes[i], i == len - 1);
    }
}

void __no_inline_not_in_flash_func(joybus_send_byte)(joybus_port_t *port, uint8_t byte, bool stop) {
    uint32_t data_shifted = (byte << 24) | (stop << 23);
    pio_sm_put_blocking(port->pio, port->sm, data_shifted);
}

uint __no_inline_not_in_flash_func(joybus_receive_bytes)(
    joybus_port_t *port,
    uint8_t *buf,
    uint len,
    uint64_t timeout_us,
    bool first_byte_can_timeout
) {
    uint8_t bytes_received;

    for (bytes_received = 0; bytes_received < len; bytes_received++) {
        /* Read timeout in case we don't receive as many bytes as we expected for some reason.
         * Usually this timeout is only applied after we receive the first byte, because we don't
         * know how long we'll have to wait for the first byte, but we know how long we should
         * have to wait between bytes in one message. However, sometimes we might want to read one
         * byte of a command, do some processing, and then read another byte. In that case, we
         * would have to also apply the timeout to the first byte of the second read, because it
         * isn't actually the first byte of the command. */
        if (bytes_received > 0 || first_byte_can_timeout) {
            absolute_time_t timeout_timestamp = make_timeout_time_us(timeout_us);
            while (pio_sm_is_rx_fifo_empty(port->pio, port->sm)) {
                if (time_reached(timeout_timestamp)) {
                    return bytes_received;
                }
            }
        }

        buf[bytes_received] = joybus_receive_byte(port);
    }

    return bytes_received;
}

uint8_t __no_inline_not_in_flash_func(joybus_receive_byte)(joybus_port_t *port) {
    return pio_sm_get_blocking(port->pio, port->sm);
}
