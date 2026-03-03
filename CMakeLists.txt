cmake_minimum_required(VERSION 3.25)
cmake_policy(VERSION 3.25)

set(ROBO_TOOLCHAIN_FILE ClangArmToolchain)
set(CMAKE_INSTALL_MESSAGE LAZY)

enable_testing()
project(robobus VERSION 1.0.0)
enable_language(C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-mfloat-abi=softfp -mfpu=fpv4-sp-d16 -mthumb -mcpu=cortex-m4)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -gfull -fstandalone-debug -gdwarf-5 -gz")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gfull -fstandalone-debug -gdwarf-5 -gz")

add_compile_options(-Os -ffunction-sections -fdata-sections)

find_package(Nano REQUIRED)
find_package(CMSIS5 REQUIRED)
find_package(syoch-robotics REQUIRED)

add_subdirectory(lib)

install(
  EXPORT robobusTargets
  FILE robobusTargets.cmake
  NAMESPACE robobus::
  DESTINATION lib/cmake/robobus
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/robobusConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/robobusConfig.cmake
  INSTALL_DESTINATION lib/cmake/robobus
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/robobusConfig.cmake
  DESTINATION lib/cmake/robobus
)
