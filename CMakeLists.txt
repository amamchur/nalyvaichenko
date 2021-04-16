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

function(RAGEL_GENERATE_CPP RGL_SRC CPP_SRC)
    get_filename_component(ABS_FIL ${RGL_SRC} ABSOLUTE)
    get_filename_component(ABS_PATH ${RGL_SRC} PATH)
    get_filename_component(FILE_NAME ${RGL_SRC} NAME)
    set(ABS_CPP_SRC ${ABS_FIL}.cpp)

    add_custom_command(
            OUTPUT "${ABS_CPP_SRC}"
            COMMAND ragel
            ARGS -G1 -L -o "${ABS_CPP_SRC}" "${ABS_FIL}"
            DEPENDS "${ABS_FIL}"
            COMMENT "Running ragel file ${FILE_NAME}"
            VERBATIM)
    set_source_files_properties(${CPP_SRC} PROPERTIES GENERATED TRUE)
    set(${CPP_SRC} ${ABS_CPP_SRC} PARENT_SCOPE)
endfunction()

include_directories(include)

ragel_generate_cpp(src/cmd_line_parser.rl CMD_STATE_MACHINE_SRC)
ragel_generate_cpp(src/terminal_machine.rl TERMINAL_STATE_MACHINE_SRC)

add_mcu_executable(
        nalyvaichenko
        atmega328p
        src/main.cpp
        src/segment_detector.cpp
        src/terminal_input.cpp
        src/barman.cpp
        ${TERMINAL_STATE_MACHINE_SRC}
        ${CMD_STATE_MACHINE_SRC}
        )
target_compile_definitions(nalyvaichenko PRIVATE F_CPU=16000000UL)
