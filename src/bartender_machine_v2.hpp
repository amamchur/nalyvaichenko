#ifndef NALYVAICHENKO_BARTENDER_MACHINE_V2_HPP
#define NALYVAICHENKO_BARTENDER_MACHINE_V2_HPP

#include "./sector_detector.hpp"
#include "./stepper_kinematics.hpp"

class bartender_machine_v2 {
public:
    enum class state {
        idle,

    };
    state state_;
    stepper_kinematics<> sk;
    sector_detector detector_;

    void update_period();
    void start();
    void stop();
    [[noreturn]] void main_task();
    void handle_timer();
    void handle_adc();
};

#endif
