#ifndef _JOYBUS_N64_DEFINITIONS_H
#define _JOYBUS_N64_DEFINITIONS_H

#include <pico/stdlib.h>

enum class N64Command {
    PROBE = 0x00,
    RESET = 0xFF,
    POLL = 0x01,
    READ_EXPANSION_BUS = 0x02,
    WRITE_EXPANSION_BUS = 0x03,
};

typedef struct __attribute__((packed)) {
    bool dpad_right : 1;
    bool dpad_left : 1;
    bool dpad_down : 1;
    bool dpad_up : 1;
    bool start : 1;
    bool z : 1;
    bool b : 1;
    bool a : 1;

    bool c_right : 1;
    bool c_left : 1;
    bool c_down : 1;
    bool c_up : 1;
    bool r : 1;
    bool l : 1;
    uint8_t reserved1 : 1;
    uint8_t reserved0 : 1;

    uint8_t stick_x;
    uint8_t stick_y;
} n64_report_t;

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
    .c_right = 0,
    .c_left = 0,
    .c_down = 0,
    .c_up = 0,
    .r = 0,
    .l = 0,
    .reserved1 = 0,
    .reserved0 = 0,
    .stick_x = 0,
    .stick_y = 0,
};

static constexpr n64_status_t default_n64_status = {
    .device = 0x0005,
    .status = 0x02,
};

#endif
