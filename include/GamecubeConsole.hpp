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
    joybus_port_t port;
};

#endif
