//
// Created by andrii on 04.04.21.
//

#include "./segment_detector.hpp"

#include <zoal/arch/avr/utils/usart_transmitter.hpp>
#include <zoal/board/arduino_uno.hpp>
#include <zoal/io/stepper_28byj.hpp>
#include <zoal/shield/uno_multi_functional.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

FUSES = {.low = 0xFF, .high = 0xD7, .extended = 0xFC};

constexpr uint32_t steps_per_revolution = 4096u;
constexpr uint32_t debug_delay_ms = 0;
constexpr uint32_t step_delay_ms = 1;
constexpr uint8_t rotation_direction = 1;

volatile uint32_t milliseconds = 0;

using pcb = zoal::board::arduino_uno;
using mcu = pcb::mcu;
using timer = mcu::timer_00;
using usart = mcu::usart_00;
using adc = mcu::adc_00;
using counter = zoal::utils::ms_counter<decltype(milliseconds), &milliseconds>;
using tools = zoal::utils::tool_set<mcu, F_CPU, counter, void>;
using delay = tools::delay;
using overflow_to_tick = zoal::utils::timer_overflow_to_tick<F_CPU, 32, 256>;
using scheduler_type = zoal::utils::function_scheduler<counter, 8, void *>;
using shield_type = zoal::shield::uno_multi_functional<pcb, uint32_t>;

shield_type shield;
scheduler_type hardware_scheduler;
scheduler_type general_scheduler;

//using stepper_type = zoal::io::stepper_28byj<pcb::ard_d08, pcb::ard_d09, pcb::ard_d10, pcb::ard_d11, 8>;
using stepper_type = zoal::io::stepper_28byj<pcb::ard_d10, pcb::ard_d11, pcb::ard_d12, pcb::ard_d13, 8>;
stepper_type stepper;

using usart_tx_transport = zoal::utils::usart_transmitter<usart, 32, zoal::utils::interrupts_off>;
usart_tx_transport transport;
using tx_stream_type = zoal::io::output_stream<usart_tx_transport>;
tx_stream_type stream(transport);

zoal::data::ring_buffer<uint8_t, 16> rx_buffer;

segment_detector detector;

void initialize_hardware() {
    using namespace zoal::gpio;
    using usart_cfg = zoal::periph::usart_115200<F_CPU>;
    using adc_cfg = zoal::periph::adc_config<>;

    // Power on modules
    api::optimize<api::clock_on<usart, timer>>();

    // Disable all modules before applying settings
    api::optimize<api::disable<usart, timer, adc>>();
    api::optimize<
        //
        mcu::mux::usart<usart, mcu::pd_00, mcu::pd_01, mcu::pd_04>::connect,
        mcu::cfg::usart<usart, usart_cfg>::apply,
        //
        mcu::mux::adc<adc, pcb::ard_a00>::connect,
        mcu::cfg::adc<adc, adc_cfg>::apply,
        //
        mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply,
        //
        mcu::irq::timer<timer>::enable_overflow_interrupt,
        //
        shield_type::gpio_cfg,
        stepper_type::gpio_cfg
        //
        >();

    // Enable system interrupts
    zoal::utils::interrupts::on();

    // Enable all modules
    api::optimize<api::enable<usart, timer, adc>>();

    adc::enable();
    adc::enable_interrupt();
}

uint16_t min_adc = 0xFFFF;
uint16_t max_adc = 0;

int total_segments = 0;

void perform_calibration(void *) {
    mcu::mux::adc<adc, pcb::ard_a05>::connect();
    int value = adc::read();
    bool result = detector.handle(value);
    auto state = detector.state;
    if (result && state == function_state::sector_a) {
        stream << "Segment: " << total_segments << "\r\n";
        total_segments++;
        shield.dec_to_segments(total_segments);

        shield_type::beeper::on();
        delay::ms(5);
        shield_type::beeper::off();
    }

    stepper.step_now();
    if (stepper.steps_left > 0) {
        hardware_scheduler.schedule(step_delay_ms, perform_calibration);
    } else {
        stepper.stop();

        shield.dec_to_segments(total_segments);
        stream << "Total segments: " << total_segments << "\r\n";
        stream << "min_adc: " << min_adc << "\r\n";
        stream << "max_adc: " << max_adc << "\r\n";
    }
}

