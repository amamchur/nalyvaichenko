cmake_minimum_required(VERSION 3.6)
project(nalyvaichenko CXX C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if (USE_QT)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif ()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options("-Os")
endif ()

include_directories(/usr/local/include $ENV{ZOAL_PATH})

function(RAGEL_GENERATE_CPP RGL_SRC CPP_SRC)
    get_filename_component(ABS_FIL ${RGL_SRC} ABSOLUTE)
    get_filename_component(ABS_PATH ${RGL_SRC} PATH)
    get_filename_component(FILE_NAME ${RGL_SRC} NAME)
    set(ABS_CPP_SRC ${ABS_FIL}.cpp)

    add_custom_command(
            OUTPUT "${ABS_CPP_SRC}"
            COMMAND ragel
            ARGS -G2 -L -o "${ABS_CPP_SRC}" "${ABS_FIL}"
            DEPENDS "${ABS_FIL}"
            COMMENT "Running ragel file ${FILE_NAME}"
            VERBATIM)
    set_source_files_properties(${CPP_SRC} PROPERTIES GENERATED TRUE)
    set(${CPP_SRC} ${ABS_CPP_SRC} PARENT_SCOPE)
endfunction()

include_directories(include)

ragel_generate_cpp(src/parsers/command_machine.rl COMMAND_STATE_MACHINE_SRC)
ragel_generate_cpp(src/parsers/terminal_machine.rl TERMINAL_STATE_MACHINE_SRC)

add_mcu_executable(
        nalyvaichenko
        atmega2560
        src/app_state.cpp
        src/config.cpp
        src/df_player.cpp
        src/fonts/roboto_regular_12.cpp
        src/fonts/roboto_regular_16.cpp
        src/fonts/roboto_regular_18.cpp
        src/gui.cpp
        src/hardware.cpp
        src/logo/ascii_logo.cpp
        src/logo/ecafe_logo.cpp
        src/main.cpp
        src/message.cpp
        src/sector_detector.cpp
        src/terminal_input.cpp
        src/tty_terminal.cpp
        src/volatile_data.cpp
        ${TERMINAL_STATE_MACHINE_SRC}
        ${COMMAND_STATE_MACHINE_SRC}
)

target_compile_definitions(nalyvaichenko PRIVATE F_CPU=16000000UL)

add_host_executable(emu
        src/emu/main.cpp
        src/emu/oledscreen.cpp
        src/emu/mainwindow.cpp
        src/emu/mainwindow.ui
        src/gui.cpp
        src/message.cpp
        src/config.cpp
        src/hardware.cpp
        src/app_state.cpp
        src/volatile_data.cpp
        src/df_player.cpp
        src/fonts/roboto_regular_12.cpp
        src/fonts/roboto_regular_16.cpp
        src/fonts/roboto_regular_18.cpp
        src/logo/ascii_logo.cpp
        src/logo/ecafe_logo.cpp
        )
target_include_directories(emu PRIVATE src/emu)
target_compile_definitions(emu PRIVATE F_CPU=16000000UL)