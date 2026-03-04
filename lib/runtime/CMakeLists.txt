add_library(runtime INTERFACE)
add_library(robobus::runtime ALIAS runtime)

target_include_directories(runtime INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/robobus/runtime>
)

target_link_libraries(runtime INTERFACE
    syoch-robotics::common
    syoch-robotics::logger
)

if (${NON_THREAD})
    target_compile_definitions(runtime INTERFACE
        NON_THREAD=1
    )
else()
    message(STATUS "Using thread (runtime) (rr; ${NON_THREAD}, csp: ${CMAKE_SUB_PROJECT}) <<< TRACK ME!")
    target_compile_definitions(runtime INTERFACE
        NON_THREAD=0
    )
endif()

install(TARGETS runtime
  EXPORT robobusTargets
  INCLUDES DESTINATION include/robobus/runtime
)

install(DIRECTORY include/
  DESTINATION include/robobus/runtime
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
