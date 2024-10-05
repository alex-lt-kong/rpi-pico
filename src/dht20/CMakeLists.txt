cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(dht20_driver C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD "pico_w")

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

add_compile_options(-Wall -O2)

add_executable(dht20-driver src/main.c)

add_subdirectory("dht20-pico")
target_include_directories(dht20 PUBLIC "dht20-pico")
target_link_libraries(dht20-driver pico_stdlib hardware_i2c pico_binary_info dht20)

pico_add_extra_outputs(dht20-driver)
pico_enable_stdio_usb(dht20-driver 1)
pico_enable_stdio_uart(dht20-driver 1)

