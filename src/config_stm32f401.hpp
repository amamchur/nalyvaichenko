#ifndef NALYVAICHENKO_CONFIG_STM32F401_HPP
#define NALYVAICHENKO_CONFIG_STM32F401_HPP

#include "stm32f4xx_hal.h"

#include <zoal/mcu/stm32f401ccux.hpp>
#include <zoal/utils/cmsis_os2/delay.hpp>

using counter = zoal::utils::ms_counter<uint32_t, &uwTick>;
using mcu = zoal::mcu::stm32f401ccux;
using delay = zoal::utils::cmsis_os2::delay<84000000>;
using tty_usart = mcu::usart_01;
using df_player_usart = mcu::usart_02;
using adc = mcu::adc_01;
using i2c = mcu::i2c_01;

using i2c_clk = mcu::pb_06;
using i2c_sda = mcu::pb_07;

using encoder_pin_a = mcu::pb_12;
using encoder_pin_b = mcu::pb_13;
using encoder_pin_btn = mcu::pa_08;

using hall_sensor = mcu::pa_00;
using ir_sensor = mcu::pa_01;
using start_signal = mcu::pc_14;
using stop_signal = mcu::pc_15;

using stepper_a = mcu::pb_03;
using stepper_b = mcu::pa_15;
using stepper_c = mcu::pa_12;
using stepper_d = mcu::pa_11;

using pump_pwm_timer = mcu::timer_03;
using sensor_adc = mcu::adc_01;

using pump_signal = mcu::pb_00;
using valve_signal = mcu::pb_05;

#endif
