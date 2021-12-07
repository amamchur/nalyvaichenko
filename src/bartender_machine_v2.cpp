#include "bartender_machine_v2.hpp"

#include "./config.hpp"

void bartender_machine_v2::update_period() {
    auto period = sk.period();
    if (isinf(period)) {
        stop();
    } else {
        motor_step_pwm_channel::set(static_cast<uint32_t>(period / 2));
        motor_pwm_timer::TIMERx_CNT::ref() = 0;
        motor_pwm_timer::TIMERx_ARR::ref() = static_cast<uint32_t>(period);
        motor_pwm_timer::enable();
    }
}

void bartender_machine_v2::start() {
    sk.setup(32 * 200 * 2, 2000, 32 * 200);
    motor_en::low();
    motor_step_pwm_channel::connect();
    update_period();
    motor_pwm_timer::enable();
}

void bartender_machine_v2::stop() {
    motor_en::high();
    motor_pwm_timer::disable();
    motor_step_pwm_channel::disconnect();
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
    sk.inc_step();
    update_period();
}

void bartender_machine_v2::handle_adc() {}
