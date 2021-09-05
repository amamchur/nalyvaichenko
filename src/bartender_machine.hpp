#ifndef NALYVAICHENKO_BARTENDER_MACHINE_HPP
#define NALYVAICHENKO_BARTENDER_MACHINE_HPP

#include "./app_state.hpp"
#include "./message.hpp"
#include "./sector_detector.hpp"
#include "./tty_terminal.hpp"
#include "./voice.hpp"

#include <stdint.h>
#include <zoal/gpio/pin_mode.hpp>
#include <zoal/utils/scheduler.hpp>

template<
    //
    class TicksType,
    class Stepper,
    class PumpSignal,
    class HallSensorAdcChannel,
    class IrSensorAdcChannel>
class bartender_machine : public event_handler {
public:
    using scheduler_type = zoal::utils::lambda_scheduler<uint32_t, 8, 8>;
    using stepper_type = Stepper;
    using hall_channel = HallSensorAdcChannel;
    using ir_channel = IrSensorAdcChannel;
    using pump_signal = PumpSignal;

    static constexpr uint32_t steps_per_revolution = 4096u;
    static constexpr uint8_t forward_direction = 1;
    static constexpr uint32_t debug_delay_ms = 0;
    static constexpr uint32_t step_delay_ms = 1;

    void process_event(event &e) override {
        auto segments = global_app_state.settings.segments_;
        auto rs = global_app_state.settings.revolver_settings_[segments];
        switch (e.type) {
        case event_type::settings_changed:
            total_segments_ = segments;
            portion_time_ = rs.portion_time_;
            portion_delay_ = rs.portion_delay_;
            ir_max_value_ = rs.ir_max_value_;
            ir_min_value_ = rs.ir_min_value_;
            sector_adjustment_ = rs.sector_adjustment_;
            detector_.rising_threshold = global_app_state.settings.hall_rising_threshold;
            detector_.falling_threshold = global_app_state.settings.hall_falling_threshold;
            break;
        default:
            break;
        }
    }

    void handle(TicksType ms) {
        scheduler_.handle(ms);
    }

    void calibrate() {
        memset(hall_values_per_segment_, 0, sizeof(hall_values_per_segment_));

        min_hall_value_ = INT16_MAX;
        max_hall_value_ = 0;
        total_segments_ = 0;

        scheduler_.clear();
        stepper_.rotate(steps_per_revolution, forward_direction);
        calibrate_to_segment_a();
    }

    void stop_machine() {
        typename pump_signal::low();
        typename valve_signal::low();

        portions_left_ = 0;
        stepper_.stop();
        scheduler_.clear();
        send_event(event_type::machine_stop);
    }

    void start() {
        stop_machine();

        portions_left_ = total_segments_;
        portions_made_ = 0;
        go_to_next_segment();
    }

    void next_segment() {
        stop_machine();
        go_to_next_segment();
    }

private:
    void calibrate_rotate_30_degrees() {
        if (stepper_.steps_left > 0) {
            stepper_.step_now();
            scheduler_.schedule(0, step_delay_ms, [this]() { calibrate_rotate_30_degrees(); });
        } else {
            stepper_.rotate(steps_per_revolution, forward_direction);
            scheduler_.schedule(0, debug_delay_ms, [this]() { perform_calibration(); });
        }
    }

    void calibrate_to_segment_a() {
        int value = hall_channel::read();
        auto result = detector_.handle(value);
        auto state = detector_.sector_state_;
        if (result == detection_result::changed && state == sector_state::sector) {
            stepper_.rotate(steps_per_revolution / 12, forward_direction);
            scheduler_.schedule(0, debug_delay_ms, [this]() { calibrate_rotate_30_degrees(); });
            return;
        }

        if (stepper_.steps_left > 0) {
            stepper_.step_now();
            scheduler_.schedule(0, step_delay_ms, [this]() { calibrate_to_segment_a(); });
        } else {
            stop_machine();
            send_command(command_type::render_screen);
        }
    }

    void stop_pump_and_go_to_next() {
        typename pump_signal::low();
        typename valve_signal::low();

        portions_made_++;
        command cmd{};
        cmd.type = command_type::play;
        cmd.value = voice::cheers + portions_made_;
        send_command(cmd);
        scheduler_.schedule(0, portion_delay_, [this]() { make_next_if_needed(); });
    }

    void perform_adjust_segment() {
        if (stepper_.steps_left > 0) {
            stepper_.step_now();
            scheduler_.schedule(0, step_delay_ms, [this]() { perform_adjust_segment(); });
        } else {
            scheduler_.schedule(0, 0, [this]() { make_portion(); });
        }
    }