void calibrate_rotate_30_degrees(void *) {
    stepper.step_now();
    if (stepper.steps_left > 0) {
        hardware_scheduler.schedule(step_delay_ms, calibrate_rotate_30_degrees);
    } else {
        stream << "Detecting segments "
               << "\r\n";
        total_segments = 0;

        stepper.stop();
        stepper.rotate(steps_per_revolution, rotation_direction);
        hardware_scheduler.schedule(debug_delay_ms, perform_calibration);
    }
}

void calibrate_to_segment_a(void *) {
    stepper.step_now();

    mcu::mux::adc<adc, pcb::ard_a05>::connect();
    int value = adc::read();
    bool result = detector.handle(value);
    auto state = detector.state;
    if (result && state == function_state::sector_a) {
        total_segments = 0;
        stepper.stop();
        stepper.rotate(steps_per_revolution / 12, rotation_direction);
        hardware_scheduler.schedule(debug_delay_ms, calibrate_rotate_30_degrees);
        return;
    }

    if (stepper.steps_left > 0) {
        hardware_scheduler.schedule(step_delay_ms, calibrate_to_segment_a);
    } else {
        stream << "Revolver error!!!\r\n";
    }
}

void calibrate() {
    total_segments = 0;
    shield.dec_to_segments(total_segments);

    hardware_scheduler.clear();
    stepper.rotate(steps_per_revolution, rotation_direction);
    calibrate_to_segment_a(nullptr);
}

void go_to_segment_a(void *) {
    stepper.step_now();

    mcu::mux::adc<adc, pcb::ard_a05>::connect();
    int value = adc::read();
    bool result = detector.handle(value);
    auto state = detector.state;
    if (result && state == function_state::sector_a) {
        stepper.stop();
        shield_type::beeper::on();
        delay::ms(5);
        shield_type::beeper::off();
        return;
    }

    if (stepper.steps_left > 0) {
        hardware_scheduler.schedule(step_delay_ms, go_to_segment_a);
    } else {
        stream << "Revolver error!!!\r\n";
    }
}

void go_to_rotate_30_deg(void *) {
    stepper.step_now();
    if (stepper.steps_left > 0) {
        hardware_scheduler.schedule(step_delay_ms, go_to_rotate_30_deg);
    } else {
        stepper.rotate(steps_per_revolution, rotation_direction);
        hardware_scheduler.schedule(debug_delay_ms, go_to_segment_a);
    }
}

void go_to_next_segment() {
    hardware_scheduler.clear();
    stepper.rotate(steps_per_revolution / 12, rotation_direction);
    hardware_scheduler.schedule(0, go_to_rotate_30_deg);
}

void button_handler(zoal::io::button_event e, uint8_t button) {
    if (e != zoal::io::button_event::press) {
        return;
    }

    switch (button) {
    case 0:
        go_to_next_segment();
        break;
    case 1:
        calibrate();
        break;
    case 2:
        stream << "Button 2!\r\n";
        break;
    default:
        break;
    }
}

int main() {
    initialize_hardware();

    mcu::mux::adc<adc, pcb::ard_a05>::connect();
    detector.handle(adc::read());

    stream << "\033cStart!!!\r\n";

    shield.dec_to_segments(total_segments);
    calibrate();

    while (true) {
        hardware_scheduler.handle();
        general_scheduler.handle();

        shield.handle_buttons(milliseconds, button_handler);
        shield.dynamic_indication();
    }

    return 0;
}

ISR(ADC_vect) {
    auto v = adc::value();
    if (max_adc < v) {
        max_adc = v;
    }

    if (min_adc > v) {
        min_adc = v;
    }
}

ISR(TIMER0_OVF_vect) {
    milliseconds += overflow_to_tick::step();
}

ISR(USART_RX_vect) {
    usart::rx_handler<>([](uint8_t value) { rx_buffer.push_back(value); });
}

ISR(USART_UDRE_vect) {
    usart::tx_handler([](uint8_t &value) { return usart_tx_transport::tx_buffer.pop_front(value); });
}
