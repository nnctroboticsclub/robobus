add_library(robobus-bus STATIC
    include/robobus/network/interface/dummy.cpp
)

target_include_directories(robobus-bus PUBLIC
    include
)

target_link_libraries(robobus-bus PUBLIC
    syoch-robotics::common
    syoch-robotics::logger
    syoch-robotics::rd16
    robobus-runtime
)
