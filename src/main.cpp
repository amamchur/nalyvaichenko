//
// Created by andrii on 04.04.21.
//

#include "./cmd_line_parser.hpp"
#include "./segment_detector.hpp"
#include "./terminal_input.hpp"

#include <avr/io.h>
#include <zoal/arch/avr/stream.hpp>
#include <zoal/arch/avr/utils/usart_transmitter.hpp>
#include <zoal/board/arduino_uno.hpp>
#include <zoal/io/stepper_28byj.hpp>
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

scheduler_type hardware_scheduler;
scheduler_type general_scheduler;

using stepper_type = zoal::io::stepper_28byj<pcb::ard_d10, pcb::ard_d11, pcb::ard_d12, pcb::ard_d13, 8>;
stepper_type stepper;

using usart_tx_transport = zoal::utils::usart_transmitter<usart, 32, zoal::utils::interrupts_off>;
usart_tx_transport transport;
using tx_stream_type = zoal::io::output_stream<usart_tx_transport>;
tx_stream_type stream(transport);

zoal::data::ring_buffer<uint8_t, 16> rx_buffer;

segment_detector detector;

constexpr size_t terminal_str_size = 64;
using command_line_parser = zoal::misc::command_line_parser;
char terminal_buffer[terminal_str_size];
zoal::misc::terminal_input terminal(terminal_buffer, sizeof(terminal_buffer));
auto terminal_greeting = "\033[0;32mmcu\033[m$ ";

const char help_cmd[] PROGMEM = "help";
const char calibrate_cmd[] PROGMEM = "calibrate";
const char next_cmd[] PROGMEM = "next";
const char go_cmd[] PROGMEM = "go";
const char adc_cmd[] PROGMEM = "adc";
const char pump_cmd[] PROGMEM = "pump";

using hall_sensor = pcb::ard_a05;
using ir_sensor = pcb::ard_a04;
using pump_signal = pcb::ard_d02;

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

uint32_t portion_delay = 850;
int total_segments = 6;
int drinks_left = 0;

void stop_machine(void *v = nullptr) {
    drinks_left = 0;
    stepper.stop();
    hardware_scheduler.clear();
    pump_signal::low();
}

void perform_calibration(void *) {
    mcu::mux::adc<adc, hall_sensor>::connect();
    int value = adc::read();
    bool result = detector.handle(value);
    auto state = detector.state;
    if (result && state == function_state::sector_a) {
        stream << "Segment: " << total_segments << "\r\n";
        total_segments++;
    }

    stepper.step_now();
    if (stepper.steps_left > 0) {
        hardware_scheduler.schedule(step_delay_ms, perform_calibration);
    } else {
        stepper.stop();

        stream << "Total segments: " << total_segments << "\r\n";
        terminal.sync();
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

    mcu::mux::adc<adc, hall_sensor>::connect();
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
        stop_machine();
        stream << "Revolver error!!!\r\n";
        terminal.sync();
    }
}

void calibrate() {
    total_segments = 0;

    hardware_scheduler.clear();
    stepper.rotate(steps_per_revolution, rotation_direction);
    calibrate_to_segment_a(nullptr);
}

void go_to_next_segment();

void make_next_if_needed(void *) {
    stream << "drinks_left: " << drinks_left << "\r\n";

    if (drinks_left > 0) {
        go_to_next_segment();
    } else {
        drinks_left = 0;
        stop_machine();
    }
}

void make_drink(void *) {
    mcu::mux::adc<adc, ir_sensor>::connect();
    int value = adc::read();
    drinks_left--;
    if (value < 700 && drinks_left >= 0) {
        stream << "make_drink\r\n";

        pump_signal::mode<zoal::gpio::pin_mode::output>();
        pump_signal::high();
        delay::ms(portion_delay);
        pump_signal::low();
        hardware_scheduler.schedule(200, make_next_if_needed);
    } else {
        hardware_scheduler.schedule(0, make_next_if_needed);
    }
}

