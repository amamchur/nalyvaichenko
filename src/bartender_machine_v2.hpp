#ifndef NALYVAICHENKO_BARTENDER_MACHINE_V2_HPP
#define NALYVAICHENKO_BARTENDER_MACHINE_V2_HPP

#include "./message.hpp"
#include "./sector_detector.hpp"
#include "./stepper_kinematics.hpp"

#include <zoal/func/function.hpp>
#include <zoal/data/ring_buffer.hpp>

class bartender_machine_v2;

class bartender_machine_task {
public:
    zoal::func::function<16, void> start_;
    zoal::func::function<16, bool> handle_timer_;
    zoal::func::function<16, bool> handle_adc_;

    bartender_machine_task() = default;

    template<class S, class T, class A>
    bartender_machine_task(S s, T t, A a) {
        start_.template assign(s);
        handle_timer_.template assign(t);
        handle_adc_.template assign(a);
    }
private:

};

class bartender_machine_v2 : public event_handler {
public:
    static constexpr uint32_t step_per_rotation = 32 * 200;

    enum class state { idle, hold };

    enum class job { none, looking_segment };

    void update_period();
    void stop();
    void hold();
    [[noreturn]] void main_task();
    void handle_timer();
    void handle_adc();
    void rpm(uint32_t value);
    void next_segment();
    void push_find_segment();
    void process_event(event &e) override;

    void go();
    void motor_test();
private:
    zoal::data::ring_buffer<bartender_machine_task, 16> tasks_;

    state state_{state::idle};
    job job_{job::none};
    stepper_kinematics<> sk;
    sector_detector detector_;
    double speed_{step_per_rotation / 3.0};
    double acceleration_{step_per_rotation * 10};
    uint8_t segments_{6};

    void push_task(bartender_machine_task &task);
    void rotate(double steps, double speed = 0);
};

#endif
