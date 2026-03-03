add_library(bus STATIC
    include/robobus/network/interface/dummy.cpp
)
add_library(robobus::bus ALIAS bus)

target_include_directories(bus PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/robobus/bus>
)

target_link_libraries(bus PUBLIC
    syoch-robotics::common
    syoch-robotics::logger
    syoch-robotics::rd16
    robobus::runtime
)

install(TARGETS bus
  EXPORT robobusTargets
  INCLUDES DESTINATION include/robobus/bus
)

install(DIRECTORY include/
  DESTINATION include/robobus/bus
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
