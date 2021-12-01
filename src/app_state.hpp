#ifndef NALYVAICHENKO_APP_STATE_HPP
#define NALYVAICHENKO_APP_STATE_HPP

#include "settings.hpp"

#include <stdint.h>
#include <zoal/func/function.hpp>

class app_state {
public:
    bool flash_editor{false};
    settings_type settings;

    int max_hall_value_{0};
    int min_hall_value_{0};

    void load_settings();
    void save_settings();
};

extern app_state global_app_state;

#endif //NALYVAICHENKO_APP_STATE_HPP
