cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(PICO_SDK_FETCH_FROM_GIT 1)
include(pico_sdk_import.cmake)

project(joybus_pio CXX C ASM)

pico_sdk_init()

include_directories(include)

add_executable(
  gc_console_test
  test/gc_console.cpp
  src/joybus.c
  src/GamecubeConsole.cpp
)

pico_generate_pio_header(gc_console_test ${CMAKE_CURRENT_LIST_DIR}/src/joybus.pio)

target_link_libraries(gc_console_test pico_stdlib hardware_pio)

pico_enable_stdio_usb(gc_console_test 1)
pico_enable_stdio_uart(gc_console_test 0)

pico_add_extra_outputs(gc_console_test)