    void adjust_segment() {
        int dir = sector_adjustment_ > 0 ? forward_direction : 1 - forward_direction;
        stepper_.rotate(abs(sector_adjustment_), dir);
        scheduler_.schedule(0, step_delay_ms, [this]() { perform_adjust_segment(); });
    }

    void make_portion() {
        int value = ir_channel::read();
        portions_left_--;

        bool match = value > ir_min_value_ && value < ir_max_value_;
        if (match && portions_left_ >= 0) {
            typename pump_signal::high();
            typename valve_signal::high();
            scheduler_.schedule(0, portion_time_, [this]() { stop_pump_and_go_to_next(); });
        } else {
            scheduler_.schedule(0, 0, [this]() { make_next_if_needed(); });
        }
    }

    void go_to_segment_a() {
        typename pump_signal::low();
        typename valve_signal::low();

        int value = hall_channel::read();
        auto result = detector_.handle(value);
        auto state = detector_.sector_state_;
        if (result == detection_result::changed && state == sector_state::sector) {
            scheduler_.schedule(0, 0, [this]() { adjust_segment(); });
            return;
        }

        if (stepper_.steps_left > 0) {
            stepper_.step_now();
            scheduler_.schedule(0, step_delay_ms, [this]() { go_to_segment_a(); });
        } else {
            stop_machine();
        }
    }

    void rotate_30_deg() {
        typename pump_signal::low();
        typename valve_signal::low();

        if (stepper_.steps_left > 0) {
            stepper_.step_now();
            scheduler_.schedule(0, step_delay_ms, [this]() { rotate_30_deg(); });
        } else {
            stepper_.rotate(steps_per_revolution / total_segments_, forward_direction);
            scheduler_.schedule(0, debug_delay_ms, [this]() { go_to_segment_a(); });
        }
    }

    void go_to_next_segment() {
        typename pump_signal::low();
        typename valve_signal::low();

        if (total_segments_ == 0) {
            stop_machine();
            return;
        }

        scheduler_.clear();
        stepper_.rotate(steps_per_revolution / 12, forward_direction);
        scheduler_.schedule(0, 0, [this]() { rotate_30_deg(); });
    }

    void make_next_if_needed() {
        typename pump_signal::low();
        typename valve_signal::low();

        if (portions_left_ == 0) {
            command cmd{};
            cmd.type = command_type::play;
            cmd.value = voice::cheers;
            send_command(cmd);
        }

        if (portions_left_ > 0) {
            go_to_next_segment();
        } else {
            portions_left_ = 0;
            stop_machine();
        }
    }

    void perform_calibration() {
        int value = hall_channel::read();
        auto result = detector_.handle(value);
        auto state = detector_.sector_state_;

        if (max_hall_value_ < value) {
            max_hall_value_ = value;
        }

        if (min_hall_value_ > value) {
            min_hall_value_ = value;
        }

        if (result == detection_result::changed && state == sector_state::sector) {
            hall_values_per_segment_[total_segments_] = max_hall_value_;
            total_segments_++;
        }

        if (stepper_.steps_left > 0) {
            stepper_.step_now();
            scheduler_.schedule(0, step_delay_ms, [this]() { perform_calibration(); });
        } else {
            stepper_.stop();
            global_app_state.max_hall_value_ = max_hall_value_;
            global_app_state.min_hall_value_ = min_hall_value_;
            global_app_state.settings.segments_ = total_segments_;
            global_app_state.save_settings();
            send_event(event_type::calibration_finished);

            command cmd{};
            cmd.type = command_type::play;
            cmd.value = voice::calibration_finished;
            send_command(cmd);

            cmd.type = command_type::play;
            cmd.value = voice::segs_found + total_segments_;
            send_command(cmd);
            //
            //            tty_stream << "total_segments_: " << total_segments_ << "\r\b";
            //
            //            for (int i = 0; i < total_segments_; i++) {
            //                tty_stream << hall_values_per_segment_[i] << " ";
            //            }
            //            tty_stream << "\r\n";
        }
    }

    stepper_type stepper_;
    scheduler_type scheduler_;
    sector_detector detector_;

    uint32_t portion_time_{850};
    uint32_t portion_delay_{100};
    int portions_left_{0};
    int portions_made_{0};
    int total_segments_{6};
    int ir_min_value_{0};
    int ir_max_value_{300};
    int min_hall_value_{0};
    int max_hall_value_{0};
    int hall_values_per_segment_[6];
    int sector_adjustment_{-10};
};

#endif
