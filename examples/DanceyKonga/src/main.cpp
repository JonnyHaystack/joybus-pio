/**
 * Use two DK Bongos layed-out in a diamond as a Dance Pad.
 */

#include "GamecubeConsole.hpp"
#include "GamecubeController.hpp"
#include "gamecube_definitions.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>
#include <Arduino.h>

/**
 * Raspberry Pi Pico pinout
 *                          __
 *                         |  |
 *  GC console      +------+--+------+
 *  port data  <----+1      usb    40|
 *                  |2             39+----> 3.3V power
 *  GC console <----+3             38|      from GC
 *  port ground     |4             37|      console
 *                +-+5             36|      port to Pico
 *  GC            | |6             35|      & Controllers
 *  Controller <--+ |7             34|
 *  2 data          |8             33+----> GC
 *                  |9             32|      Controller
 *  GC <------------+10            31|      Ground
 *  Controller      |11            30|
 *  1 data          |12            29|
 *            GC <--+13            28|
 *    Controller    |14            27|
 *        Ground    |15            26|
 *                  |16            25|
 * onboard button   |17            24|
 *        ground <--+18            23|
 *           &      |19            22|
 *        signal <--+20    o o o   21|
 *                  '----------------'
 */

// note please!
// Above: Pin Numbers
// Below: GPIO Numbers

#define ONBOARD_BUTTON_PIN 15
	// an extra button, separate from the DK Bongo pair
	
#define JOYBUS_OUTPUT_PIN 0
	// the GameCube console
	
#define JOYBUS_INPUT_PIN_1 7
	// Controller 1
	
#define JOYBUS_INPUT_PIN_2 3
	// Controller 2
	
#define LED_PIN LED_BUILTIN
	// An LED on the Pico. Will flash once GC is connected

/**
 * Records the relevant button state of the two DK Bongos,
 * so we can convert them to uint32_t and transmit from the
 * second pio core (1) to the first (0).
 */
struct controllers_pair_state {
	bool a1 = false;
	bool b1 = false;
	bool x1 = false;
	bool y1 = false;
	bool start1 = false;

	bool a2 = false;
	bool b2 = false;
	bool x2 = false;
	bool y2 = false;
	bool start2 = false;
};

uint32_t controllers_pair_state_to_uint32_t(controllers_pair_state w)
{
	uint32_t result=0;
	result = (result << 1) + w.a1;
	result = (result << 1) + w.b1;
	result = (result << 1) + w.x1;
	result = (result << 1) + w.y1;
	result = (result << 1) + w.start1;
	result = (result << 1) + w.a2;
	result = (result << 1) + w.b2;
	result = (result << 1) + w.x2;
	result = (result << 1) + w.y2;
	result = (result << 1) + w.start2;
	return result;
}

controllers_pair_state uint32_t_to_controllers_pair_state(uint32_t bits)
{
	controllers_pair_state result;
	uint32_t i=0u;
	result.start2 = ((bits >> i++) & 1);
	result.y2 = ((bits >> i++) & 1);
	result.x2 = ((bits >> i++) & 1);
	result.b2 = ((bits >> i++) & 1);
	result.a2 = ((bits >> i++) & 1);
	result.start1 = ((bits >> i++) & 1);
	result.y1 = ((bits >> i++) & 1);
	result.x1 = ((bits >> i++) & 1);
	result.b1 = ((bits >> i++) & 1);
	result.a1 = ((bits >> i++) & 1); 
	return result;
}

GamecubeConsole *gamecube_console;
GamecubeController *gamecube_controller_1;
GamecubeController *gamecube_controller_2;
gc_report_t gamecube_console_report = default_gc_report;  // only for use on first core

uint32_t output_state_bits = 0u;  // only for use on first core

gc_report_t gamecube_controller_1_report = default_gc_report;  // only for use on second core
gc_report_t gamecube_controller_2_report = default_gc_report;  // only for use on second core
controllers_pair_state input_state;  // only for use on second core
uint32_t input_state_watermark = 0;  // only for use on second core

bool core_0_setup = false;
bool core_1_setup = false;
bool gamecube_console_ready = false;

void trill_led(int times)
{
	for (int i=0; i < times; i++)
	{
		digitalWrite(LED_BUILTIN, HIGH);
		delay(70);
		digitalWrite(LED_BUILTIN, LOW);
		delay(70);
	}
}

