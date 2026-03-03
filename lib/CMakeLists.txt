add_subdirectory(bus)
add_subdirectory(runtime)

add_library(dummy STATIC
    source/dummy.cpp
)

target_link_libraries(dummy PRIVATE
    syoch-robotics::common
    robobus::bus
    robobus::runtime
)