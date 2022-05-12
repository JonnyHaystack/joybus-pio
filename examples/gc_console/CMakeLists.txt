add_executable(
  gc_console
  gc_console.cpp
  ../../src/joybus.c
  ../../src/GamecubeConsole.cpp
)

pico_generate_pio_header(gc_console ${CMAKE_CURRENT_LIST_DIR}/../../src/joybus.pio)

target_link_libraries(gc_console pico_stdlib hardware_pio)

pico_enable_stdio_usb(gc_console 1)
pico_enable_stdio_uart(gc_console 0)

pico_add_extra_outputs(gc_console)
