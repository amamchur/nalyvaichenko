#include "config.hpp"

#include <avr/eeprom.h>

settings_type eeprom_settings __attribute__((section(".eeprom"))) = {
    //
    6,
    850,
    750,
    500,
    300,
    0
};

void load_settings(settings_type &settings) {
    eeprom_read_block(&settings, &eeprom_settings, sizeof(settings_type));
    if (settings.total_segments_ > 0) {
        return;
    }

    settings.total_segments_ = 6;
    settings.portion_delay_ = 850;
    settings.sector_a_hall_value = 750;
    settings.sector_b_hall_value = 500;
    settings.ir_max_value_ = 300;
}

void save_settings(settings_type &settings) {
    eeprom_write_block(&settings, &eeprom_settings, sizeof(settings_type));
}
