#include "adc.h"
#include "config.hpp"
#include "event_manager.hpp"
#include "hardware.hpp"

zoal::mem::reserve_mem<usart_stream_type, 32> tty_rx_stream(1);
zoal::mem::reserve_mem<usart_stream_type, 32> tty_tx_stream(1);

zoal::mem::reserve_mem<usart_stream_type, 32> player_rx_stream(1);
zoal::mem::reserve_mem<usart_stream_type, 32> player_tx_stream(1);

i2c_req_dispatcher_type i2c_req_dispatcher;

zoal::periph::i2c_request &request = i2c_req_dispatcher.request;
zoal::utils::i2c_scanner scanner;
oled_type screen;

encoder_type encoder;
start_button_type start_button;
stop_button_type stop_button;

encoder_button_type encoder_button;
df_player player;

void initialize_hardware() {
    using api = zoal::gpio::api;

    using tty_usart_params = zoal::periph::usart_115200<apb2_clock_freq>;
    using tty_usart_mux = mcu::mux::usart<tty_usart, tty_usart_rx, tty_usart_tx>;
    using tty_usart_cfg = mcu::cfg::usart<tty_usart, tty_usart_params>;

    using i2c_params = zoal::periph::i2c_fast_mode<apb1_clock_freq>;
    using i2c_mux = mcu::mux::i2c<i2c, i2c_sda, i2c_clk>;
    using i2c_cfg = mcu::cfg::i2c<i2c, i2c_params>;

    using flash_spi_params = zoal::periph::spi_params<apb2_clock_freq>;
    using flash_spi_mux = mcu::mux::spi<flash_spi, flash_spi_mosi, flash_spi_miso, flash_spi_sck>;
    using flash_spi_cfg = mcu::cfg::spi<flash_spi, flash_spi_params>;

    using oled_spi_params = zoal::periph::spi_params<apb2_clock_freq>;
    using oled_spi_mux = mcu::mux::spi<oled_spi, oled_miso, oled_mosi, oled_sck>;
    using oled_spi_cfg = mcu::cfg::spi<oled_spi, oled_spi_params>;

    using adc_params = zoal::periph::adc_params<>;
    using adc_cfg = mcu::cfg::adc<sensor_adc, adc_params>;

    using timer_params = zoal::periph::timer_params<
        //
        apb2_clock_freq,
        pwm_divider,
        pwm_period,
        zoal::periph::timer_mode::up>;
    using timer_cfg = mcu::cfg::timer<motor_pwm_timer, timer_params>;

    // Enable bus clock
    api::optimize<
        //
        tty_usart_mux::clock_on,
        tty_usart_cfg::clock_on,
        //
        i2c_mux::clock_on,
        i2c_cfg::clock_on,
        //
        flash_spi_mux::clock_on,
        flash_spi_cfg::clock_on,
        //
        oled_spi_mux::clock_on,
        oled_spi_cfg::clock_on,
        //
        adc_cfg::clock_on,
        //
        timer_cfg::clock_on,
        //
        mcu::port_a::clock_on_cas,
        mcu::port_b::clock_on_cas,
        mcu::port_c::clock_on_cas
        //
        >();

    // Disable peripherals before configuration
    api::optimize<api::disable<tty_usart, i2c, oled_spi, flash_spi, sensor_adc, motor_pwm_timer>>();

    api::optimize<
        //
        tty_usart_mux::connect,
        tty_usart_cfg::apply,
        //
        i2c_mux::connect,
        i2c_cfg::apply,
        //
        flash_spi_mux::connect,
        flash_spi_cfg::apply,
        //
        oled_spi_mux::connect,
        oled_spi_cfg::apply,
        //
        timer_cfg::apply,
        zoal::ct::type_list<
            //
            motor_pwm_timer::TIMERx_CR1::template cas<0, motor_pwm_timer::TIMERx_CR1_OPM>,
            motor_pwm_timer::TIMERx_DIER::template cas<0, motor_pwm_timer::TIMERx_DIER_UIE>>,
        //
        api::mode<zoal::gpio::pin_mode::output,
                  ///
                  flash_spi_cs,
                  oled_cs,
                  oled_ds,
                  oled_res,
                  motor_dir,
                  motor_en,
                  motor_step>,
        api::high<flash_spi_cs, oled_cs, motor_en, motor_step>,
        api::low<motor_dir>,
        api::mode<zoal::gpio::pin_mode::input_pull_up, encoder_pin_a, encoder_pin_b, encoder_pin_btn>
        //
        >();

    // Enable peripherals after configuration
    api::optimize<api::enable<tty_usart, i2c, oled_spi, flash_spi, sensor_adc>>();

    HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
    HAL_NVIC_SetPriority(TIM2_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    //    HAL_NVIC_SetPriority(ADC_IRQn, 8, 0);
    //    HAL_NVIC_EnableIRQ(ADC_IRQn);
    //
    //    sensor_adc::enable_interrupt();
    tty_usart::enable_rx();

    zoal::utils::interrupts::on();
}

void tty_tx_transport::send_byte(uint8_t value) {
    tty_tx_stream.send(value, portMAX_DELAY);
    tty_usart ::enable_tx();
}

void tty_tx_transport::send_data(const void *data, size_t size) {
    auto ptr = reinterpret_cast<const char *>(data);
    while (size > 0) {
        auto sent = tty_tx_stream.send(ptr, size, 0);
        tty_usart ::enable_tx();
        size -= sent;
        ptr += sent;
    }
}

void df_player_tx_transport::send_byte(uint8_t value) {
    player_tx_stream.send(value, portMAX_DELAY);
    tty_usart ::enable_tx();
}

void df_player_tx_transport::send_data(const void *data, size_t size) {
    auto ptr = reinterpret_cast<const char *>(data);
    while (size > 0) {
        auto sent = player_tx_stream.send(ptr, size, 0);
        tty_usart ::enable_tx();
        size -= sent;
        ptr += sent;
    }
}

extern "C" void USART1_IRQHandler() {
    tty_usart::tx_handler([](uint8_t &value) {
        //
        return tty_tx_stream.receive_isr(&value, 1) > 0;
    });
    tty_usart::rx_handler([](uint8_t byte) {
        tty_rx_stream.send_isr(byte);
        event_manager::set_isr(hardware_event_tty_rx);
    });
}

extern "C" void USART2_IRQHandler() {
    df_player_usart::tx_handler([](uint8_t &value) {
        //
        return player_tx_stream.receive_isr(&value, 1) > 0;
    });
    df_player_usart::rx_handler([](uint8_t byte) {
        player_rx_stream.send_isr(byte);
        event_manager::set_isr(hardware_event_player_rx);
    });
}

extern "C" void I2C1_EV_IRQHandler() {
    i2c::handle_request_irq(request, []() {
        //
        event_manager::set_isr(hardware_event_i2c);
    });
}

extern "C" void I2C1_ER_IRQHandler() {
    i2c::handle_request_irq(request, []() {
        //
        event_manager::set_isr(hardware_event_i2c);
    });
}

bartender_machine_v2 machine;

extern "C" void TIM2_IRQHandler(void) {
    motor_pwm_timer::TIMERx_SR::ref() &= ~motor_pwm_timer::TIMERx_SR_UIF;
    machine.handle_timer();
}

volatile uint16_t sensors_values[2];

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        machine.handle_adc();
    }
}

//extern "C" void ADC_IRQHandler(void) {
//    zoal::periph::adc_dispatcher<sensor_adc>::api.complete(sensor_adc::value());
//    event_manager::set_isr(hardware_event_adc);
//}
