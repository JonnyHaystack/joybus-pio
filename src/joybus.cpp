#include "joybus.hpp"

int joybus_port_init(joybus_port_t *port, PIO pio) {
    int sm = pio_claim_unused_sm(pio, false);
    if (sm < 0) {
        return 1;
    }

    return joybus_port_init(port, pio, sm);
}

int joybus_port_init(joybus_port_t *port, PIO pio, uint sm) {
    return joybus_port_init(port, pio, sm, pio_add_program(pio, &joybus_program));
}

int joybus_port_init(joybus_port_t *port, PIO pio, uint sm, uint offset) {
    port->pio = pio;
    port->sm = sm;
    port->offset = offset;

    return 0;
}

/**
 * @brief Send and receive a certain number of bytes 
 * 
 * @param pio 
 * @param sm 
 * @param offset 
 * @param pin 
 * @param message 
 * @param message_len 
 * @param response_buf 
 * @param response_len 
 * @param read_timeout_us 
 * @return uint8_t 
 */
uint8_t joybus_send_receive(
    PIO pio,
    uint sm,
    uint offset,
    uint pin,
    uint8_t *message,
    uint message_len,
    uint8_t *response_buf,
    uint response_len,
    uint read_timeout_us
) {
    // If the message has length zero, we send nothing and manually init
    // the state machine for receiving.
    if (message_len > 0) {
        joybus_send_bytes(pio, sm, offset, pin, message, message_len);
    } else {
        joybus_program_receive_init(pio, sm, offset, pin);
    }

    return joybus_receive_bytes(
        pio,
        sm,
        offset,
        pin,
        response_buf,
        response_len,
        read_timeout_us
    );
}

void joybus_send_bytes(
    PIO pio,
    uint sm,
    uint offset,
    uint pin,
    uint8_t *bytes,
    uint len
) {
    joybus_program_send_init(pio, sm, offset, pin);

    for (int i = 0; i < len; i++) {
        joybus_send_byte(pio, sm, bytes[i], i == len - 1);
    }
}

void joybus_send_byte(PIO pio, uint sm, uint8_t byte, bool stop) {
    uint32_t data_shifted = (byte << 24) | (stop << 23);
    pio_sm_put_blocking(pio, sm, data_shifted);
}

uint8_t joybus_receive_bytes(
    PIO pio,
    uint sm,
    uint offset,
    uint8_t *buf,
    uint len,
    uint64_t timeout_us
) {
    uint8_t bytes_received;

    for (bytes_received = 0; bytes_received < len; bytes_received++) {
        /* Read timeout in case we don't receive as many bytes as we expected
         * for some reason.
         * This timeout is only applied after we receive the first byte, because
         * we don't know how long we'll have to wait for the first byte but we
         * know how long we should have to wait between bytes in one message. */
        absolute_time_t timeout_timestamp = make_timeout_time_us(timeout_us);
        while (bytes_received > 0 && pio_sm_is_rx_fifo_empty(pio, sm)) {
            if (time_reached(timeout_timestamp)) {
                return bytes_received;
            }
        }

        if (!joybus_receive_byte(pio, sm, &buf[bytes_received])) {
            return bytes_received;
        }
    }

    return bytes_received;
}

bool joybus_receive_byte(PIO pio, uint sm, uint8_t *byte) {
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
        absolute_time_t timeout_timestamp = make_timeout_time_us(10);
        while (bits_received > 0 && pio_sm_is_rx_fifo_empty(pio, sm)) {
            if (time_reached(timeout_timestamp)) {
                return false;
            }
        }
        printf("RX FIFO level: %d\n", pio_sm_get_rx_fifo_level(pio, sm));

        // TODO: Technically masking on the LSB shouldn't be necessary because
        // the autopush threshold is set to 1, so we're only expecting the LSB
        // to be set in each byte that is pushed anyway.
        bool received_bit = pio_sm_get_blocking(pio, sm) & 0x01;

        received_byte |= received_bit << bits_received;
    }

    *byte = received_byte;

    return true;
}