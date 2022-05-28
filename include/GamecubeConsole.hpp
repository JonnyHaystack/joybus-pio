#ifndef _JOYBUS_GAMECUBECONSOLE_HPP
#define _JOYBUS_GAMECUBECONSOLE_HPP

#include "gamecube_definitions.h"
#include "joybus.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>

class GamecubeConsole {
  public:
    GamecubeConsole(uint pin, PIO pio, int sm = -1, int offset = -1);
    ~GamecubeConsole();
    bool WaitForPoll();
    void SendReport(gc_report_t *report);
    int GetOffset();

  private:
    enum command {
        PROBE = 0x00,
        RESET = 0xFF,
        ORIGIN = 0x41,
        RECALIBRATE = 0x42,
        POLL = 0x40,
    };

    static constexpr uint incoming_bit_length_us = 5;
    static constexpr uint max_command_bytes = 3;
    static constexpr uint receive_timeout_us = incoming_bit_length_us * 10;
    static constexpr uint reset_wait_period_us =
        (incoming_bit_length_us * 8) * (max_command_bytes - 1) + receive_timeout_us;
    static constexpr uint64_t reply_delay = incoming_bit_length_us - 1;

    joybus_port_t _port;
    absolute_time_t _receive_end;
};

#endif
