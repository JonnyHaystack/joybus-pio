#include "N64Console.hpp"

#include "n64_definitions.h"
#include "joybus.h"

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

    // We make at most 10 attempts to receive and respond to PROBE commands from a console. GameCube
    // and N64 both have the same PROBE command (0x00), so we don't know for sure that we're
    // connected to a GameCube until we receive a GameCube ORIGIN command.
    for (uint8_t attempts = 0; attempts < 110; attempts++) {
        // Always apply timeout (10ms), so that we don't block indefinitely if nothing is connected.
        if (joybus_receive_bytes(&_port, received, 1, 10'000, true) != 1) {
            continue;
        }

        switch (received[0]) {
            case RESET_N64:
            case PROBE_N64:
                busy_wait_us(reply_delay);
                joybus_send_bytes(&_port, (uint8_t *)&default_n64_status, sizeof(n64_status_t));
                attempts = 0;
                //busy_wait_us(100);
                break;
            case POLL_N64:
                busy_wait_us(reply_delay);
                joybus_send_bytes(&_port, (uint8_t *)&default_n64_report, sizeof(n64_report_t));
                busy_wait_us(150); //Wait for command to finish sending before returning
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
    while (true) {
        WaitForPollStart();
        PollStatus_N64 status = WaitForPollEnd();

        if (status == PollStatus_N64::ERROR) {
            // If poll is invalid, wait long enough for command to finish, then
            // reset receiving.
            busy_wait_us(reset_wait_period_us);
            joybus_port_reset(&_port);
            continue;
        }

        return status == PollStatus_N64::RUMBLE_ON ? true : false;
    }
}

void __no_inline_not_in_flash_func(N64Console::WaitForPollStart)() {
    // Buffer for receiving command.
    uint8_t received[1];
    uint received_len;

    while (true) {
        joybus_receive_bytes(&_port, received, 1, receive_timeout_us, false);

        switch (received[0]) {
            case RESET_N64:
            case PROBE_N64:
                // Wait for stop bit before responding.
                busy_wait_us(reply_delay);
                joybus_send_bytes(&_port, (uint8_t *)&default_n64_status, sizeof(n64_status_t));
                break;
            case POLL_N64:
                //busy_wait_us(reply_delay);
                return;
            default:
                // If we received an invalid command, wait long enough for command
                // to finish, then reset receiving.
                busy_wait_us(reset_wait_period_us);
                joybus_port_reset(&_port);
        }
    }
}

/*
PollStatus_N64 __no_inline_not_in_flash_func(N64Console::WaitForPollEnd)() {
    // Buffer for receiving command.
    uint8_t received[2];
    uint received_len;

    // Poll command is 3 bytes total, so read the next 2 bytes now.
    received_len = joybus_receive_bytes(&_port, received, 2, receive_timeout_us, true);

    // Second byte indicates the reading mode.
    if (received_len != 2 || received[0] > 0x07) {
        // Return error status in the case of a timeout or invalid reading mode.
        return PollStatus_N64::ERROR;
    }

    // Store reading mode to be used for report conversion when report is sent.
    _reading_mode = received[0];

    // Set timeout for how long to wait until we can send response because we don't
    // want to reply before the console is done sending.
    _receive_end = make_timeout_time_us(reply_delay);

    // Return value of rumble bit (least significant bit in last byte).
    return received[1] & 0x01 ? PollStatus_N64::RUMBLE_ON : PollStatus_N64::RUMBLE_OFF;
}
*/

void __no_inline_not_in_flash_func(N64Console::SendReport)(n64_report_t *report) {
    // Wait for receive timeout to end before responding.
    //while (!time_reached(_receive_end)) {
    //    tight_loop_contents();
    //}
    // TODO: Translate report according to reading mode.
    joybus_send_bytes(&_port, (uint8_t *)report, sizeof(n64_report_t));
}

int N64Console::GetOffset() {
    return _port.offset;
}
