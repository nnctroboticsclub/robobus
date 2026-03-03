add_library(robobus-runtime INTERFACE)
find_package(syoch-robotics REQUIRED)

target_include_directories(robobus-runtime INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(robobus-runtime INTERFACE
    syoch-robotics::common
    syoch-robotics::logger
)

if (${NON_THREAD})
    target_compile_definitions(robobus-runtime INTERFACE
        NON_THREAD=1
    )
else()
    message(STATUS "Using thread (robobus-runtime) (rr; ${NON_THREAD}, csp: ${CMAKE_SUB_PROJECT}) <<< TRACK ME!")
    target_compile_definitions(robobus-runtime INTERFACE
        NON_THREAD=0
    )
endif()