void go_to_segment_a(void *) {
    pump_signal::low();
    stepper.step_now();

    mcu::mux::adc<adc, hall_sensor>::connect();
    int value = adc::read();
    bool result = detector.handle(value);
    auto state = detector.state;
    if (result && state == function_state::sector_a) {
        stepper.stop();
        terminal.sync();

        hardware_scheduler.schedule(0, make_drink);
        return;
    }

    if (stepper.steps_left > 0) {
        hardware_scheduler.schedule(step_delay_ms, go_to_segment_a);
    } else {
        stop_machine();
        stepper.stop();
        stream << "Revolver error!!!\r\n";
        terminal.sync();
    }
}

void go_to_rotate_30_deg(void *) {
    pump_signal::low();
    stepper.step_now();
    if (stepper.steps_left > 0) {
        hardware_scheduler.schedule(step_delay_ms, go_to_rotate_30_deg);
    } else {
        stepper.rotate(steps_per_revolution / total_segments, rotation_direction);
        hardware_scheduler.schedule(debug_delay_ms, go_to_segment_a);
    }
}

void go_to_next_segment() {
    pump_signal::low();

    if (total_segments == 0) {
        stop_machine();
        return;
    }

    hardware_scheduler.clear();
    stepper.rotate(steps_per_revolution / 12, rotation_direction);
    hardware_scheduler.schedule(0, go_to_rotate_30_deg);
}

void vt100_callback(const zoal::misc::terminal_input *, const char *s, const char *e) {
    transport.send_data(s, e - s);
}

void cmd_select_callback(zoal::misc::command_line_machine *p, zoal::misc::command_line_event e) {
    if (e == zoal::misc::command_line_event::line_end) {
        return;
    }

    auto ts = p->token_start();
    auto te = p->token_end();
    p->callback(&command_line_parser::empty_callback);

    stream << "\r\n";

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(help_cmd), ts, te)) {
        stream << "Help!!\r\n";
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(calibrate_cmd), ts, te)) {
        calibrate();
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(next_cmd), ts, te)) {
        drinks_left = 1;
        go_to_next_segment();
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(go_cmd), ts, te)) {
        drinks_left = total_segments;
        go_to_next_segment();
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(pump_cmd), ts, te)) {
        pump_signal::mode<zoal::gpio::pin_mode::output>();
        pump_signal::high();
        delay::ms(500);
        stop_machine();
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(adc_cmd), ts, te)) {
        mcu::mux::adc<adc, hall_sensor>::connect();
        int value = adc::read();
        stream << "hall: " << value << "\r\n";

        mcu::mux::adc<adc, ir_sensor>::connect();
        value = adc::read();
        stream << "ir: " << value << "\r\n";
    }

    terminal.sync();
}

void input_callback(const zoal::misc::terminal_input *, const char *s, const char *e) {
    command_line_parser cmd_parser(nullptr, 0);
    cmd_parser.callback(cmd_select_callback);
    cmd_parser.scan(s, e, e);
    terminal.sync();
}

int main() {
    initialize_hardware();

    terminal.vt100_feedback(&vt100_callback);
    terminal.input_callback(&input_callback);
    terminal.greeting(terminal_greeting);
    terminal.clear();
    terminal.sync();

    //    pump_signal::mode<zoal::gpio::pin_mode::output>();
    //    pump_signal::high();
    //    delay::ms(100);
    //    stop_machine();

    while (true) {
        uint8_t rx_byte = 0;
        bool result;
        {
            zoal::utils::interrupts_off scope_off;
            result = rx_buffer.pop_front(rx_byte);
        }

        if (result) {
            terminal.push(&rx_byte, 1);
        }

        hardware_scheduler.handle();
        general_scheduler.handle();
    }

    return 0;
}

ISR(ADC_vect) {}

ISR(TIMER0_OVF_vect) {
    milliseconds += overflow_to_tick::step();
}

ISR(USART_RX_vect) {
    usart::rx_handler<>([](uint8_t value) { rx_buffer.push_back(value); });
}

ISR(USART_UDRE_vect) {
    usart::tx_handler([](uint8_t &value) { return usart_tx_transport::tx_buffer.pop_front(value); });
}
