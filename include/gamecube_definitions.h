#ifndef GAMECUBE_DEFINITIONS_H
#define GAMECUBE_DEFINITIONS_H

#include "pico/stdlib.h"

#define GC_CMD_PROBE 0x00
#define GC_CMD_RECALIBRATE 0xFF
#define GC_CMD_ORIGIN 0x41
#define GC_CMD_POLL 0x40

typedef struct {
    uint8_t a : 1;
    uint8_t b : 1;
    uint8_t x : 1;
    uint8_t y : 1;
    uint8_t start : 1;
    uint8_t reserved0 : 3;

    uint8_t dpad_left : 1;
    uint8_t dpad_right : 1;
    uint8_t dpad_down : 1;
    uint8_t dpad_up : 1;
    uint8_t z : 1;
    uint8_t r : 1;
    uint8_t l : 1;
    uint8_t reserved1 : 1;

    uint8_t stick_x;
    uint8_t stick_y;
    uint8_t cstick_x;
    uint8_t cstick_y;
    uint8_t l_analog;
    uint8_t r_analog;
} gc_report_t;

#endif /* end of include guard: GAMECUBE_DEFINITIONS_H */
