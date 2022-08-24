cmake_minimum_required(VERSION 3.10)
project(nalyvaichenko CXX C ASM)

cmake_policy(SET CMP0071 NEW)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if (USE_QT)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    find_package(GIF REQUIRED)
    find_package(PNG REQUIRED)
    find_package(Qt6 COMPONENTS Widgets REQUIRED)
    find_package(Boost 1.67 COMPONENTS log_setup log program_options filesystem thread regex json REQUIRED)
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
            ARGS -G0 -L -o "${ABS_CPP_SRC}" "${ABS_FIL}"
            DEPENDS "${ABS_FIL}"
            COMMENT "Running ragel file ${FILE_NAME}"
            VERBATIM)
    set_source_files_properties(${CPP_SRC} PROPERTIES GENERATED TRUE)
    set(${CPP_SRC} ${ABS_CPP_SRC} PARENT_SCOPE)
endfunction()

include_directories(include)

ragel_generate_cpp(src/parsers/command_machine.rl COMMAND_STATE_MACHINE_SRC)
ragel_generate_cpp(src/parsers/terminal_machine.rl TERMINAL_STATE_MACHINE_SRC)
ragel_generate_cpp(src/parsers/flash_machine.rl FLASH_STATE_MACHINE_SRC)
ragel_generate_cpp(src/parsers/df_player_machine.rl DF_PLAYER_STATE_MACHINE_SRC)

add_host_executable(test
        src/emu/test.cpp
        ${DF_PLAYER_STATE_MACHINE_SRC}
        )

add_host_executable(flash
        src/emu/flash.cpp
        ${FLASH_STATE_MACHINE_SRC}
        ${COMMAND_STATE_MACHINE_SRC}
        )

add_host_executable(emu
        src/emu/main.cpp
        src/emu/oledscreen.cpp
        src/emu/mainwindow.cpp
        src/emu/mainwindow.ui
        src/gui.cpp
        src/message.cpp
        src/config.cpp
        src/settings.cpp
        src/app_state.cpp
        src/event_manager.cpp
        src/flash_manager.cpp
        src/fonts/roboto_regular_12.cpp
        src/fonts/roboto_regular_16.cpp
        src/fonts/roboto_regular_18.cpp
        src/fonts/pixel_16.cpp
        src/logo/ascii_logo.cpp
        src/hardware_host.cpp
        ${FLASH_STATE_MACHINE_SRC}
        )
target_include_directories(emu PRIVATE src/emu)
target_compile_definitions(emu PRIVATE F_CPU=16000000UL)

## --- STM32 Projects ---
# Begin cubemx-proj variables

# STM32CubeMX project variables for cubemx-proj/stm32f401
set(STM32F401_SRC 
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/list.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/queue.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/timers.c
        cubemx-proj/stm32f401/Core/Src/adc.c
        cubemx-proj/stm32f401/Core/Src/dma.c
        cubemx-proj/stm32f401/Core/Src/freertos.c
        cubemx-proj/stm32f401/Core/Src/gpio.c
        cubemx-proj/stm32f401/Core/Src/i2c.c
        cubemx-proj/stm32f401/Core/Src/main.c
        cubemx-proj/stm32f401/Core/Src/spi.c
        cubemx-proj/stm32f401/Core/Src/stm32f4xx_hal_msp.c
        cubemx-proj/stm32f401/Core/Src/stm32f4xx_hal_timebase_tim.c
        cubemx-proj/stm32f401/Core/Src/stm32f4xx_it.c
        cubemx-proj/stm32f401/Core/Src/tim.c
        cubemx-proj/stm32f401/Core/Src/usart.c
        cubemx-proj/stm32f401/Core/Src/system_stm32f4xx.c
        cubemx-proj/stm32f401/startup_stm32f401xc.s)
set(STM32F401_INC 
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/CMSIS/Include
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Inc
        cube-repo/STM32Cube_FW_F4_V1.27.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/include
        cube-repo/STM32Cube_FW_F4_V1.27.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
        cubemx-proj/stm32f401/Core/Inc)
set(STM32F401_DEFS -DUSE_HAL_DRIVER -DSTM32F401xC)
set(STM32F401_OPT_MCU -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(STM32F401_FLASH_LD cubemx-proj/stm32f401/STM32F401CCUx_FLASH.ld)
# End cubemx-proj variables

add_mcu_executable(stm32f401 stm32
        src/main.cpp
        src/terminal_input.cpp
        src/tty_terminal.cpp
        src/message.cpp
        src/logo/ascii_logo.cpp
        src/event_manager.cpp
        src/hardware_stm32f401.cpp
        src/app_state.cpp
        src/df_player.cpp
        src/message_processor.cpp
        src/sector_detector.cpp
        src/settings.cpp
        src/gui.cpp
        src/fonts/roboto_regular_16.cpp
        src/fonts/pixel_16.cpp
        src/fonts/pixel_20.cpp
        src/config.cpp
        src/flash_manager.cpp
        src/bartender_machine.cpp
        ${TERMINAL_STATE_MACHINE_SRC}
        ${COMMAND_STATE_MACHINE_SRC}
        ${FLASH_STATE_MACHINE_SRC}
        ${DF_PLAYER_STATE_MACHINE_SRC}
        )
