add_executable(
  GamecubeConsole
  main.cpp
  ../../src/joybus.c
  ../../src/GamecubeConsole.cpp
)

pico_generate_pio_header(GamecubeConsole ${CMAKE_CURRENT_LIST_DIR}/../../src/joybus.pio)

target_link_libraries(GamecubeConsole pico_stdlib hardware_pio)

pico_enable_stdio_usb(GamecubeConsole 1)
pico_enable_stdio_uart(GamecubeConsole 0)

pico_add_extra_outputs(GamecubeConsole)
