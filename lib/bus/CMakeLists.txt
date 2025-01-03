add_library(robobus-bus INTERFACE)

target_include_directories(robobus-bus INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(robobus-bus INTERFACE
    syoch-robotics-logger rd16
)
