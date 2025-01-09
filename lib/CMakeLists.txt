add_subdirectory(bus)
add_subdirectory(runtime)

add_library(robobus-dummy STATIC
    source/dummy.cpp
)

target_link_libraries(robobus-dummy
    robobus-bus
    robobus-runtime
)