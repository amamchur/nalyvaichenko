#ifndef NALYVAICHENKO_APP_STATE_HPP
#define NALYVAICHENKO_APP_STATE_HPP

#include <stdint.h>
#include <zoal/func/function.hpp>

typedef enum {
    app_state_flags_idle = 0,
    app_state_flags_calibration = 1 << 0,
    app_state_flags_error = 1 << 1,
} app_state_flags;

class app_state {
public:
    volatile uint32_t flags{app_state_flags_idle};
    zoal::func::function<32, float> progress_fn;
    int total_segments;
};

extern app_state global_app_state;

#endif //NALYVAICHENKO_APP_STATE_HPP
