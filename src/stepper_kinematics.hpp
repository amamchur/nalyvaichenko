#ifndef NALYVAICHENKO_STEPPER_KINEMATICS_HPP
#define NALYVAICHENKO_STEPPER_KINEMATICS_HPP

#include <cmath>
#include <cstdint>

template<uint32_t TimeExponent = 1000000>
class stepper_kinematics {
public:
    float target_step_{0};
    float current_step_{0};
    float max_speed_{1}; // steps/sec
    float current_speed_{0}; // steps/sec
    float acceleration_{1}; // steps/sec^2
    float period_{INFINITY}; // seconds / TimeExponent

    void absolute(float target, float accel, float max_speed, float start_speed = 0) {
        target_step_ = target;
        current_step_ = 0;
        acceleration_ = accel;
        max_speed_ = max_speed;
        current_speed_ = start_speed;
        calculate_period();
    }

    void relative(float steps, float accel, float max_speed) {
        target_step_ += steps;
        acceleration_ = accel;
        max_speed_ = max_speed;
        calculate_period();
    }

    float speed() const {
        return current_speed_;
    }

    float period() const {
        return period_;
    }

    void inc_step() {
        if (current_step_ < target_step_) {
            current_step_++;
            calculate_period();
        }
    }

    void calculate_period() {
        if (current_step_ >= target_step_) {
            period_ = INFINITY;
            current_speed_ = 0;
            return;
        }

        auto steps_left = target_step_ - current_step_;
        auto speed = sqrtf(2.0f * steps_left * acceleration_);
        if (speed > current_speed_) {
            if (current_speed_ == 0) {
                speed = sqrtf(2.0f * acceleration_);
            } else {
                speed = acceleration_ * period_ / TimeExponent + current_speed_;
            }
            if (speed > max_speed_) {
                speed = max_speed_;
            }
        }
        if (speed < 1) {
            speed = 0;
        }

        current_speed_ = speed;
        period_ = TimeExponent / speed;
    }

    void reset() {
        target_step_ = 0;
        current_speed_ = 0;
        current_step_ = 0;
        period_ = INFINITY;
    }
};

#endif
