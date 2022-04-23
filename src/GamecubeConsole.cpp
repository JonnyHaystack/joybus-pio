#include "GamecubeConsole.hpp"

#include "gamecube_definitions.h"
#include "joybus.h"

#include "hardware/pio.h"
#include "pico/stdlib.h"

const uint GamecubeConsole::incoming_bit_length_us = 5;
const uint GamecubeConsole::max_command_len = 3;

GamecubeConsole::GamecubeConsole(uint pin, PIO pio) {
    joybus_port_init(&port, pin, pio, -1, -1);
}

GamecubeConsole::GamecubeConsole(uint pin, PIO pio, uint sm) {
    joybus_port_init(&port, pin, pio, sm, -1);
}

GamecubeConsole::GamecubeConsole(uint pin, PIO pio, uint sm, uint offset) {
    joybus_port_init(&port, pin, pio, sm, offset);

    receive_timeout_us = incoming_bit_length_us * 10;
    reset_wait_period_us =
        (incoming_bit_length_us * 8) * (max_command_len - 1) + receive_timeout_us;
}

GamecubeConsole::~GamecubeConsole() {
    joybus_port_terminate(&port);
}

bool GamecubeConsole::WaitForPoll() {
    uint8_t response[3];
    uint response_len;

    while (true) {
        // TODO: Experiment with reading just one byte, checking it against
        // known commands, then reading more if it's a command with more length.
        uint response_len =
            joybus_receive_bytes(&port, response, sizeof(response), receive_timeout_us);

        if (response_len == 1 && response[0] == 0x00) {
            uint8_t status[] = { 0x09, 0x00, 0x03 };
            joybus_send_bytes(&port, status, sizeof(status));
        } else if (response_len == 1 && response[0] == 0x41) {
            uint8_t origin[] = { 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x1F, 0x1F, 0x00, 0x00 };
            joybus_send_bytes(&port, origin, sizeof(origin));
        } else if (response_len == 3 && response[0] == 0x40 && response[1] <= 0x07) {
            // Return value of rumble bit (least significant bit in last byte).
            return response[2] & 0x01;
        } else {
            // If we received an invalid command, wait long enough for command
            // to finish, then reset receiving.
            sleep_us(reset_wait_period_us);
            joybus_port_reset(&port);
        }
    }
}

void GamecubeConsole::SendReport(gc_report_t *report) {
    joybus_send_bytes(&port, (uint8_t *)report, sizeof(gc_report_t));
}