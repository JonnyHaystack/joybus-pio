#include "joybus.h"

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
 * @param read_timeout_ms 
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
    uint read_timeout_ms
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
        read_timeout_ms
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
    uint pin,
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

        buf[bytes_received] = joybus_receive_byte(pio, sm);
    }

    return bytes_received;
}

uint8_t joybus_receive_byte(PIO pio, uint sm) {
    // TODO: Change autopush threshold to 1 and add bit timeout which works
    // the same as the above byte timeout.
    // Make sure byte timeout will still work correctly with new autopush
    // threshold.
    // Should torture test this reading by spamming continuous bits at it from
    // another Pico, and seeing if the FIFO fills up. We want to make sure
    // we're reading it faster than bits are added to it. i.e. we can only spend
    // 130*4 cycles per bit doing stuff outside of pio_sm_get_blocking(),
    // including what we do in joybus_receive_bytes(), because we'll still be
    // accumulating bits here while we do stuff in there.
    return (uint8_t) pio_sm_get_blocking(pio, sm);
}