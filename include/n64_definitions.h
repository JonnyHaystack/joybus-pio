#ifndef _JOYBUS_N64_DEFINITIONS_H
#define _JOYBUS_N64_DEFINITIONS_H

#include <pico/stdlib.h>

enum command_n64 {
    PROBE_N64 = 0x00,
    RESET_N64 = 0xFF,
    POLL_N64 = 0x01,
    WRITE_N64 = 0x02 //Unused at the moment
};

typedef struct __attribute__((packed)) {
    uint8_t dpad_right : 1;
    uint8_t dpad_left : 1;
    uint8_t dpad_down : 1;
    uint8_t dpad_up : 1;
    uint8_t start : 1;
    uint8_t z : 1;
    uint8_t b : 1;
    uint8_t a : 1;

    uint8_t cbutton_right : 1;
    uint8_t cbutton_left : 1;
    uint8_t cbutton_down : 1;
    uint8_t cbutton_up : 1;
    uint8_t r : 1;
    uint8_t l : 1;
    uint8_t reserved1 : 1;
    uint8_t origin : 1;

    uint8_t stick_x;
    uint8_t stick_y;
} n64_report_t;

typedef struct __attribute__((packed)) {
    n64_report_t initial_inputs;
} n64_origin_t;

typedef struct __attribute__((packed)) {
    uint16_t device;
    uint8_t status;
} n64_status_t;

static constexpr n64_report_t default_n64_report = {
    .dpad_right = 0,
    .dpad_left = 0,
    .dpad_down = 0,
    .dpad_up = 0,
    .start = 0,
    .z = 0,
    .b = 0,
    .a = 0,

    .cbutton_right = 0,
    .cbutton_left = 0,
    .cbutton_down = 0,
    .cbutton_up = 0,
    .r = 0,
    .l = 0,
    .reserved1 = 0,
    .origin = 0,

    .stick_x = 0,
    .stick_y = 0,
};

static constexpr n64_origin_t default_n64_origin = {
    .initial_inputs = default_n64_report,
};

static constexpr n64_status_t default_n64_status = {
    .device = 0x0005,
    .status = 0x02,
};

#endif
