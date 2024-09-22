cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(MqttTest C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD "pico_w")


if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

add_definitions(-DWIFI_SSID="${WIFI_SSID}")
add_definitions(-DWIFI_PASSWORD="${WIFI_PASSWORD}")
add_definitions(-DMQTT_BROKER_IP="${MQTT_BROKER_IP}")

add_compile_options(-Wall -O2)       

add_executable(mqtt-client   mqtt-client.c)

target_include_directories(mqtt-client PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(mqtt-client
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        pico_lwip_mqtt
        )      

pico_add_extra_outputs(mqtt-client)

pico_enable_stdio_usb(mqtt-client 1) # enable usb output
pico_enable_stdio_uart(mqtt-client 1) # enable uart output

