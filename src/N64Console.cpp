#include "N64Console.hpp"

#include "joybus.h"
#include "n64_definitions.h"

#include <hardware/pio.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>
#include <pico/time.h>

N64Console::N64Console(uint pin, PIO pio, int sm, int offset) {
    joybus_port_init(&_port, pin, pio, sm, offset);
}

N64Console::~N64Console() {
    joybus_port_terminate(&_port);
}

bool __no_inline_not_in_flash_func(N64Console::Detect)() {
    // Buffer for receiving command.
    uint8_t received[1];

    // We make at most 60 attempts to receive and respond to PROBE commands from a console. GameCube
    // and N64 both have the same PROBE command (0x00), so we don't know for sure that we're
    // connected to an N64 until we receive a N64 ORIGIN command.
    for (uint8_t attempts = 0; attempts < 60; attempts++) {
        // Always apply timeout (20ms), so that we don't block indefinitely if nothing is connected.
        if (joybus_receive_bytes(&_port, received, 1, 20'000, true) != 1) {
            continue;
        }

        switch ((N64Command)received[0]) {
            case N64Command::RESET:
            case N64Command::PROBE:
                busy_wait_us(reply_delay);
                joybus_send_bytes(&_port, (uint8_t *)&default_n64_status, sizeof(n64_status_t));
                attempts = 0;
                break;
            case N64Command::POLL:
                busy_wait_us(reply_delay);
                joybus_send_bytes(&_port, (uint8_t *)&default_n64_report, sizeof(n64_report_t));
                return true;
            default:
                // If we received an invalid command, wait long enough for command
                // to finish, then reset receiving.
                busy_wait_us(reset_wait_period_us);
                joybus_port_reset(&_port);
        }
    }

    return false;
}

bool __no_inline_not_in_flash_func(N64Console::WaitForPoll)() {
    // Buffer for receiving command.
    uint8_t received[1];
    uint received_len;

    while (true) {
        joybus_receive_bytes(&_port, received, 1, receive_timeout_us, false);

        switch ((N64Command)received[0]) {
            case N64Command::RESET:
            case N64Command::PROBE:
                // Wait for stop bit before responding.
                busy_wait_us(reply_delay);
                joybus_send_bytes(&_port, (uint8_t *)&default_n64_status, sizeof(n64_status_t));
                break;
            case N64Command::POLL:
                // Set timeout for how long to wait until we can send response because we don't
                // want to reply before the console is done sending.
                _receive_end = make_timeout_time_us(reply_delay);

                return false;
            default:
                // If we received an invalid command, wait long enough for command
                // to finish, then reset receiving.
                busy_wait_us(reset_wait_period_us);
                joybus_port_reset(&_port);
        }
    }
}

void __no_inline_not_in_flash_func(N64Console::SendReport)(n64_report_t *report) {
    // Wait for receive timeout to end before responding.
    while (!time_reached(_receive_end)) {
        tight_loop_contents();
    }

    joybus_send_bytes(&_port, (uint8_t *)report, sizeof(n64_report_t));
}

int N64Console::GetOffset() {
    return _port.offset;
}
