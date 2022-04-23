#ifndef GAMECUBECONSOLE_H
#define GAMECUBECONSOLE_H

#include "joybus.hpp"

#include "hardware/pio.h"

#include "gamecube_definitions.h"

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
