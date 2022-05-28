#ifndef _JOYBUS_JOYBUS_H
#define _JOYBUS_JOYBUS_H

#include <hardware/pio.h>
#include <pico/stdlib.h>

/**
 * @brief A structure representing a Joybus instance on a given GPIO pin
 */
typedef struct {
    uint pin;
    PIO pio;
    uint sm;
    uint offset;
    pio_sm_config config;
} joybus_port_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the joybus PIO program and populate necessary information in a struct
 *
 * @param port Pointer to the port structure to initialize
 * @param pin The pin to use for the joybus instance
 * @param pio The PIO instance; either pio0 or pio1
 * @param sm The state machine to run the joybus instance on. Pass in -1 to automatically claim
 * unused.
 * @param offset The instruction memory offset at which to load the program. Pass in -1 to allocate
 * automatically.
 *
 * @return The offset at which the joybus program is loaded
 */
uint joybus_port_init(joybus_port_t *port, uint pin, PIO pio, int sm, int offset);

/**
 * @brief Cleanly terminate the joybus PIO instance, freeing the state machine, and uninstalling the
 * joybus program from the PIO instance
 *
 * @param port Pointer to the port to terminate
 */
void joybus_port_terminate(joybus_port_t *port);

/**
 * @brief Reset the state machine to clean state ready to start receiving data from a connected
 * joybus device/host
 *
 * @param port The joybus instance to reset
 */
void joybus_port_reset(joybus_port_t *port);

/**
 * @brief Send a certain number of bytes to a joybus device/host, then attempt to receive back a
 * certain number of bytes
 *
 * @param port The joybus instance to use for sending and receiving bytes
 * @param message The bytes to send
 * @param message_len The number of bytes to send
 * @param response_buf The buffer to write received bytes into
 * @param response_len The number of bytes to attempt to receive
 * @param read_timeout_us How many microseconds to wait to receive each byte after the first before
 * timing out
 *
 * @return The actual number of bytes received
 */
uint joybus_send_receive(
    joybus_port_t *port,
    uint8_t *message,
    uint message_len,
    uint8_t *response_buf,
    uint response_len,
    uint read_timeout_us
);

/**
 * @brief Send a certain number of bytes to a joybus device/host
 *
 * @param port The joybus instance to send bytes to
 * @param bytes The bytes to send
 * @param len The number of bytes to send
 */
void joybus_send_bytes(joybus_port_t *port, uint8_t *bytes, uint len);

/**
 * @brief Send an individual byte to a joybus device/host
 *
 * @param port The joybus instance to send the byte to
 * @param byte The byte to send
 * @param stop If true, write a stop bit after the last byte to indicate end of transmission
 */
void joybus_send_byte(joybus_port_t *port, uint8_t byte, bool stop);

/**
 * @brief Attempt to receive a certain number of bytes from a joybus device/host
 *
 * @param port The joybus instance to receive bytes from
 * @param buf The buffer to write received bytes into
 * @param len The number of bytes to attempt to receive
 * @param timeout_us How many microseconds to wait before timing out for each byte after the first
 * @param first_byte_can_timeout If true, the timeout is also applied to the first byte
 *
 * @return The actual number of bytes received
 */
uint joybus_receive_bytes(
    joybus_port_t *port,
    uint8_t *buf,
    uint len,
    uint64_t timeout_us,
    bool first_byte_can_timeout
);

/**
 * @brief Receive a single byte from a joybus device/host
 *
 * @param port The joybus instance to receive the byte from
 *
 * @return The received byte
 */
uint8_t joybus_receive_byte(joybus_port_t *port);

#ifdef __cplusplus
}
#endif

#endif
