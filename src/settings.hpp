#ifndef NALYVAICHENKO_SETTINGS_HPP
#define NALYVAICHENKO_SETTINGS_HPP

#include <cstddef>
#include <cstdint>

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
    uint32_t magic_number;
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

