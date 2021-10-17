#include "config.hpp"

settings_type eeprom_settings __attribute__((section(".eeprom"))) = {
    //
    6,
    80,
    850,
    600,
    0,
    {// portion_settings
     {25, 710},
     {50, 1400},
     {75, 2100}},
    {// revolver_settings
     {0, 10000, 0, 0},
     {100, 50, 0, -10},
     {100, 50, 0, -10},
     {100, 50, 0, -10},
     {100, 50, 0, -10},
     {100, 50, 0, -10},
     {100, 50, 0, -10}}};

#ifdef __AVR_ARCH__

#include <avr/eeprom.h>

void load_settings(settings_type &settings) {
    eeprom_read_block(&settings, &eeprom_settings, sizeof(settings_type));
}

void save_settings(settings_type &settings) {
    eeprom_write_block(&settings, &eeprom_settings, sizeof(settings_type));
}

#else

void load_settings(settings_type &settings) {
}

void save_settings(settings_type &settings) {
}

#endif