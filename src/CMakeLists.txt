add_library(helper
    helper.c
)

target_link_libraries(helper pico_stdlib)

add_subdirectory(dht20)
add_subdirectory(mqtt-temp-collector)
add_subdirectory(mqtt-sample-client)
add_subdirectory(mqtts-sample-client)
add_subdirectory(blink)