add_executable(
  GamecubeController
  main.cpp
  ../../src/joybus.c
  ../../src/GamecubeController.cpp
)

pico_generate_pio_header(GamecubeController ${CMAKE_CURRENT_LIST_DIR}/../../src/joybus.pio)

target_link_libraries(GamecubeController pico_stdlib hardware_pio)

pico_enable_stdio_usb(GamecubeController 1)
pico_enable_stdio_uart(GamecubeController 0)

pico_add_extra_outputs(GamecubeController)
