//
// Created by andrii on 03.07.21.
//

#ifndef NALYVAICHENKO_BARTENDER_MACHINE_HPP
#define NALYVAICHENKO_BARTENDER_MACHINE_HPP

#include "./app_state.hpp"
#include "./command.hpp"
#include "./segment_detector.hpp"
#include "./tty_terminal.hpp"

#include <stdint.h>
#include <zoal/gpio/pin_mode.hpp>
#include <zoal/utils/scheduler.hpp>

template<
    //
    class Counter,
    class Stepper,
    class PumpSignal,
    class HallSensorAdcChannel,
    class IrSensorAdcChannel>
class bartender_machine {
public:
    using scheduler_type = zoal::utils::lambda_scheduler<uint32_t, 8, 8>;
    using stepper_type = Stepper;
    using hall_channel = HallSensorAdcChannel;
    using ir_channel = IrSensorAdcChannel;
    using pump_signal = PumpSignal;

    static constexpr uint32_t steps_per_revolution = 4096u;
    static constexpr uint8_t rotation_direction = 1;
    static constexpr uint32_t debug_delay_ms = 0;
    static constexpr uint32_t step_delay_ms = 1;

    explicit bartender_machine(app_state &state)
        : app_state_(state) {}

    void handle() {
        scheduler_.handle(Counter::now());
    }

    void make_drink() {
        int value = ir_channel::read();
        portions_left_--;
        if (value < 700 && portions_left_ >= 0) {
            typename pump_signal::template mode<zoal::gpio::pin_mode::output>();
            typename pump_signal::high();
            scheduler_.schedule(0, portion_delay_, [this]() { make_next_if_needed(); });
        } else {
            scheduler_.schedule(0, 0, [this]() { make_next_if_needed(); });
        }
    }

    void go_to_segment_a() {
        typename pump_signal::low();
        stepper_.step_now();

        int value = hall_channel::read();
        bool result = detector_.handle(value);
        auto state = detector_.state;
        if (result && state == function_state::sector_a) {
            stepper_.stop();
            scheduler_.schedule(0, 0, [this]() { make_drink(); });
            return;
        }

        if (stepper_.steps_left > 0) {
            scheduler_.schedule(0, step_delay_ms, [this]() { go_to_segment_a(); });
        } else {
            stop_machine();
            stepper_.stop();

            app_state_.flags = app_state_flags_error;
        }
    }

    void go_to_rotate_30_deg() {
        typename pump_signal::low();

        stepper_.step_now();
        if (stepper_.steps_left > 0) {
            scheduler_.schedule(0, step_delay_ms, [this](){
                go_to_rotate_30_deg();
            });
        } else {
            stepper_.rotate(steps_per_revolution / total_segments_, rotation_direction);
            scheduler_.schedule(0, debug_delay_ms, [this](){
                go_to_segment_a();
            });
        }
    }

    void go_to_next_segment() {
        typename pump_signal::low();

        if (total_segments_ == 0) {
            stop_machine();
            return;
        }

        scheduler_.clear();
        stepper_.rotate(steps_per_revolution / 12, rotation_direction);
        scheduler_.schedule(0, 0, [this]() { go_to_rotate_30_deg(); });
    }

    void make_next_if_needed() {
        typename pump_signal::low();

        if (portions_left_ > 0) {
            go_to_next_segment();
        } else {
            portions_left_ = 0;
            stop_machine();
        }
    }

    void perform_calibration() {
        int value = hall_channel::read();
        bool result = detector_.handle(value);
        auto state = detector_.state;

        if (result && state == function_state::sector_a) {
            total_segments_++;
        }

        stepper_.step_now();
        if (stepper_.steps_left > 0) {
            scheduler_.schedule(0, step_delay_ms, [this]() { perform_calibration(); });
        } else {
            stepper_.stop();
            app_state_.flags = app_state_flags_idle;
            send_command(command_type::request_render_frame);
        }
    }

    void calibrate_rotate_30_degrees() {
        stepper_.step_now();
        if (stepper_.steps_left > 0) {
            scheduler_.schedule(0, step_delay_ms, [this]() { calibrate_rotate_30_degrees(); });
        } else {
            total_segments_ = 0;

            stepper_.stop();
            stepper_.rotate(steps_per_revolution, rotation_direction);
            scheduler_.schedule(0, debug_delay_ms, [this]() { perform_calibration(); });
        }
    }

    void calibrate_to_segment_a() {
        stepper_.step_now();

        int value = hall_channel::read();
        bool result = detector_.handle(value);
        auto state = detector_.state;
        if (result && state == function_state::sector_a) {
            total_segments_ = 0;
            stepper_.stop();
            stepper_.rotate(steps_per_revolution / 12, rotation_direction);
            scheduler_.schedule(0, debug_delay_ms, [this]() { calibrate_rotate_30_degrees(); });

            return;
        }

        if (stepper_.steps_left > 0) {
            scheduler_.schedule(0, step_delay_ms, [this]() { calibrate_to_segment_a(); });
        } else {
            stop_machine();
            app_state_.flags = app_state_flags_error;
        }
    }

    void calibrate() {
        app_state_.flags |= app_state_flags_calibration;
        app_state_.progress_fn = [this]() { return (float)(steps_per_revolution - stepper_.steps_left) / steps_per_revolution; };
        total_segments_ = 0;

        scheduler_.clear();
        stepper_.rotate(steps_per_revolution, rotation_direction);
        calibrate_to_segment_a();
    }

    void stop_machine() {
        typename pump_signal::low();

        portions_left_ = 0;
        stepper_.stop();
        scheduler_.clear();

        app_state_.flags = app_state_flags_idle;
        app_state_.progress_fn.reset();
    }

private:
    stepper_type stepper_;
    scheduler_type scheduler_;
    segment_detector detector_;

    uint32_t portion_delay_{850};
    int total_segments_{6};
    int portions_left_{0};

    app_state &app_state_;
};

#endif
