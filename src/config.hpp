#ifndef NALYVAICHENKO_CONFIG_HPP
#define NALYVAICHENKO_CONFIG_HPP

#include "./volatile_data.hpp"

#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

#if defined (__AVR_ATmega2560__)
#include "config_atmega2560.hpp"
#elif defined(STM32F401xC)
#include "config_stm32f401.hpp"
#endif

using pump_pwm_channel = mcu::mux::pwm_channel<pump_pwm_timer, pump_signal>;
using hall_channel = mcu::mux::adc_channel<sensor_adc, hall_sensor>;
using ir_channel = mcu::mux::adc_channel<sensor_adc, ir_sensor>;

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
