set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_COMPILER_WORKS 1)

set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(ARM_OBJCOPY arm-none-eabi-objcopy)
set(ARM_OBJDUMP arm-none-eabi-objdump)
set(ARM_SIZE arm-none-eabi-size)

set(COMMON_FLAGS "-specs=nosys.specs -specs=nano.specs")
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS}")
set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS}")

function(add_zoal_tests)
    add_executable(zoal_tests apps/_empty.cpp)
endfunction(add_zoal_tests)

function(add_host_executable NAME)
    add_executable(${NAME} src/_empty.cpp)
endfunction(add_host_executable)

function(add_mcu_executable NAME MCU)
    if (${MCU} MATCHES "^[sS][tT][mM]32")
        string(TOUPPER ${NAME} UPPER_NAME)

        set(ARM_SRC "${${UPPER_NAME}_SRC}")
        set(ARM_INC "${${UPPER_NAME}_INC}")
        set(ARM_DEFS "${${UPPER_NAME}_DEFS}")
        set(ARM_FLASH_LD "${${UPPER_NAME}_FLASH_LD}")
        set(ARM_OPT_MCU "${${UPPER_NAME}_OPT_MCU}")

        set(ARM_COMMON_FLAGS ${ARM_OPT_MCU}
                -mthumb-interwork
                -fdata-sections
                -ffunction-sections
                -fmessage-length=0
                -fno-exceptions
                -g
                )

        add_executable(${NAME} ${ARM_SRC} ${ARGN})
        target_include_directories(${NAME} PUBLIC ${ARM_INC})
        target_compile_definitions(${NAME} PUBLIC ${ARM_DEFS})
        target_compile_options(${NAME} PUBLIC ${ARM_COMMON_FLAGS})

        message(STATUS ${ARM_DEFS})

        get_filename_component(ARM_FLASH_LD_ABS ${ARM_FLASH_LD} ABSOLUTE)
        target_link_options(${NAME} PUBLIC
                -T${ARM_FLASH_LD_ABS}
                -Wl,-Map,${NAME}.map
                -Wl,--gc-sections,--print-memory-usage
                ${ARM_COMMON_FLAGS})

        set(LST_FILE ${NAME}.lst)
        add_custom_command(
                TARGET ${NAME}
                POST_BUILD
                COMMAND ${ARM_OBJDUMP} -S ${NAME} > ${LST_FILE})
        set_property(
                DIRECTORY
                APPEND
                PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                "${LST_FILE}"
                "${NAME}.map")
    else ()
        add_executable(${NAME} src/_empty.cpp)
    endif ()
endfunction(add_mcu_executable)
