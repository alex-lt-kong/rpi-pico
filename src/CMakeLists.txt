add_library(helper
    helper.c
)

target_link_libraries(helper pico_stdlib)

add_subdirectory(dht20)
add_subdirectory(mqtt-client)