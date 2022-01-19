#ifndef NALYVAICHENKO_CONFIG_STM32F401_HPP
#define NALYVAICHENKO_CONFIG_STM32F401_HPP

#include "stm32f4xx_hal.h"

#include <zoal/freertos/event_group.hpp>
#include <zoal/freertos/stream_buffer.hpp>
#include <zoal/freertos/task.hpp>
#include <zoal/gfx/glyph_renderer.hpp>
#include <zoal/gfx/renderer.hpp>
#include <zoal/ic/sh1106.hpp>
#include <zoal/ic/w25qxx.hpp>
#include <zoal/mcu/stm32f401ccux.hpp>
#include <zoal/mem/reserve_mem.hpp>
#include <zoal/periph/i2c.hpp>
#include <zoal/utils/cmsis_os2/delay.hpp>

constexpr uint32_t system_clock_freq = 84000000;
constexpr uint32_t ahb_clock_freq = 84000000;
constexpr uint32_t apb1_clock_freq = 42000000;
constexpr uint32_t apb2_clock_freq = 84000000;

using counter = zoal::utils::ms_counter<uint32_t, &uwTick>;
using mcu = zoal::mcu::stm32f401ccux;
using delay = zoal::utils::cmsis_os2::delay<84000000>;

using tty_usart = mcu::usart_01;
using tty_usart_rx = mcu::pb_07;
using tty_usart_tx = mcu::pb_06;
using tty_usart_params = zoal::periph::usart_115200<apb2_clock_freq>;
using tty_usart_mux = mcu::mux::usart<tty_usart, tty_usart_rx, tty_usart_tx>;
using tty_usart_cfg = mcu::cfg::usart<tty_usart, tty_usart_params>;

using oled_spi = mcu::spi_02;
using oled_miso = mcu::pb_14;
using oled_mosi = mcu::pb_15;
using oled_sck = mcu::pb_13;
using oled_cs = mcu::pa_10;
using oled_ds = mcu::pa_09;
using oled_res = mcu::pa_08;
using oled_spi_params = zoal::periph::spi_params<apb2_clock_freq>;
using oled_spi_mux = mcu::mux::spi<oled_spi, oled_miso, oled_mosi, oled_sck>;
using oled_spi_cfg = mcu::cfg::spi<oled_spi, oled_spi_params>;

using flash_spi = mcu::spi_01;
using flash_spi_mosi = mcu::pa_06;
using flash_spi_miso = mcu::pa_07;
using flash_spi_sck = mcu::pa_05;
using flash_spi_cs = mcu::pa_04;
using flash_spi_params = zoal::periph::spi_params<apb2_clock_freq>;
using flash_spi_mux = mcu::mux::spi<flash_spi, flash_spi_mosi, flash_spi_miso, flash_spi_sck>;
using flash_spi_cfg = mcu::cfg::spi<flash_spi, flash_spi_params>;
using w25q32 = zoal::ic::w25qxx<flash_spi, flash_spi_cs, delay>;

using df_player_usart = mcu::usart_02;
using df_player_usart_rx = mcu::pa_03;
using df_player_usart_tx = mcu::pa_02;
using df_player_busy = mcu::pc_13;
using df_player_usart_params = zoal::periph::usart_9600<apb1_clock_freq>;
using df_player_mux = mcu::mux::usart<df_player_usart, df_player_usart_rx, df_player_usart_tx>;
using df_player_cfg = mcu::cfg::usart<df_player_usart, df_player_usart_params>;

using sensor_adc = mcu::adc_01;
using adc_params = zoal::periph::adc_params<>;
using adc_cfg = mcu::cfg::adc<sensor_adc, adc_params>;

using i2c = mcu::i2c_01;
using i2c_clk = mcu::pb_08;
using i2c_sda = mcu::pb_09;
using i2c_params = zoal::periph::i2c_fast_mode<apb1_clock_freq>;
using i2c_mux = mcu::mux::i2c<i2c, i2c_sda, i2c_clk>;
using i2c_cfg = mcu::cfg::i2c<i2c, i2c_params>;

using encoder_pin_a = mcu::pb_05;
using encoder_pin_b = mcu::pb_04;
using encoder_pin_btn = mcu::pb_03;

using hall_sensor = mcu::pa_00;
using ir_sensor = mcu::pa_01;
using start_signal = mcu::pc_14;
using stop_signal = mcu::pc_15;

using motor_dir = mcu::pb_02;
using motor_en = mcu::pb_10;
using motor_step = mcu::pa_15;

constexpr uint32_t pwm_divider = 83;
constexpr uint32_t pwm_period = 1999;
constexpr uint32_t micro_steps = 32;
constexpr uint32_t steps_per_revolution = 200 * micro_steps;

using machine_timer = mcu::timer_02;
using machine_timer_params = zoal::periph::timer_params<
    //
    apb2_clock_freq,
    pwm_divider,
    pwm_period,
    zoal::periph::timer_mode::up>;
using machine_timer_cfg = mcu::cfg::timer<machine_timer, machine_timer_params>;
using motor_step_pwm_channel = mcu::mux::pwm_channel<machine_timer, motor_step>;

using pump_pwm_timer = mcu::timer_03;
using pump_pwm_timer_params = zoal::periph::timer_params<apb2_clock_freq, 83, 999, zoal::periph::timer_mode::up>;
using pump_pwm_timer_cfg = mcu::cfg::timer<pump_pwm_timer, pump_pwm_timer_params>;

using pump_signal = mcu::pb_00;
using pump_pwm_channel = mcu::mux::pwm_channel<pump_pwm_timer, pump_signal>;

using valve_signal = mcu::pb_05;

using hall_adc_channel = mcu::mux::adc_channel<sensor_adc, hall_sensor, 56>;
using ir_adc_channel = mcu::mux::adc_channel<sensor_adc, ir_sensor, 56>;

using usart_stream_type = zoal::freertos::stream_buffer<zoal::freertos::freertos_allocation_type::static_mem>;
extern zoal::mem::reserve_mem<usart_stream_type, 32> tty_rx_stream;
extern zoal::mem::reserve_mem<usart_stream_type, 32> tty_tx_stream;
extern zoal::mem::reserve_mem<usart_stream_type, 32> player_rx_stream;
extern zoal::mem::reserve_mem<usart_stream_type, 32> player_tx_stream;

class tty_tx_transport {
public:
    static void send_byte(uint8_t value);
    static void send_data(const void *data, size_t size);
};

class df_player_tx_transport {
public:
    static void send_byte(uint8_t value);
    static void send_data(const void *data, size_t size);
};

using adapter = zoal::ic::sh1106_adapter_0<128, 64>;
using graphics = zoal::gfx::renderer<uint8_t, adapter>;
using oled_type = zoal::ic::sh1106_spi<128, 64, oled_spi, oled_res, oled_ds, oled_cs, delay>;

#endif
