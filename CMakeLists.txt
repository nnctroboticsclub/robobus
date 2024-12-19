add_library(robobus INTERFACE)

target_include_directories(robobus INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(robobus INTERFACE
    syoch-robotics-logger
    dcan
    rd16
)
