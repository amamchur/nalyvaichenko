#ifndef NALYVAICHENKO_HARDWARE_ATMEGA2560_HPP
#define NALYVAICHENKO_HARDWARE_ATMEGA2560_HPP

#include "./config.hpp"
#include "bartender_machine.hpp"
#include "df_player.hpp"

#include <zoal/board/arduino_mega.hpp>
#include <zoal/gfx/renderer.hpp>
#include <zoal/ic/sh1106.hpp>
#include <zoal/ic/ssd1306.hpp>
#include <zoal/io/button.hpp>
#include <zoal/io/rotary_encoder.hpp>
#include <zoal/io/stepper_28byj.hpp>
#include <zoal/periph/adc_request_dispatcher.hpp>
#include <zoal/periph/i2c.hpp>
#include <zoal/periph/i2c_request_dispatcher.hpp>
#include <zoal/utils/i2c_scanner.hpp>

template<>
class zoal::periph::adc_dispatcher<sensor_adc> : public zoal::periph::adc_dispatcher_base<sensor_adc, sizeof(void *) * 4> {};

using i2c_req_dispatcher_type = zoal::periph::i2c_request_dispatcher<i2c, sizeof(void *) * 4>;

extern i2c_req_dispatcher_type i2c_req_dispatcher;
extern zoal::utils::i2c_scanner scanner;

extern oled_type screen;

using encoder_button_config = zoal::io::button_config<true, 20, 500, 50>;
using button_config = zoal::io::button_config<true, 20, 500, 0>;

using encoder_button_type = zoal::io::button<uint32_t, encoder_pin_btn, encoder_button_config>;
using start_button_type = zoal::io::button<uint32_t, start_signal, button_config>;
using stop_button_type = zoal::io::button<uint32_t, stop_signal, button_config>;
using encoder_type = zoal::io::rotary_encoder<
    //
    encoder_pin_a,
    encoder_pin_b,
    zoal::io::rotary_2phase_machine>;
extern encoder_type encoder;
extern start_button_type start_button;
extern stop_button_type stop_button;
extern encoder_button_type encoder_button;
extern df_player player;

void initialize_hardware();
void initialize_i2c_devices();

#endif
