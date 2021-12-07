#ifndef NALYVAICHENKO_STEPPER_KINEMATICS_HPP
#define NALYVAICHENKO_STEPPER_KINEMATICS_HPP

#include <cmath>
#include <cstdint>

template<class StepType = uint32_t>
class stepper_kinematics {
public:
    StepType target_step_{0};
    StepType current_step_{0};
    double max_speed_{1}; // steps/sec
    double current_speed{0}; // steps/sec
    double acceleration_{1}; // steps/sec^2
    double period_{INFINITY}; // microseconds

    void setup(StepType target, double accel, double max_speed, double start_speed = 0) {
        target_step_ = target;
        current_step_ = 0;
        acceleration_ = accel;
        max_speed_ = max_speed;
        current_speed = start_speed;
        calculate_period();
    }

    double speed() const {
        return current_speed;
    }

    double period() const {
        return period_;
    }

    void inc_step() {
        current_step_++;
        calculate_period();
    }

    void calculate_period() {
        if (current_step_ >= target_step_) {
            period_ = INFINITY;
            current_speed = 0;
            return;
        }

        auto steps_left = target_step_ - current_step_;
        auto speed = sqrt(2.0 * steps_left * acceleration_);
        if (speed > current_speed) {
            if (current_speed == 0) {
                speed = sqrt(2.0 * acceleration_);
            } else {
                speed = current_speed + abs(acceleration_ / current_speed);
            }
            if (speed > max_speed_) {
                speed = max_speed_;
            }
        }

        current_speed = speed;
        period_ = 1000000 / speed;
    }
};

#endif
