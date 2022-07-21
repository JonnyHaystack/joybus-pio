add_executable(
  N64Console
  main.cpp
  ../../src/joybus.c
  ../../src/N64Console.cpp
)

pico_generate_pio_header(N64Console ${CMAKE_CURRENT_LIST_DIR}/../../src/joybus.pio)

target_link_libraries(N64Console pico_stdlib hardware_pio)

pico_enable_stdio_usb(N64Console 1)
pico_enable_stdio_uart(N64Console 0)

pico_add_extra_outputs(N64Console)
