#include "bartender_machine.hpp"

#include "./app_state.hpp"
#include "./config.hpp"
#include "./storage/anim.hpp"
#include "./storage/voice.hpp"
#include "adc.h"

constexpr uint8_t task_state_default = 0;
constexpr uint8_t task_state_find_segment_correction = 1;
constexpr uint8_t task_state_portion_rotate = 1;
constexpr uint8_t task_state_portion_check = 2;
constexpr uint8_t task_state_portion_make = 3;

bool bartender_machine::update_period() {
    auto period = sk.period();
    if (isinf(period)) {
        hold();
        return false;
    } else {
        motor_step_pwm_channel::set(static_cast<uint32_t>(period / 2));
        machine_timer::TIMERx_ARR::ref() = static_cast<uint32_t>(period);
        machine_timer::TIMERx_CNT::ref() = 0;
        machine_timer::enable();
        return true;
    }
}

void bartender_machine::absolute_rotate(float steps, float speed) {
    if (speed <= 0) {
        speed = speed_;
    }

    sk.absolute(steps, acceleration_, speed);
    motor_enable::on();
    motor_step_pwm_channel::connect();
    update_period();
    machine_timer::enable();
}

void bartender_machine::relative_rotate(float steps, float speed) {
    if (speed <= 0) {
        speed = speed_;
    }

    sk.relative(steps, acceleration_, speed);
    motor_enable::on();
    motor_step_pwm_channel::connect();
    update_period();
    machine_timer::enable();
}

void bartender_machine::stop_machine() {
    motor_enable::off();
    machine_timer::disable();
    motor_step_pwm_channel::disconnect();

    pump_pwm_timer::disable();
    pump_pwm_channel::disconnect();
    pump_signal::low();
    valve_signal::low();

    sk.reset();
    HAL_ADC_Stop_DMA(&hadc1);

    machine_timer::TIMERx_SR::ref() &= ~machine_timer::TIMERx_SR_UIF;
    tasks_.clear();
}

void bartender_machine::hold() {
    motor_enable::on();
    machine_timer::disable();
    motor_step_pwm_channel::disconnect();
    machine_timer::TIMERx_SR::ref() &= ~machine_timer::TIMERx_SR_UIF;
}

void bartender_machine::main_task() {
    while (true) {
        delay::ms(1);
        //        auto e = events.wait(0xFF);
        //        if ((e & event_stop) == event_stop) {
        //            //                tty_stream;
        //        }
    }
}

void bartender_machine::handle_timer() {
    bartender_machine_task *t;
    if (!tasks_.front(&t)) {
        return;
    }

    bool finished = t->handle_timer_(*t);
    if (!finished) {
        return;
    }

    while (finished) {
        tasks_.pop_front();
        if (!tasks_.front(&t)) {
            return;
        }
        finished = t->start_(*t);
    }
}

void bartender_machine::handle_adc() {
    bartender_machine_task *t;
    if (!tasks_.front(&t)) {
        return;
    }

    bool finished = t->handle_adc_(*t);
    if (!finished) {
        return;
    }

    while (finished) {
        tasks_.pop_front();
        if (!tasks_.front(&t)) {
            return;
        }
        finished = t->start_(*t);
    }
}

void bartender_machine::rpm(uint32_t value) {
    speed_ = static_cast<float>(value) / 60.0f * steps_per_revolution;
}

void bartender_machine::acceleration(float value) {
    acceleration_ = value;
}

void bartender_machine::next_segment() {
    push_find_segment();
}

void bartender_machine::rotate(float steps) {
    auto start = [this, steps](bartender_machine_task &) {
        absolute_rotate(steps);
        return false;
    };
    auto timer = [this](bartender_machine_task &) {
        sk.inc_step();
        auto updated = update_period();
        if (!update_period()) {
            stop_machine();
        }
        return !updated;
    };
    auto adc = [](bartender_machine_task &) { return false; };
    bartender_machine_task t(start, timer, adc);
    push_task(t);
}

void bartender_machine::push_find_segment() {
    auto start = [this](bartender_machine_task &) {
        auto steps = static_cast<float>(steps_per_revolution) / static_cast<float>(segments_) * 2.0f;
        motor_dir_cw::on();
        absolute_rotate(steps, speed_);
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sensors_values, 2);
        return false;
    };
    auto timer = [this](bartender_machine_task &t) {
        sk.inc_step();
        switch (t.state) {
        case task_state_default: {
            auto updated = update_period();
            if (!updated) {
                tasks_.clear();
                return true;
            }
            break;
        }
        case task_state_find_segment_correction: {
            auto updated = update_period();
            if (!updated) {
                hold();
                sk.reset();
                return true;
            }
            break;
        }
        default:
            break;
        }
        if (t.state == task_state_default) {
            auto updated = update_period();
            if (!updated) {
                tasks_.clear();
                return true;
            }
        }
        return false;
    };
    auto adc = [this](bartender_machine_task &t) {
        auto hall = hall_sensor_value();
        auto result = detector_.handle(hall);
        if (result == detection_result::changed && detector_.sector_state_ == sector_state::sector) {
            auto steps_left = sk.target_step_ - sk.current_step_;
            auto steps = static_cast<float>(steps_per_revolution) / static_cast<float>(segments_);
            steps = steps - steps_left + static_cast<float>(correction_);
            relative_rotate(steps, speed_);
            t.state = task_state_find_segment_correction;
        } else {
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sensors_values, 2);
        }
        return false;
    };
    bartender_machine_task t(start, timer, adc);
    push_task(t);
}

