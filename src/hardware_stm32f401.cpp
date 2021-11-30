#include "config.hpp"
#include "event_manager.hpp"
#include "hardware.hpp"

zoal::mem::reserve_mem<stream_buffer_type, 32> rx_stream_buffer(1);
zoal::mem::reserve_mem<stream_buffer_type, 32> tx_stream_buffer(1);

i2c_req_dispatcher_type i2c_req_dispatcher;
zoal::periph::i2c_request &request = i2c_req_dispatcher.request;
zoal::utils::i2c_scanner scanner;
oled_type screen;

bartender_machine_type bartender;
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

    using flash_params = zoal::periph::spi_params<apb2_clock_freq>;
    using flash_spi_mux = mcu::mux::spi<flash_spi, flash_spi_mosi, flash_spi_miso, flash_spi_sck>;
    using flash_spi_cfg = mcu::cfg::spi<flash_spi, flash_params>;

    using oled_params = zoal::periph::spi_params<apb2_clock_freq>;
    using oled_spi_mux = mcu::mux::spi<oled_spi, oled_mosi, oled_miso, oled_sck>;
    using oled_spi_cfg = mcu::cfg::spi<oled_spi, oled_params>;

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
        mcu::port_c::clock_on_cas
        //
        >();
    api::optimize<api::disable<tty_usart, i2c, oled_spi, flash_spi>>();

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
        api::mode<zoal::gpio::pin_mode::output, flash_spi_cs, oled_cs, oled_ds, oled_res>,
        api::high<flash_spi_cs, oled_cs>,
        api::mode<zoal::gpio::pin_mode::output, mcu::pc_13>>();
    api::optimize<api::enable<tty_usart, i2c, oled_spi, flash_spi>>();

    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

    tty_usart::enable_rx();

    zoal::utils::interrupts::on();
}

extern "C" void USART1_IRQHandler() {
    tty_usart::tx_handler([](uint8_t &value) {
        //
        return tx_stream_buffer.receive_isr(&value, 1) > 0;
    });
    tty_usart::rx_handler([](uint8_t byte) {
        rx_stream_buffer.send_isr(byte);
        event_manager::set_isr(hardware_event_tty_rx);
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
