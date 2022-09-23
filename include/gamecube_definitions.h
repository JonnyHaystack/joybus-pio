#ifndef _JOYBUS_GAMECUBE_DEFINITIONS_H
#define _JOYBUS_GAMECUBE_DEFINITIONS_H

#include <pico/stdlib.h>

enum class GamecubeCommand {
    PROBE = 0x00,
    RESET = 0xFF,
    ORIGIN = 0x41,
    RECALIBRATE = 0x42,
    POLL = 0x40,
};

typedef struct __attribute__((packed)) {
    bool a : 1;
    bool b : 1;
    bool x : 1;
    bool y : 1;
    bool start : 1;
    bool origin : 1;
    uint8_t reserved0 : 2;

    bool dpad_left : 1;
    bool dpad_right : 1;
    bool dpad_down : 1;
    bool dpad_up : 1;
    bool z : 1;
    bool r : 1;
    bool l : 1;
    uint8_t reserved1 : 1;

    uint8_t stick_x;
    uint8_t stick_y;
    uint8_t cstick_x;
    uint8_t cstick_y;
    uint8_t l_analog;
    uint8_t r_analog;
} gc_report_t;

typedef struct __attribute__((packed)) {
    gc_report_t initial_inputs;
    uint8_t reserved0;
    uint8_t reserved1;
} gc_origin_t;

typedef struct __attribute__((packed)) {
    uint16_t device;
    uint8_t status;
} gc_status_t;

static constexpr gc_report_t default_gc_report = {
    .a = 0,
    .b = 0,
    .x = 0,
    .y = 0,
    .start = 0,
    .origin = 0,
    .reserved0 = 0,
    .dpad_left = 0,
    .dpad_right = 0,
    .dpad_down = 0,
    .dpad_up = 0,
    .z = 0,
    .r = 0,
    .l = 0,
    .reserved1 = 1,
    .stick_x = 128,
    .stick_y = 128,
    .cstick_x = 128,
    .cstick_y = 128,
    .l_analog = 0,
    .r_analog = 0,
};

static constexpr gc_origin_t default_gc_origin = {
    .initial_inputs = default_gc_report,
    .reserved0 = 0,
    .reserved1 = 0,
};

static constexpr gc_status_t default_gc_status = {
    .device = 0x0009,
    .status = 0x03,
};

#endif
