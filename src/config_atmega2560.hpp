#ifndef NALYVAICHENKO_CONFIG_ATMEGA2560_HPP
#define NALYVAICHENKO_CONFIG_ATMEGA2560_HPP

#include <zoal/board/arduino_mega.hpp>

using pcb = zoal::board::arduino_mega;
using mcu = pcb::mcu;
using timer = mcu::timer_00;
using tty_usart = mcu::usart_00;
using df_player_usart = mcu::usart_01;
using adc = mcu::adc_00;
using i2c = mcu::i2c_00;

using i2c_clk = pcb::ard_d21;
using i2c_sda = pcb::ard_d20;

using encoder_pin_a = pcb::ard_d38;
using encoder_pin_b = pcb::ard_d40;
using encoder_pin_btn = pcb::ard_d42;
using encoder_pin_vcc = pcb::ard_d44;
using encoder_pin_gnd = pcb::ard_d46;
using hall_sensor = pcb::ard_a05;
using ir_sensor = pcb::ard_a04;
using pump_signal = pcb::ard_d05;
using valve_signal = pcb::ard_d03;
using start_signal = pcb::ard_d13;
using stop_signal = pcb::ard_d11;
using stepper_a = pcb::ard_d31;
using stepper_b = pcb::ard_d29;
using stepper_c = pcb::ard_d27;
using stepper_d = pcb::ard_d25;

using tty_usart_rx = pcb::ard_d00;
using tty_usart_tx = pcb::ard_d01;
using df_player_usart_rx = pcb::ard_d19;
using df_player_usart_tx = pcb::ard_d18;

using pump_pwm_timer = mcu::timer_03;
using sensor_adc = mcu::adc_00;

using counter = zoal::utils::ms_counter<decltype(milliseconds), &milliseconds>;
using tools = zoal::utils::tool_set<mcu, F_CPU, counter, void>;
using delay = tools::delay;
using overflow_to_tick = zoal::utils::timer_overflow_to_tick<F_CPU, 32, 256>;

#endif
