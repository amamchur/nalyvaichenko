//
// Created by andrii on 07.07.21.
//

#ifndef NALYVAICHENKO_CONFIG_HPP
#define NALYVAICHENKO_CONFIG_HPP

#include <zoal/board/arduino_mega.hpp>

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

struct settings_type {
    int total_segments_;
    int portion_delay_;
    int sector_a_hall_value;
    int sector_b_hall_value;
    int ir_max_value_;
    int ir_min_value_;
};

void load_settings(settings_type& settings);
void save_settings(settings_type& settings);

#endif //NALYVAICHENKO_CONFIG_HPP
