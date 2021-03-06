#include "settings.hpp"

#include "config.hpp"

constexpr uint32_t magic_number = 0xDEADBEEF;
constexpr uint32_t settings_flash_address = 0;

const settings_type default_settings = {
    //
    magic_number,
    6,
    80,
    2500,
    3100,
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

void load_settings(settings_type &settings) {
    w25q32::fast_read(settings_flash_address, &settings, sizeof(settings_type));
    if (settings.magic_number_ != magic_number) {
        settings = default_settings;
    }
}

void save_settings(settings_type &settings) {
    settings.magic_number_ = magic_number;
    w25q32::sector_erase(w25q32::address_to_sector(settings_flash_address));
    w25q32::write(settings_flash_address, &settings, sizeof(settings_type));
}
