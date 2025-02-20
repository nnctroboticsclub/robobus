add_library(robobus-runtime INTERFACE)

target_include_directories(robobus-runtime INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(robobus-runtime INTERFACE
    syoch-robotics-common
    syoch-robotics-logger
)

if (${NON_THREAD})
    target_compile_definitions(robobus-runtime INTERFACE
        NON_THREAD=1
    )
else()
    message(STATUS "Using thread (rr; ${NON_THREAD}) <<< TRACK ME!")
    target_compile_definitions(robobus-runtime INTERFACE
        NON_THREAD=0
    )
    target_link_libraries(robobus-runtime INTERFACE
        syoch-robotics-thread
    )
endif()