#include "config.hpp"

#include <avr/eeprom.h>

settings_type eeprom_settings __attribute__((section(".eeprom"))) = {
    //
    6,
    80,
    850,
    600,
    0,
    {// portion_settings
     {11, 101},
     {21, 102},
     {31, 103},
     {41, 104},
     {51, 105}},
    {// revolver_settings
     {0, 0, 10000, 0, 0},
     {800, 100, 50, 0, -10},
     {800, 100, 50, 0, -10},
     {800, 100, 50, 0, -10},
     {800, 100, 50, 0, -10},
     {800, 100, 50, 0, -10},
     {800, 100, 50, 0, -10}}};

void load_settings(settings_type &settings) {
    eeprom_read_block(&settings, &eeprom_settings, sizeof(settings_type));
}

void save_settings(settings_type &settings) {
    eeprom_write_block(&settings, &eeprom_settings, sizeof(settings_type));
}
