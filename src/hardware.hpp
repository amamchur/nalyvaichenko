//
// Created by andrii on 03.07.21.
//

#ifndef NALYVAICHENKO_HARDWARE_HPP
#define NALYVAICHENKO_HARDWARE_HPP

#include "./config.hpp"
#include "bartender_machine.hpp"
#include "df_player.hpp"

#include <zoal/board/arduino_mega.hpp>
#include <zoal/gfx/renderer.hpp>
#include <zoal/ic/ssd1306.hpp>
#include <zoal/ic/sh1106.hpp>
#include <zoal/io/button.hpp>
#include <zoal/io/rotary_encoder.hpp>
#include <zoal/io/stepper_28byj.hpp>
#include <zoal/periph/i2c.hpp>
#include <zoal/periph/i2c_request_dispatcher.hpp>
#include <zoal/utils/i2c_scanner.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

extern volatile uint32_t milliseconds;
using counter = zoal::utils::ms_counter<decltype(milliseconds), &milliseconds>;
using tools = zoal::utils::tool_set<mcu, F_CPU, counter, void>;
using delay = tools::delay;
using overflow_to_tick = zoal::utils::timer_overflow_to_tick<F_CPU, 32, 256>;

using i2c_req_dispatcher_type = zoal::periph::i2c_request_dispatcher<i2c, sizeof(void *) * 4>;
extern i2c_req_dispatcher_type i2c_req_dispatcher;
extern zoal::periph::i2c_request &request;
extern zoal::utils::i2c_scanner scanner;

using ssd1306_interface = zoal::ic::ssd1306_interface_i2c<0x3C>;
using oled_type = zoal::ic::sh1106<128, 64, ssd1306_interface>;
using adapter = zoal::ic::sh1106_adapter_0<128, 64>;
using graphics = zoal::gfx::renderer<uint8_t, adapter>;
extern oled_type screen;

using hall_channel = zoal::periph::adc_channel<mcu, adc, hall_sensor>;
using ir_channel = zoal::periph::adc_channel<mcu, adc, ir_sensor>;

using stepper_type = zoal::io::stepper_28byj<pcb::ard_d31, pcb::ard_d29, pcb::ard_d27, pcb::ard_d25, 8>;
using bartender_machine_type = bartender_machine<counter, stepper_type, pump_signal, hall_channel, ir_channel>;
extern bartender_machine_type bartender;

using encoder_button_type = zoal::io::button<uint32_t, encoder_pin_btn>;
using encoder_type = zoal::io::rotary_encoder<
    //
    encoder_pin_a,
    encoder_pin_b,
    zoal::io::rotary_2phase_machine,
    zoal::gpio::pin_mode::input_floating>;
extern encoder_type encoder;
extern encoder_button_type encoder_button;
extern df_player player;

void initialize_hardware();
void initialize_i2c_devices();

#endif //NALYVAICHENKO_HARDWARE_HPP
