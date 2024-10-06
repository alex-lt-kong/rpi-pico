cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
project(blink C CXX ASM)

include_directories(src)
add_executable(blink ./src/main.c)

pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 1)
pico_add_extra_outputs(blink)
target_include_directories(blink PRIVATE ${CMAKE_CURRENT_LIST_DIR} )
# pull in common dependencies
target_link_libraries(blink pico_stdlib pico_cyw43_arch_lwip_threadsafe_background hardware_adc)

