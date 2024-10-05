cmake_minimum_required(VERSION 3.20)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

include_directories(src/dht20)

add_executable(dht20-driver
    src/main.c
)


add_subdirectory(src/dht20)
#target_include_directories(dht20 PUBLIC dht20)
#target_link_directories(dht20 PUBLIC dht20)

pico_enable_stdio_usb(dht20-driver 1)
pico_enable_stdio_uart(dht20-driver 1)
pico_add_extra_outputs(dht20-driver)
target_link_libraries(dht20-driver pico_stdlib hardware_i2c dht20)