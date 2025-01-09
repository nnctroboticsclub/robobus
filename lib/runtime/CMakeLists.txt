add_library(robobus-runtime INTERFACE)

target_include_directories(robobus-runtime INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(robobus-runtime INTERFACE
    syoch-robotics-logger
    syoch-robotics-thread
)
