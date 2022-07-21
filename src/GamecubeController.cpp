#include "GamecubeController.hpp"

#include <hardware/pio.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>
#include <pico/time.h>

GamecubeController::GamecubeController(uint pin, uint polling_rate, PIO pio, int sm, int offset) {
    joybus_port_init(&_port, pin, pio, sm, offset);
    _polling_period_us = 1'000'000 / polling_rate;
    _next_poll = get_absolute_time();
}

GamecubeController::~GamecubeController() {
    joybus_port_terminate(&_port);
}

void __no_inline_not_in_flash_func(GamecubeController::_wait_poll_cooldown)() {
    // Wait for start of next polling period.
    while (!time_reached(_next_poll)) {
        tight_loop_contents();
    }

    // Reset poll cooldown.
    _next_poll = make_timeout_time_us(_polling_period_us);
}

bool __no_inline_not_in_flash_func(GamecubeController::_init)() {
    // Send probe command.
    uint8_t probe_cmd[] = { (uint8_t)GamecubeCommand::PROBE };
    joybus_send_bytes(&_port, probe_cmd, sizeof(probe_cmd));

    // Read and validate probe response.
    uint received_len = joybus_receive_bytes(
        &_port,
        (uint8_t *)&_status,
        sizeof(gc_status_t),
        receive_timeout_us,
        true
    );

    // If response is invalid, return false.
    if (received_len != sizeof(gc_status_t) || _status.device == 0) {
        return false;
    }

    // Wait until start of next polling period before sending origin.
    _wait_poll_cooldown();

    // Send origin command.
    uint8_t origin_cmd[] = { (uint8_t)GamecubeCommand::ORIGIN };
    joybus_send_bytes(&_port, origin_cmd, sizeof(origin_cmd));

    // Read and validate origin response.
    gc_origin_t origin;
    received_len = joybus_receive_bytes(
        &_port,
        (uint8_t *)&origin,
        sizeof(gc_origin_t),
        receive_timeout_us,
        true
    );

    // If response is invalid, return false.
    if (received_len != sizeof(gc_origin_t)) {
        return false;
    }

    return true;
}

bool __no_inline_not_in_flash_func(GamecubeController::Poll)(gc_report_t *report, bool rumble) {
    // If controller is uninitialized, do probe/origin sequence.
    if (!_initialized) {
        _wait_poll_cooldown();

        _initialized = _init();
        if (!_initialized) {
            return false;
        }
    }

    _wait_poll_cooldown();

    // Send poll command.
    uint8_t poll_cmd[] = { (uint8_t)GamecubeCommand::POLL, 0x03, rumble };
    joybus_send_bytes(&_port, poll_cmd, sizeof(poll_cmd));

    // Read and validate report.
    uint8_t received_len = joybus_receive_bytes(
        &_port,
        (uint8_t *)report,
        sizeof(gc_report_t),
        receive_timeout_us,
        true
    );

    // If report origin bit is 1, it indicates that the controller is not initialized properly, so
    // we want to restart the initialization process.
    if (received_len != sizeof(gc_report_t) || report->origin) {
        _initialized = false;
        return false;
    }

    return true;
}

int GamecubeController::GetOffset() {
    return _port.offset;
}
