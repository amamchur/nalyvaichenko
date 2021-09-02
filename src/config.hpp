//
// Created by andrii on 07.07.21.
//

#ifndef NALYVAICHENKO_CONFIG_HPP
#define NALYVAICHENKO_CONFIG_HPP

#include "./volatile_data.hpp"

#include <zoal/board/arduino_mega.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

using pcb = zoal::board::arduino_mega;
using mcu = pcb::mcu;
using timer = mcu::timer_00;
using tty_usart = mcu::usart_00;
using df_player_usart = mcu::usart_01;
using adc = mcu::adc_00;
using i2c = mcu::i2c_00;

using encoder_pin_a = pcb::ard_d38;
using encoder_pin_b = pcb::ard_d40;
using encoder_pin_btn = pcb::ard_d42;
using encoder_pin_vcc = pcb::ard_d44;
using encoder_pin_gnd = pcb::ard_d46;

using hall_sensor = pcb::ard_a05;
using ir_sensor = pcb::ard_a04;
using pump_signal = pcb::ard_d48;
using valve_signal = pcb::ard_d49;

using counter = zoal::utils::ms_counter<decltype(milliseconds), &milliseconds>;
using tools = zoal::utils::tool_set<mcu, F_CPU, counter, void>;
using delay = tools::delay;
using overflow_to_tick = zoal::utils::timer_overflow_to_tick<F_CPU, 32, 256>;

struct settings_type {
    int total_segments_;
    int portion_time_;
    int portion_delay_;
    int sector_a_hall_value;
    int sector_b_hall_value;
    int ir_max_value_;
    int ir_min_value_;
};

void load_settings(settings_type &settings);
void save_settings(settings_type &settings);

#endif //NALYVAICHENKO_CONFIG_HPP
