
#include "hardware.hpp"

#include "app_state.hpp"
#include "df_player.hpp"
#include "volatile_data.hpp"

i2c_req_dispatcher_type i2c_req_dispatcher;
zoal::periph::i2c_request &request = i2c_req_dispatcher.request;
zoal::utils::i2c_scanner scanner;
oled_type screen;

bartender_machine_type bartender;
encoder_type encoder;
encoder_button_type encoder_button;
df_player player;


void initialize_hardware() {
    using namespace zoal::gpio;
    using tty_usart_cfg = zoal::periph::usart_115200<F_CPU>;
    using df_player_usart_cfg = zoal::periph::usart_9600<F_CPU>;
    using adc_cfg = zoal::periph::adc_config<>;
    using i2c_cfg = zoal::periph::i2c_fast_mode<F_CPU>;

    // Power on modules
    api::optimize<api::clock_on<tty_usart, i2c, timer>>();

    // Disable all modules before applying settings
    api::optimize<api::disable<tty_usart, df_player_usart, i2c, timer, adc>>();
    api::optimize<
        //
        mcu::mux::usart<tty_usart, mcu::pe_00, mcu::pe_01>::connect,
        mcu::cfg::usart<tty_usart, tty_usart_cfg>::apply,
        //
        mcu::mux::usart<df_player_usart, mcu::pd_02, mcu::pd_03>::connect,
        mcu::cfg::usart<df_player_usart, df_player_usart_cfg>::apply,
        //
        mcu::mux::adc<adc, pcb::ard_a00>::connect,
        mcu::cfg::adc<adc, adc_cfg>::apply,
        //
        mcu::mux::i2c<i2c, mcu::pd_01, mcu::pd_00>::connect,
        mcu::cfg::i2c<i2c, i2c_cfg>::apply,
        //
        mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply,
        //
        mcu::irq::timer<timer>::enable_overflow_interrupt,
        //
        stepper_type::gpio_cfg,
        //
        api::mode<zoal::gpio::pin_mode::input, encoder_pin_a, encoder_pin_b>,
        api::mode<zoal::gpio::pin_mode::input_pull_up, encoder_pin_btn>,
        api::mode<zoal::gpio::pin_mode::output, encoder_pin_vcc, encoder_pin_gnd, pump_signal, valve_signal>,
        api::low<pump_signal, valve_signal, encoder_pin_gnd>,
        api::high<encoder_pin_vcc>
        //
        >();

    // Enable system interrupts
    zoal::utils::interrupts::on();

    // Enable all modules
    api::optimize<api::enable<tty_usart, df_player_usart, timer, adc, i2c>>();

    adc::enable();
}

void initialize_i2c_devices() {
    screen.init(i2c_req_dispatcher)([](int) {});
    i2c_req_dispatcher.handle_until_finished();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

ISR(TIMER0_OVF_vect) {
    hardware_events |= hardware_event_tick;
    milliseconds += overflow_to_tick::step();
}

ISR(TWI_vect) {
    hardware_events |= hardware_event_i2c;
    i2c::handle_request_irq(request);
}

#pragma clang diagnostic pop
