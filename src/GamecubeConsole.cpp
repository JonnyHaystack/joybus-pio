#include "GamecubeConsole.hpp"

#include "gamecube_definitions.h"
#include "joybus.h"

#include <hardware/pio.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>
#include <pico/time.h>

GamecubeConsole::GamecubeConsole(uint pin, PIO pio, int sm, int offset) {
    joybus_port_init(&_port, pin, pio, sm, offset);
}

GamecubeConsole::~GamecubeConsole() {
    joybus_port_terminate(&_port);
}

bool __not_in_flash_func(GamecubeConsole::WaitForPoll)() {
    // Buffer for receiving command.
    uint8_t received[2];
    uint received_len;

    while (true) {
        joybus_receive_bytes(&_port, received, 1, receive_timeout_us, false);

        switch (received[0]) {
            case RESET:
            case PROBE:
                // Wait for stop bit before responding.
                sleep_us(reply_delay);
                joybus_send_bytes(&_port, (uint8_t *)&default_gc_status, sizeof(gc_status_t));
                break;
            case RECALIBRATE:
            case ORIGIN:
                // Wait for stop bit before responding.
                sleep_us(reply_delay);
                joybus_send_bytes(&_port, (uint8_t *)&default_gc_origin, sizeof(gc_origin_t));
                break;
            case POLL:
                // Poll command is 3 bytes total, so read the next 2 bytes now.
                received_len = joybus_receive_bytes(&_port, received, 2, receive_timeout_us, true);
                // Second byte indicates the reading mode.
                // TODO: Implement other reading modes
                if (received_len == 2 && received[0] <= 0x07) {
                    // Set timeout for how long to wait until we can send response because we don't
                    // want to reply before the console is done sending.
                    _receive_end = make_timeout_time_us(reply_delay);

                    // Return value of rumble bit (least significant bit in last byte).
                    return received[1] & 0x01;
                }
                break;
            default:
                // If we received an invalid command, wait long enough for command
                // to finish, then reset receiving.
                sleep_us(reset_wait_period_us);
                joybus_port_reset(&_port);
        }
    }
}

void __not_in_flash_func(GamecubeConsole::SendReport)(gc_report_t *report) {
    // Wait for receive timeout to end before responding.
    while (!time_reached(_receive_end)) {
        tight_loop_contents();
    }
    joybus_send_bytes(&_port, (uint8_t *)report, sizeof(gc_report_t));
}

int GamecubeConsole::GetOffset() {
    return _port.offset;
}