void bartender_machine::push_release() {
    auto start = [this](bartender_machine_task &) {
        motor_enable::off();
        machine_timer::disable();
        motor_step_pwm_channel::disconnect();
        sk.reset();
        HAL_ADC_Stop_DMA(&hadc1);
        machine_timer::TIMERx_SR::ref() &= ~machine_timer::TIMERx_SR_UIF;
        return true;
    };

    bartender_machine_task t(start, null_task_handler, null_task_handler);
    push_task(t);
}

void bartender_machine::process_event(event &e) {
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

void bartender_machine::push_task(bartender_machine_task &task) {
    bool start_now = tasks_.empty();
    tasks_.push_back(task);
    if (start_now) {
        task.start_(task);
    }
}

void bartender_machine::notify_done() {
    bartender_machine_task t([this](bartender_machine_task &t) {
        if (drinks_made_ != 0) {
            send_command_isr(command_type::ui_anim, anim::drink + rand() % 8);
            send_command_isr(command_type::play, voice::cheers);
        } else {
            send_command_isr(command_type::play, voice::no_drink);
        }
        return true;
    }, null_task_handler, null_task_handler);
    push_task(t);
}

void bartender_machine::go() {
    stop_machine();

    drinks_made_ = 0;

    push_find_segment();

    auto start = [this](bartender_machine_task &t) {
        auto steps = static_cast<float>(steps_per_revolution) / static_cast<float>(segments_);
        motor_dir_cw::on();
        t.state = task_state_portion_rotate;
        relative_rotate(steps, speed_);
        return false;
    };
    auto timer = [this](bartender_machine_task &t) {
        switch (t.state) {
        case task_state_portion_rotate: {
            sk.inc_step();
            auto updated = update_period();
            if (!updated) {
                hold();
                t.state = task_state_portion_check;
                HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sensors_values, 2);
            }
            break;
        }
        case task_state_portion_make:
            machine_timer::disable();
            pump_pwm_timer::disable();
            pump_pwm_channel::disconnect();
            pump_signal::low();
            valve_signal::low();

            send_command_isr(command_type::play, voice::drink_ready + drinks_made_);
            drinks_made_++;
            return true;
        }
        return false;
    };
    auto adc = [this](bartender_machine_task &t) {
        if (t.state == task_state_portion_check) {
            auto ir = ir_sensor_value();
            auto hall = hall_sensor_value();
            if (ir > ir_value_ || hall < hall_value_) {
                return true;
            }
            motor_enable::off();

            t.state = task_state_portion_make;
            machine_timer::TIMERx_ARR::ref() = portion_time_ms_ * 1000; // us
            machine_timer::TIMERx_CNT::ref() = 0;
            machine_timer::enable();

            pump_pwm_channel::connect();
            pump_pwm_channel::set(950);
            pump_pwm_timer::enable();
            valve_signal::high();

            return false;
        }
        return true;
    };

    for (int i = 0; i < segments_; i++) {
        bartender_machine_task t(start, timer, adc);
        t.portion = i;
        push_task(t);
    }

    push_release();
    notify_done();
}

bool bartender_machine::null_task_handler(bartender_machine_task &) {
    return true;
}

void bartender_machine::pump(uint32_t delay_ticks) {
    auto start = [delay_ticks](bartender_machine_task &t) {
        motor_enable::off();
        motor_step_pwm_channel::disconnect();
        machine_timer::disable();
        machine_timer::TIMERx_ARR::ref() = delay_ticks * 1000;
        machine_timer::TIMERx_CNT::ref() = 0;
        machine_timer::enable();

        pump_pwm_channel::connect();
        pump_pwm_channel::set(950);
        pump_pwm_timer::enable();
        valve_signal::high();
        return false;
    };
    auto timer = [](bartender_machine_task &t) {
        machine_timer::disable();
        pump_pwm_timer::disable();
        pump_pwm_channel::disconnect();
        pump_signal::low();
        valve_signal::low();
        return true;
    };

    bartender_machine_task t(start, timer, null_task_handler);
    push_task(t);
}

void bartender_machine::valve(int delay_ticks) {
    auto start = [delay_ticks](bartender_machine_task &t) {
        motor_enable::off();
        motor_step_pwm_channel::disconnect();
        machine_timer::disable();
        machine_timer::TIMERx_ARR::ref() = delay_ticks * 1000;
        machine_timer::TIMERx_CNT::ref() = 0;
        machine_timer::enable();

        valve_signal::high();
        return false;
    };
    auto timer = [](bartender_machine_task &t) {
        machine_timer::enable();
        pump_pwm_channel::disconnect();
        pump_signal::low();
        valve_signal::low();
        return true;
    };

    bartender_machine_task t(start, timer, null_task_handler);
    push_task(t);
}
