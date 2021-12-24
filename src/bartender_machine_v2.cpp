#include "bartender_machine_v2.hpp"

#include "./app_state.hpp"
#include "./config.hpp"
#include "adc.h"

void bartender_machine_v2::update_period() {
    auto period = sk.period();
    if (isinf(period)) {
        hold();
    } else {
        motor_step_pwm_channel::set(static_cast<uint32_t>(period / 2));
        motor_pwm_timer::TIMERx_CNT::ref() = 0;
        motor_pwm_timer::TIMERx_ARR::ref() = static_cast<uint32_t>(period);
        motor_pwm_timer::enable();
    }
}

void bartender_machine_v2::rotate(double steps, double speed) {
    if (speed <= 0) {
        speed = speed_;
    }

    sk.setup(steps, acceleration_, speed);
    motor_en::low();
    motor_step_pwm_channel::connect();
    update_period();
    motor_pwm_timer::enable();
}

void bartender_machine_v2::stop() {
    motor_en::high();
    motor_pwm_timer::disable();
    motor_step_pwm_channel::disconnect();
    sk.reset();
    HAL_ADC_Stop_DMA(&hadc1);

    motor_pwm_timer::TIMERx_SR::ref() &= ~motor_pwm_timer::TIMERx_SR_UIF;
    state_ = state::idle;
    job_ = job::none;
    tasks_.clear();
}

void bartender_machine_v2::hold() {
    motor_en::low();
    motor_pwm_timer::disable();
    motor_step_pwm_channel::disconnect();
    motor_pwm_timer::TIMERx_SR::ref() &= ~motor_pwm_timer::TIMERx_SR_UIF;
    state_ = state::hold;
    job_ = job::none;
}

void bartender_machine_v2::main_task() {
    while (true) {
        delay::ms(1);
        //        auto e = events.wait(0xFF);
        //        if ((e & event_stop) == event_stop) {
        //            //                tty_stream;
        //        }
    }
}

void bartender_machine_v2::handle_timer() {
    bartender_machine_task *t;
    if (tasks_.front(&t)) {
        bool finished = t->handle_timer_();
        if (finished) {
            tasks_.pop_front();
            if (tasks_.front(&t)) {
                t->start_();
            }
        }
    }
}

void bartender_machine_v2::handle_adc() {
    bartender_machine_task *t;
    if (tasks_.front(&t)) {
        bool finished = t->handle_adc_();
        if (finished) {
            tasks_.pop_front();
            if (tasks_.front(&t)) {
                t->start_();
            }
        }
    }
}

void bartender_machine_v2::rpm(uint32_t value) {
    speed_ = static_cast<uint32_t>(value / 60.0 * steps_per_revolution);
}

void bartender_machine_v2::next_segment() {
    auto steps = static_cast<double>(steps_per_revolution) / segments_;
    rotate(steps);
}

void bartender_machine_v2::motor_test() {
    auto start = [this]() {
        auto steps = static_cast<double>(steps_per_revolution) * 50;
        rotate(steps);
    };
    auto timer = [this]() {
        sk.inc_step();
        update_period();
        return sk.current_step_ >= sk.target_step_;
    };
    auto adc = [this]() {
        return false;
    };
    bartender_machine_task t(start, timer, adc);
    push_task(t);
}

void bartender_machine_v2::push_find_segment() {
    auto start = [this]() {
        auto steps = static_cast<double>(steps_per_revolution) / segments_ * 2;
        rotate(steps, speed_);
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sensors_values, 2);
    };
    auto timer = [this]() {
        sk.inc_step();
        update_period();
        return false;
    };
    auto adc = [this]() {
        auto hall = sensors_values[0];
        auto result = detector_.handle_v2(hall);
//        const int max_v = 2600;
//        const int min_v = 2000;
//        auto rate = sqrt((double)(max_v - hall) / (max_v - min_v));
//        if (rate < 0.7) {
//            rate = 0.7;
//        }
//        sk.max_speed_ = speed_ * rate;

        if (result == detection_result::changed && detector_.sector_state_ == sector_state::sector) {
            hold();
            return true;
        } else {
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sensors_values, 2);
        }
        return false;
    };
    bartender_machine_task t(start, timer, adc);
    push_task(t);
}

void bartender_machine_v2::process_event(event &e) {
    //    auto &s = global_app_state.settings;
    //    auto segments = s.segments_;
    //    auto power = s.pump_power_;
    //    auto rs = s.revolver_settings_[segments];
    //    auto ps = s.portion_settings_[s.current_portion_];
    //    switch (e.type) {
    //    case event_type::settings_changed:
    //        detector_.rising_threshold = global_app_state.settings.hall_rising_threshold_;
    //        detector_.falling_threshold = global_app_state.settings.hall_falling_threshold_;
    //        break;
    //    default:
    //        break;
    //    }
}

void bartender_machine_v2::push_task(bartender_machine_task &task) {
    bool start_now = tasks_.empty();
    tasks_.push_back(task);
    if (start_now) {
        task.start_();
    }
}

void bartender_machine_v2::go() {
    push_find_segment();

    auto start = [this]() {
        auto steps = static_cast<double>(steps_per_revolution) / segments_;
        rotate(steps, speed_);
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sensors_values, 2);
    };
    auto timer = [this]() {
        sk.inc_step();
        update_period();
        return sk.current_step_ >= sk.target_step_;
    };
    auto adc = [this]() {
//        auto hall = sensors_values[0];
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sensors_values, 2);
        return false;
    };

    for (int i = 0; i < segments_; i++) {
        bartender_machine_task t(start, timer, adc);
        push_task(t);
    }
}
