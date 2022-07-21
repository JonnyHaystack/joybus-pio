add_executable(
  N64Controller
  main.cpp
  ../../src/joybus.c
  ../../src/N64Controller.cpp
)

pico_generate_pio_header(N64Controller ${CMAKE_CURRENT_LIST_DIR}/../../src/joybus.pio)

target_link_libraries(N64Controller pico_stdlib hardware_pio)

pico_enable_stdio_usb(N64Controller 1)
pico_enable_stdio_uart(N64Controller 0)

pico_add_extra_outputs(N64Controller)
