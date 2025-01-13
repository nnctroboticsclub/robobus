add_library(robobus-runtime INTERFACE)

target_include_directories(robobus-runtime INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(robobus-runtime INTERFACE
    syoch-robotics-common
    syoch-robotics-logger
)

if (NON_THREAD)
    target_compile_definitions(robobus-runtime INTERFACE
        NON_THREAD=1
    )
else()
    target_link_libraries(robobus-runtime INTERFACE
        syoch-robotics-thread
    )
endif()