#include "event_manager.hpp"
#include "gpio.h"
#include "gui.hpp"
#include "hardware.hpp"
#include "message_processor.hpp"
#include "stm32f4xx_hal.h"

[[noreturn]] void zoal_main_task(void *);
[[noreturn]] void zoal_scheduler_task(void *);

using task_type = zoal::freertos::task<zoal::freertos::freertos_allocation_type::static_mem>;
__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> main_task(zoal_main_task, "main");
__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> schedule_task(zoal_scheduler_task, "scheduler");

extern "C" void SystemClock_Config(void);

[[noreturn]] void zoal_scheduler_task(void *) {
    for (;;) {
        vTaskDelay(1);
        auto ms = counter::now();
        bartender.handle(ms);
        general_scheduler.handle(ms);
        encoder.handle([](zoal::io::rotary_event e) {
            if (e == zoal::io::rotary_event::direction_1) {
                send_event(event_type::encoder_ccw);
            } else {
                send_event(event_type::encoder_cw);
            }
        });

        encoder_button.handle(counter::now(), [](zoal::io::button_event event) {
            if (event == zoal::io::button_event::press) {
                send_event(event_type::encoder_press);
            }
        });
    }
}

void process_terminal_rx() {
    uint8_t rx_buffer[8];
    size_t size;
    do {
        size = tty_rx_stream.receive(rx_buffer, sizeof(rx_buffer), 0);
        if (size != 0) {
            terminal.push_and_scan(rx_buffer, size);
        }
    } while (size == sizeof(rx_buffer));
}

void process_player_rx() {
    uint8_t rx_buffer[8];
    size_t size;
    do {
        size = player_rx_stream.receive(rx_buffer, sizeof(rx_buffer), 0);
        for (int i = 0; i < size; i++) {
            player.push_byte(rx_buffer[i]);
        }
    } while (size == sizeof(rx_buffer));
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
        if (events & hardware_event_player_rx) {
            process_player_rx();
        }
        if (events & hardware_event_i2c) {
            i2c_req_dispatcher.handle();
        }
        if (events & hardware_event_msg) {
            process_message();
        }
    }
}

int main() {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();

    initialize_hardware();
    vTaskStartScheduler();
    return 0;
}
