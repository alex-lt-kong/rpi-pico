cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(mqtt-temp-collector C CXX ASM)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

#set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

#add_definitions(-DWIFI_SSID="${WIFI_SSID}")
#add_definitions(-DWIFI_PASSWORD="${WIFI_PASSWORD}")
#add_definitions(-DMQTT_BROKER_IP="${MQTT_BROKER_IP}")
#add_definitions(-DMQTT_BROKER_HOSTNAME="${MQTT_BROKER_HOSTNAME}")

add_compile_options(-Wall -O2)

add_executable(mqtt-temp-collector mqtt-temp-collector.c)

target_include_directories(mqtt-temp-collector PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}
)

# include_directories(../dht20/src/dht20)
include_directories(..)

target_link_libraries(mqtt-temp-collector helper
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mbedtls
        pico_mbedtls
        pico_lwip_mqtt
        hardware_adc
        pico_stdlib hardware_i2c pico_binary_info dht20
)

pico_add_extra_outputs(mqtt-temp-collector)
pico_enable_stdio_usb(mqtt-temp-collector 1) # enable usb output
pico_enable_stdio_uart(mqtt-temp-collector 1) # enable uart output