void setup()
{
	set_sys_clock_khz(130'000, true);
	pinMode(LED_PIN, OUTPUT);
	pinMode(ONBOARD_BUTTON_PIN, INPUT_PULLUP);
	gamecube_console = new GamecubeConsole(JOYBUS_OUTPUT_PIN, pio0);
	core_0_setup = true;
	while (!core_1_setup)
	{
		tight_loop_contents();
	}
}

void setup1()
{
	while (!core_0_setup)
	{
		tight_loop_contents();
	}
	gamecube_controller_1 = new GamecubeController(JOYBUS_INPUT_PIN_1, 120, pio1);
	gamecube_controller_2 = new GamecubeController(JOYBUS_INPUT_PIN_2, 120, pio1, -1, gamecube_controller_1->GetOffset());
	core_1_setup = true;
}

void loop() {
	if (!gamecube_console_ready)
	{
		if (gamecube_console->Detect())
		{
			gamecube_console_ready = true;
			trill_led(3);
		}
		else
		{
			delay(50);
			return;
		}
	}

	while (rp2040.fifo.available() > 1)
	{
		rp2040.fifo.pop_nb(&output_state_bits);
	}

	gamecube_console->WaitForPollStart();
		
	/*
          DK Bongos pair
                                    ( ^ )
                                         \ A
                                          \
                            ( <- )         ( -> )
                                  \        
                                 B \       
                                    ( v )

          Dance Pad                                    
                         __________,----,__________
                        |          \____/          |
                        |   Z               START  |
                        |__________________________|
                        |        |        |        |
                        |   B    |   ^    |   A    |
                        |________|________|________|
                        |        |        |        |
                        |   <-   |        |   ->   |
                        |________|________|________|
                        |        |        |        |
                        |        |   v    |        |
                        |________|________|________|
	 */
	
	int button_state = !digitalRead(ONBOARD_BUTTON_PIN);

	if (rp2040.fifo.pop_nb(&output_state_bits))
	{
		controllers_pair_state output_state = uint32_t_to_controllers_pair_state(output_state_bits);
		gamecube_console_report.dpad_up = output_state.x1 || output_state.a1;
		gamecube_console_report.dpad_left = output_state.y2 || output_state.b2;
		gamecube_console_report.dpad_right = output_state.y1 || output_state.b1;
		gamecube_console_report.dpad_down = output_state.x2 || output_state.a2;
	
		gamecube_console_report.a = output_state.start1;
		gamecube_console_report.b = output_state.start2;
	}

	PollStatus status = gamecube_console->WaitForPollEnd();

	if (status == PollStatus::ERROR) {
		// alert user to problem
		digitalWrite(LED_BUILTIN, HIGH);
	}

	gamecube_console_report.start = button_state;
	gamecube_console->SendReport(&gamecube_console_report);
}

void loop1()
{
	bool gamecube_controller_1_responded = gamecube_controller_1->Poll(&gamecube_controller_1_report, 0);
	bool gamecube_controller_2_responded = gamecube_controller_2->Poll(&gamecube_controller_2_report, 0);

	/*
Unmodified DK Bongo to GameCube Controller button mapping
                                           _____       _____
  Left Bongo Top -     Y                  /     \     /     \
  Left Bongo Bottom -  B                 /   Y   \---/   X   \
  Right Bongo Top -    X                 |       | R |       |
  Right Bongo Bottom - A                 \   B   /---\   A   /
  Clap -               R                  \_____/  s  \_____/
  START/PAUSE -        START/PAUSE


Two DK Bongo Layout for Dancey Konga (unbent button mappings depicted)

                                    ( XA )       1
                                          \s
                                           \
                            ( YB )          ( YB )
                                  \         
                                  s\        
                            2       ( XA )

	 */

    if (gamecube_controller_1_responded)
    {
        input_state.a1 = gamecube_controller_1_report.a;
        input_state.b1 = gamecube_controller_1_report.b;
        input_state.x1 = gamecube_controller_1_report.x;
        input_state.y1 = gamecube_controller_1_report.y;
        input_state.start1 = gamecube_controller_1_report.start;
    }

    if (gamecube_controller_2_responded)
    {
        input_state.a2 = gamecube_controller_2_report.a;
        input_state.b2 = gamecube_controller_2_report.b;
        input_state.x2 = gamecube_controller_2_report.x;
        input_state.y2 = gamecube_controller_2_report.y;
        input_state.start2 = gamecube_controller_2_report.start;
    }

    if (gamecube_controller_1_responded || gamecube_controller_2_responded)
    {
	    uint32_t input_state_bits = controllers_pair_state_to_uint32_t(input_state);
        if (input_state_watermark != input_state_bits)
        {
            rp2040.fifo.push_nb(input_state_bits);
            input_state_watermark = input_state_bits;
        }
    }
}
