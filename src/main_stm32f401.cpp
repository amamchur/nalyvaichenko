#include "event_manager.hpp"
#include "gpio.h"
#include "gui.hpp"
#include "hardware.hpp"
#include "message_processor.hpp"
#include "stm32f4xx_hal.h"

[[noreturn]] void zoal_main_task(void *);
[[noreturn]] void zoal_scheduler_task(void *);

void process_terminal_rx();
using task_type = zoal::freertos::task<zoal::freertos::freertos_allocation_type::static_mem>;

__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> main_task(zoal_main_task, "main");
__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> schedule_task(zoal_scheduler_task, "scheduler");

extern "C" void SystemClock_Config(void);

constexpr EventBits_t i2c_event = 1 << 0;
constexpr EventBits_t usart_event = 1 << 1;
constexpr EventBits_t all_hardware_events = i2c_event | usart_event;

[[noreturn]] void zoal_scheduler_task(void *) {
    for (;;) {
        vTaskDelay(1);
        auto ms = counter::now();
        bartender.handle(ms);
        general_scheduler.handle(ms);
    }
}

[[noreturn]] void zoal_main_task(void *) {
    initialize_terminal();

    screen.init();

    global_app_state.load_settings();
    user_interface.current_screen(&user_interface.logo_screen_);

    send_command(command_type::render_screen);

    for (;;) {
        auto events = event_manager::get();
        if (events & hardware_event_tty_rx) {
            process_terminal_rx();
        }
        if (events & hardware_event_i2c) {
            i2c_req_dispatcher.handle();
        }
        if (events & hardware_event_msg) {
            process_message();
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
