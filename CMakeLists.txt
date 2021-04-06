cmake_minimum_required(VERSION 3.6)
project(nalyvaichenko CXX C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options("-Os")
endif ()

include_directories(/usr/local/include)

add_mcu_executable(
        nalyvaichenko
        atmega328p
        src/main.cpp
        src/segment_detector.cpp
        )
target_compile_definitions(nalyvaichenko PRIVATE F_CPU=16000000UL)
