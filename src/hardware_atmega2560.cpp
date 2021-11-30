#include "event_manager.hpp"
#include "hardware.hpp"

volatile uint32_t milliseconds;
i2c_req_dispatcher_type i2c_req_dispatcher;
zoal::periph::i2c_request &request = i2c_req_dispatcher.request;
zoal::utils::i2c_scanner scanner;
oled_type screen;

bartender_machine_type bartender;
encoder_type encoder;
start_button_type start_button;
stop_button_type stop_button;

encoder_button_type encoder_button;
zoal::data::ring_buffer<uint8_t, df_player_rx_buffer_size> df_player_rx_buffer;
df_player player;

void initialize_hardware() {
    using namespace zoal::gpio;

    // Power on modules
    api::optimize<api::clock_on<tty_usart, df_player_usart, i2c, timer, pump_pwm_timer, adc>>();

    // Disable all modules before applying settings
    api::optimize<api::disable<tty_usart, df_player_usart, i2c, timer, pump_pwm_timer, adc>>();
    api::optimize<
        //
        mcu::mux::usart<tty_usart, tty_usart_rx, tty_usart_tx>::connect,
        mcu::cfg::usart<tty_usart, tty_usart_cfg>::apply,
        //
        mcu::mux::usart<df_player_usart, df_player_usart_rx, df_player_usart_tx>::connect,
        mcu::cfg::usart<df_player_usart, df_player_usart_cfg>::apply,
        //
        mcu::cfg::adc<adc, adc_cfg>::apply,
        //
        mcu::mux::i2c<i2c, i2c_sda, i2c_clk>::connect,
        mcu::cfg::i2c<i2c, i2c_cfg>::apply,
        //
        mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply,
        mcu::irq::timer<timer>::enable_overflow_interrupt,
        //
        mcu::cfg::timer<pump_pwm_timer, zoal::periph::timer_mode::up, 1, 1, 0xFF>::apply,
        //
        stepper_type::gpio_cfg,
        //
        api::mode<zoal::gpio::pin_mode::input, encoder_pin_a, encoder_pin_b>,
        api::mode<zoal::gpio::pin_mode::input_pull_up, encoder_pin_btn, start_signal, stop_signal>,
        api::mode<zoal::gpio::pin_mode::output, encoder_pin_vcc, encoder_pin_gnd, pump_signal, valve_signal>,
        api::low<pump_signal, valve_signal, encoder_pin_gnd>,
        api::high<encoder_pin_vcc>
        //
        >();

    // Enable system interrupts
    zoal::utils::interrupts::on();

    // Enable all modules
    api::optimize<api::enable<tty_usart, df_player_usart, timer, pump_pwm_timer, adc, i2c>>();

    adc::enable();
}

void initialize_i2c_devices() {
    screen.init(i2c_req_dispatcher)([](int) {});
    i2c_req_dispatcher.handle_until_finished();
}

#ifdef __AVR_ARCH__

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

ISR(TIMER0_OVF_vect) {
    milliseconds += overflow_to_tick::step();
    event_manager::set(hardware_event_tick);
}

ISR(TWI_vect) {
    i2c::handle_request_irq(request);
    event_manager::set(hardware_event_i2c);
}

ISR(USART0_RX_vect) {
    tty_usart::rx_handler<>([](uint8_t value) { tty_rx_buffer.push_back(value); });
    event_manager::set(hardware_event_tty_rx);
}

ISR(USART0_UDRE_vect) {
    tty_usart::tx_handler([](uint8_t &value) { return tty_tx_transport::tx_buffer.pop_front(value); });
}

ISR(USART1_RX_vect) {
    event_manager::set(hardware_event_player_rx);
    df_player_usart::rx_handler<>([](uint8_t value) { df_player_rx_buffer.push_back(value); });
}

ISR(USART1_UDRE_vect) {
    df_player_usart::tx_handler([](uint8_t &value) { return df_player_tx_transport::tx_buffer.pop_front(value); });
}

#pragma clang diagnostic pop

#endif
