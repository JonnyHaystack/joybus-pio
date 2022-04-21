cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(PICO_SDK_FETCH_FROM_GIT 1)
include(pico_sdk_import.cmake)

project(joybus_pio CXX C ASM)

pico_sdk_init()

include_directories(include)

add_executable(
  joybus_pio
  src/main.cpp
)

pico_generate_pio_header(joybus_pio ${CMAKE_CURRENT_LIST_DIR}/src/joybus.pio)

#target_sources(joybus_pio src/main.c)

target_link_libraries(joybus_pio pico_stdlib hardware_pio)

pico_enable_stdio_usb(joybus_pio 1)
pico_enable_stdio_uart(joybus_pio 0)

pico_add_extra_outputs(joybus_pio)
