#include "event_manager.hpp"
#include "gpio.h"
#include "hardware.hpp"
#include "logo/ascii_logo.hpp"
#include "message_processor.hpp"
#include "stm32f4xx_hal.h"

[[noreturn]] void zoal_main_task(void *);

void process_terminal_rx();
using task_type = zoal::freertos::task<zoal::freertos::freertos_allocation_type::static_mem>;

__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> main_task(zoal_main_task, "main");

extern "C" void SystemClock_Config(void);

constexpr EventBits_t i2c_event = 1 << 0;
constexpr EventBits_t usart_event = 1 << 1;
constexpr EventBits_t all_hardware_events = i2c_event | usart_event;

void init_hardware() {
    using api = zoal::gpio::api;
    using tty_usart_params = zoal::periph::usart_115200<apb2_clock_freq>;
    using tty_usart_mux = mcu::mux::usart<tty_usart, tty_usart_rx, tty_usart_tx>;
    using tty_usart_cfg = mcu::cfg::usart<tty_usart, tty_usart_params>;

    api::optimize<
        //
        tty_usart_mux::clock_on,
        tty_usart_cfg::clock_on,
        mcu::port_c::clock_on_cas
        //
        >();
    api::optimize<api::disable<tty_usart>>();

    api::optimize<
        //
        tty_usart_mux::connect,
        tty_usart_cfg::apply,
        //
        api::mode<zoal::gpio::pin_mode::output, mcu::pc_13>>();
    api::optimize<api::enable<tty_usart>>();

    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    tty_usart::enable_rx();

    zoal::utils::interrupts::on();
}

[[noreturn]] void zoal_main_task(void *) {
    initialize_terminal();

    for (;;) {
        auto events = event_manager::get();
        if (events & hardware_event_tty_rx) {
            process_terminal_rx();
        }
        if (events & hardware_event_i2c) {
            i2c_req_dispatcher.handle();
        }
        if (events & hardware_event_msg) {
            process_message_();
        }
    }
}

void process_terminal_rx() {
    uint8_t rx_buffer[8];
    size_t size;
    do {
        size = rx_stream_buffer.receive(rx_buffer, sizeof(rx_buffer), 0);
        if (size != 0) {
            terminal.push_and_scan(rx_buffer, size);
        }
    } while (size == sizeof(rx_buffer));
}

int main() {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();

    initialize_hardware();
    vTaskStartScheduler();
    return 0;
}
