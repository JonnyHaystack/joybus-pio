#ifndef _JOYBUS_GAMECUBECONSOLE_HPP
#define _JOYBUS_GAMECUBECONSOLE_HPP

#include "gamecube_definitions.h"
#include "joybus.h"

#include "hardware/pio.h"
#include "pico/stdlib.h"

class GamecubeConsole {
  public:
    GamecubeConsole(uint pin, PIO pio);
    GamecubeConsole(uint pin, PIO pio, uint sm);
    GamecubeConsole(uint pin, PIO pio, uint sm, uint offset);
    ~GamecubeConsole();
    bool WaitForPoll();
    void SendReport(gc_report_t *report);

  private:
    enum command {
        PROBE = 0x00,
        RESET = 0xFF,
        ORIGIN = 0x41,
        RECALIBRATE = 0x42,
        POLL = 0x40,
    };

    joybus_port_t port;
    static const uint incoming_bit_length_us;
    static const uint max_command_len;
    uint receive_timeout_us;
    uint reset_wait_period_us;
};

#endif
