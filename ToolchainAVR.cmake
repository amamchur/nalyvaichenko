set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_C_COMPILER_WORKS 1)

set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_CXX_COMPILER_WORKS 1)

find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_OBJDUMP avr-objdump)
find_program(AVR_SIZE avr-size)

set(avrdude_device_atmega328p m328p)
set(avrdude_device_atmega32u4 m32u4)
set(avrdude_device_atmega2560 m2560)
set(avrdude_device_attiny13 t13)
set(avrdude_device_attiny85 t85)
set(avrdude_device_attiny2313 t2313)

function(add_avr_executable NAME MCU)
    if (NOT ARGN)
        message(FATAL_ERROR "No source files given for ${NAME}.")
    endif (NOT ARGN)

    set(bin_file ${NAME}.bin)
    set(map_file ${NAME}.map)
    set(lst_file ${NAME}.lst)
    set(eep_file ${NAME}.eep)

    add_executable(${NAME} ${ARGN})

    set_target_properties(
            ${NAME}
            PROPERTIES
            LINK_FLAGS "-mmcu=${MCU} -Wl,--gc-sections -mrelax -Wl,-Map,${map_file}"
    )

    target_compile_options(
            ${NAME}
            PRIVATE
            -mmcu=${MCU}
            -Wall
            -Wno-unknown-pragmas
            -fno-exceptions
            -ffunction-sections
    )

    add_custom_command(
            TARGET ${NAME}
            POST_BUILD
            COMMAND ${AVR_OBJCOPY} -j .text -j .data -O binary ${NAME} ${bin_file}
            COMMAND ${AVR_OBJDUMP} -S ${NAME} > ${lst_file}
            COMMAND ${AVR_OBJCOPY}
            --only-section .eeprom
            --set-section-flags=.eeprom=alloc,load
            --change-section-lma .eeprom=0
            --no-change-warnings
            --output-target binary
            ${NAME} ${eep_file}
            COMMAND ${AVR_SIZE} --mcu=${MCU} --format=avr ${NAME}
            COMMAND ${AVR_SIZE} ${NAME}
    )

    set(DEVICE_KEY "avrdude_device_${MCU}")
    set(AVRDUDE_DEVICE ${${DEVICE_KEY}})
    add_custom_target(${NAME}-upload
            COMMAND avrdude -V -p ${AVRDUDE_DEVICE} -c usbasp -Uflash:w:${NAME}:e
            DEPENDS ${NAME}
            VERBATIM
            )
    add_custom_target(${NAME}-eeprom
            COMMAND avrdude -p ${AVRDUDE_DEVICE} -c usbasp -Ueeprom:w:${NAME}:e
            DEPENDS ${NAME}
            VERBATIM
            )
    add_custom_target(${NAME}-fuse
            COMMAND avrdude -p ${AVRDUDE_DEVICE} -c usbasp -Uefuse:w:${NAME}:e -Uhfuse:w:${NAME}:e -Ulfuse:w:${NAME}:e
            DEPENDS ${NAME}
            VERBATIM
            )

    set_property(
            DIRECTORY
            APPEND
            PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
            "${eep_file}"
            "${map_file}"
            "${bin_file}"
            "${lst_file}"
    )
endfunction(add_avr_executable)

function(add_mcu_executable NAME MCU)
    if (${MCU} MATCHES "(atmega)|(attiny)")
        add_avr_executable(${NAME} ${MCU} ${ARGN})
    else ()
        add_executable(${NAME} apps/_empty.cpp)
    endif ()
endfunction(add_mcu_executable)

function(add_zoal_tests)
    add_executable(zoal_tests apps/_empty.cpp)
endfunction(add_zoal_tests)

function(add_host_executable NAME)
    add_executable(${NAME} apps/_empty.cpp)
endfunction(add_host_executable)

function(add_cubemx_project NAME MCU)
    add_executable(${NAME} apps/_empty.cpp)
endfunction(add_cubemx_project)

function(test_fn NAME)
    add_executable(${NAME} apps/_empty.cpp)
endfunction(test_fn)
