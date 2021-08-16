#ifndef NALYVAICHENKO_APP_STATE_HPP
#define NALYVAICHENKO_APP_STATE_HPP

#include "config.hpp"

#include <stdint.h>
#include <zoal/func/function.hpp>

typedef enum {
    app_state_flags_idle = 0,
    app_state_flags_calibration = 1 << 0,
    app_state_flags_error = 1 << 1,
} app_state_flags;

class app_state {
public:
    settings_type settings;

    int max_hall_value_;
    int min_hall_value_;

    void load_settings();
    void save_settings();
};

extern app_state global_app_state;

#endif //NALYVAICHENKO_APP_STATE_HPP
