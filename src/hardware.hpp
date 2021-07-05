//
// Created by andrii on 03.07.21.
//

#ifndef NALYVAICHENKO_HARDWARE_HPP
#define NALYVAICHENKO_HARDWARE_HPP

#include "bartender_machine.hpp"

#include <zoal/board/arduino_mega.hpp>
#include <zoal/gfx/renderer.hpp>
#include <zoal/ic/ssd1306.hpp>
#include <zoal/io/rotary_encoder.hpp>
#include <zoal/io/button.hpp>
#include <zoal/io/stepper_28byj.hpp>
#include <zoal/periph/i2c.hpp>
#include <zoal/periph/i2c_request_dispatcher.hpp>
#include <zoal/utils/i2c_scanner.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

using pcb = zoal::board::arduino_mega;
using mcu = pcb::mcu;
using timer = mcu::timer_00;
using tty_usart = mcu::usart_00;
using adc = mcu::adc_00;
using i2c = mcu::i2c_00;

extern volatile uint32_t milliseconds;
using counter = zoal::utils::ms_counter<decltype(milliseconds), &milliseconds>;
using tools = zoal::utils::tool_set<mcu, F_CPU, counter, void>;
using delay = tools::delay;
using overflow_to_tick = zoal::utils::timer_overflow_to_tick<F_CPU, 32, 256>;

using i2c_req_dispatcher_type = zoal::periph::i2c_request_dispatcher<i2c, sizeof(void *) * 4>;
extern i2c_req_dispatcher_type i2c_req_dispatcher;
extern zoal::periph::i2c_request &request;
extern zoal::utils::i2c_scanner scanner;

using ssd1306_interface = zoal::ic::ssd1306_interface_i2c<delay, typename pcb::ard_d07, typename pcb::ard_d08, 0x3C>;
using display_type = zoal::ic::ssd1306<zoal::ic::ssd1306_resolution::ssd1306_128x64, ssd1306_interface>;
using adapter = zoal::ic::ssd1306_adapter_0<128, 64>;
using graphics = zoal::gfx::renderer<uint8_t, adapter>;
extern display_type display;

using hall_sensor = pcb::ard_a05;
using hall_channel = zoal::periph::adc_channel<mcu, adc, hall_sensor>;
using ir_sensor = pcb::ard_a04;
using ir_channel = zoal::periph::adc_channel<mcu, adc, ir_sensor>;
using pump_signal = pcb::ard_d02;

using stepper_type = zoal::io::stepper_28byj<pcb::ard_d31, pcb::ard_d29, pcb::ard_d27, pcb::ard_d25, 8>;
using bartender_machine_type = bartender_machine<counter, stepper_type, pump_signal, hall_channel, ir_channel>;
extern bartender_machine_type bartender;

using encoder_pin_a = pcb::ard_d38;
using encoder_pin_b = pcb::ard_d40;
using encoder_pin_btn = pcb::ard_d42;
using encoder_pin_vcc = pcb::ard_d44;
using encoder_pin_gnd = pcb::ard_d46;
using encoder_button_type = zoal::io::button<uint32_t, encoder_pin_btn>;
using encoder_type = zoal::io::rotary_encoder<
    //
    encoder_pin_a,
    encoder_pin_b,
    zoal::io::rotary_2phase_machine,
    zoal::gpio::pin_mode::input_floating>;
extern encoder_type encoder;
extern encoder_button_type encoder_button;

void initialize_hardware();
void initialize_i2c_devices();

#endif //NALYVAICHENKO_HARDWARE_HPP
