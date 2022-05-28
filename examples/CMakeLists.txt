cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(PICO_SDK_FETCH_FROM_GIT 1)
include(pico_sdk_import.cmake)

project(joybus_pio CXX C ASM)

pico_sdk_init()

include_directories(../include)

add_subdirectory(GamecubeConsole)
