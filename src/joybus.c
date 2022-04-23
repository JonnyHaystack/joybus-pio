#include "joybus.h"

#include "joybus.pio.h"

int joybus_port_init(joybus_port_t *port, uint pin, PIO pio, int sm, int offset) {
    if (sm < 0) {
        sm = pio_claim_unused_sm(pio, false);
        if (sm < 0) {
            return 1;
        }
    }

    if (offset < 0) {
        offset = pio_add_program(pio, &joybus_program);
    } else {
        pio_add_program_at_offset(pio, &joybus_program, offset);
    }

    port->pin = pin;
    port->pio = pio;
    port->sm = sm;
    port->offset = offset;

    joybus_program_receive_init(pio, sm, offset, pin);

    return 0;
}

void joybus_port_terminate(joybus_port_t *port) {
    pio_sm_set_enabled(port->pio, port->sm, false);
    pio_sm_unclaim(port->pio, port->sm);
}

void joybus_reset_receive(joybus_port_t *port) {
    joybus_program_receive_init(port->pio, port->sm, port->offset, port->pin);
}

uint joybus_send_receive(
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
        joybus_reset_receive(port);
    }

    return joybus_receive_bytes(port, response_buf, response_len, read_timeout_us);
}

void joybus_send_bytes(joybus_port_t *port, uint8_t *bytes, uint len) {
    joybus_program_send_init(port->pio, port->sm, port->offset, port->pin);

    for (int i = 0; i < len; i++) {
        joybus_send_byte(port, bytes[i], i == len - 1);
    }
}

void joybus_send_byte(joybus_port_t *port, uint8_t byte, bool stop) {
    uint32_t data_shifted = (byte << 24) | (stop << 23);
    pio_sm_put_blocking(port->pio, port->sm, data_shifted);
}

uint joybus_receive_bytes(joybus_port_t *port, uint8_t *buf, uint len, uint64_t timeout_us) {
    uint8_t bytes_received;

    for (bytes_received = 0; bytes_received < len; bytes_received++) {
        /* Read timeout in case we don't receive as many bytes as we expected
         * for some reason.
         * This timeout is only applied after we receive the first byte, because
         * we don't know how long we'll have to wait for the first byte but we
         * know how long we should have to wait between bytes in one message. */
        if (bytes_received > 0) {
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

uint8_t joybus_receive_byte(joybus_port_t *port) {
    return pio_sm_get_blocking(port->pio, port->sm);
}

bool joybus_receive_byte_timeout(joybus_port_t *port, uint8_t *byte, uint64_t timeout_us) {
    // TODO: Change autopush threshold to 1 and add bit timeout which works
    // the same as the above byte timeout.
    // Make sure byte timeout will still work correctly with new autopush
    // threshold. I think it should.
    // Should torture test this reading by spamming continuous bits at it from
    // another Pico, and seeing if the FIFO fills up. We want to make sure
    // we're reading it faster than bits are added to it. i.e. we can only spend
    // 130*4 cycles per bit doing stuff outside of pio_sm_get_blocking(),
    // including what we do in joybus_receive_bytes(), because we'll still be
    // accumulating bits here while we do stuff in there.
    uint8_t received_byte = 0;

    for (int bits_received = 0; bits_received < 8; bits_received++) {
        if (bits_received > 0) {
            absolute_time_t timeout_timestamp = make_timeout_time_us(timeout_us);
            while (pio_sm_is_rx_fifo_empty(port->pio, port->sm)) {
                if (time_reached(timeout_timestamp)) {
                    return false;
                }
            }
        }
        // printf(
        //     "RX FIFO level: %d\n",
        //     pio_sm_get_rx_fifo_level(port->pio, port->sm)
        // );

        bool received_bit = pio_sm_get_blocking(port->pio, port->sm) & 0x01;

        received_byte |= received_bit << bits_received;
    }

    *byte = received_byte;

    return true;
}
