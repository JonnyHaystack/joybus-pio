#ifndef _JOYBUS_GAMECUBECONTROLLER_HPP
#define _JOYBUS_GAMECUBECONTROLLER_HPP

#include "gamecube_definitions.h"
#include "joybus.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>

class GamecubeController {
  public:
    /**
     * @brief Construct a new GamecubeController object
     *
     * @param pin The GPIO pin that the GameCube controller's data line is connected to
     * @param polling_rate The frequency (in Hz) at which to poll the controller
     * @param pio The PIO instance; either pio0 or pio1. Default is pio0.
     * @param sm The PIO state machine to run the joybus instance on. Default is to automatically
     * claim an unused one.
     * @param offset The instruction memory offset at which to load the PIO program. Default is to
     * allocate automatically.
     */
    GamecubeController(uint pin, uint polling_rate, PIO pio = pio0, int sm = -1, int offset = -1);

    /**
     * @brief Cleanly terminate the joybus PIO instance, freeing the state machine, and uninstalling
     * the joybus program from the PIO instance
     */
    ~GamecubeController();

    /**
     * @brief Send a poll to the GameCube controller. Delay will be added if necessary to conform
     * with the chosen polling rate.
     *
     * @param report The report buffer to write the controller's response into
     * @param rumble True to enable rumble, false to disable
     *
     * @return true if the controller responded, false otherwise
     */
    bool Poll(gc_report_t *report, bool rumble);

    /**
     * @brief Get the offset at which the PIO program was installed. Useful if you want to
     * communicate with multiple joybus devices without having to load multiple copies of the PIO
     * program.
     *
     * @return The offset at which the PIO program was installed
     */
    int GetOffset();

  private:
    static constexpr uint incoming_bit_length_us = 4;
    // Give a whole 5 bytes of leniency on receive so the controller has time to do processing
    // before responding. And another byte of leniency because the joybus_receive_bytes (and thus
    // the timeout) starts soon after the PIO starts sending our first byte.
    static constexpr uint receive_timeout_us = 5 * incoming_bit_length_us * 10;

    joybus_port_t _port;
    uint _polling_period_us;
    gc_status_t _status;
    bool _initialized;
    absolute_time_t _next_poll;

    bool _init();
    void _wait_poll_cooldown();
};

#endif