#ifndef NALYVAICHENKO_BARTENDER_MACHINE_V2_HPP
#define NALYVAICHENKO_BARTENDER_MACHINE_V2_HPP

#include "./message.hpp"
#include "./sector_detector.hpp"
#include "./stepper_kinematics.hpp"

#include <zoal/data/ring_buffer.hpp>
#include <zoal/func/function.hpp>

class bartender_machine_v2;

class bartender_machine_task {
public:
    uint8_t state{0};
    uint8_t portion{0};
    zoal::func::function<16, bool, bartender_machine_task &> start_;
    zoal::func::function<16, bool, bartender_machine_task &> handle_timer_;
    zoal::func::function<16, bool, bartender_machine_task &> handle_adc_;

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

    void stop_machine();
    void hold();
    [[noreturn]] void main_task();
    void handle_timer();
    void handle_adc();
    void rpm(uint32_t value);
    void next_segment();
    void process_event(event &e) override;

    void go();
    void motor_test();

private:
    zoal::data::ring_buffer<bartender_machine_task, 16> tasks_;

    stepper_kinematics<> sk;
    sector_detector detector_;
    float speed_{step_per_rotation / 3.0};
    float acceleration_{step_per_rotation * 2};
    uint8_t segments_{6};
    int correction_{16};
    uint32_t portion_time_ms_{850};
    uint16_t ir_value_{400};

    bool update_period();
    void push_task(bartender_machine_task &task);
    void push_find_segment();
    void push_release();
    void absolute_rotate(float steps, float speed = 0);
    void relative_rotate(float steps, float speed = 0);

    static bool null_task_handler(bartender_machine_task &);
};

#endif
