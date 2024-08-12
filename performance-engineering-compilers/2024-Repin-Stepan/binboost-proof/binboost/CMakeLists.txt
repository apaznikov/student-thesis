cmake_minimum_required(VERSION 3.25.0) # Debian 12

project(binboost C CXX ASM)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# @todo automatically detect
set(BB_ARCH "amd64" CACHE STRING "Target CPU architecture")
option(BB_DISABLE_JIT "Disable using of bibboost" OFF)
option(BB_ENABLE_TEST "Build tests" ON)
option(BB_EXCLUDE_TRAMPOLINE "Exclude ASM file from build" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

if(BB_DISABLE_JIT)
    add_compile_definitions(BB_DISABLED)
endif()

if(BB_EXCLUDE_TRAMPOLINE)
    add_compile_definitions(BB_EXCLUDE_TRAMPOLINE)
endif()

add_subdirectory(src)

if(BB_ENABLE_TEST)
    message(STATUS "Tests enabled")
    add_subdirectory(test/)
endif()
