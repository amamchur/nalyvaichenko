#include "config.hpp"

#include <avr/eeprom.h>

settings_type eeprom_settings __attribute__((section(".eeprom"))) = {
    //
    6,
    80,
    850,
    600,
    {
        {0, 0, 10000, 0,0},
        {1300, 100, 50, 0, -10},
        {1300, 100, 50, 0, -10},
        {1300, 100, 50, 0, -10},
        {1300, 100, 50, 0, -10},
        {1300, 100, 50, 0, -10},
        {1300, 100, 50, 0, -10}
    }
};

void load_settings(settings_type &settings) {
    eeprom_read_block(&settings, &eeprom_settings, sizeof(settings_type));
}

void save_settings(settings_type &settings) {
    eeprom_write_block(&settings, &eeprom_settings, sizeof(settings_type));
}
