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

using pump_pwm_timer = mcu::timer_03;
using hall_sensor = pcb::ard_a05;
using ir_sensor = pcb::ard_a04;
using pump_signal = pcb::ard_d05;
using valve_signal = pcb::ard_d03;
using pump_pwm_channel = mcu::mux::pwm_channel<pump_pwm_timer, pump_signal>;
using hall_channel = mcu::mux::adc_channel<adc, hall_sensor>;
using ir_channel = mcu::mux::adc_channel<adc, ir_sensor>;

using counter = zoal::utils::ms_counter<decltype(milliseconds), &milliseconds>;
using tools = zoal::utils::tool_set<mcu, F_CPU, counter, void>;
using delay = tools::delay;
using overflow_to_tick = zoal::utils::timer_overflow_to_tick<F_CPU, 32, 256>;

static constexpr size_t total_portions = 3;

struct revolver_settings {
    int portion_delay_;
    int ir_max_value_;
    int ir_min_value_;
    int sector_adjustment_;
};

struct portion_settings {
    uint8_t mg_;
    int time_;
};

struct settings_type {
    int segments_;
    int pump_power_;
    int hall_rising_threshold_;
    int hall_falling_threshold_;
    int current_portion_;
    portion_settings portion_settings_[total_portions];
    revolver_settings revolver_settings_[7];
};

void load_settings(settings_type &settings);
void save_settings(settings_type &settings);

#endif
