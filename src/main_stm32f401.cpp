#include "./event_manager.hpp"
#include "./gui.hpp"
#include "./hardware.hpp"
#include "./logo/ecafe_logo.hpp"
#include "./logo/test_logo.hpp"
#include "./message_processor.hpp"
#include "./parsers/flash_machine.hpp"
#include "gpio.h"
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

zoal::misc::flash_machine fm;

void fm_callback(zoal::misc::flash_machine *m, const zoal::misc::flash_cmd &cmd) {
    switch (cmd.type) {
    case zoal::misc::flash_cmd_type::erase_chip:
        w25q32::chip_erase();
        tty_stream << "Chip erased"
                   << "\r\n";
        break;
    case zoal::misc::flash_cmd_type::erase_sector:
        w25q32::sector_erase(cmd.address);
        tty_stream << "Sector " << cmd.address << " erased"
                   << "\r\n";
        break;
    case zoal::misc::flash_cmd_type::prog_mem:
        w25q32::page_program(cmd.address, cmd.data, cmd.size);
        break;
    case zoal::misc::flash_cmd_type::finish:
        tty_stream << "\r\nDone\r\n";
        terminal.sync();
        global_app_state.flash_editor = false;
        break;
    default:
        break;
    }
}

void process_terminal_rx() {
    uint8_t rx_buffer[8];
    size_t size;
    do {
        size = tty_rx_stream.receive(rx_buffer, sizeof(rx_buffer), 0);
        if (size != 0) {
            if (global_app_state.flash_editor) {
                fm.run_machine((const char *)rx_buffer, (const char *)rx_buffer + size, nullptr);
            } else {
                terminal.push_and_scan(rx_buffer, size);
            }
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
    fm.callback(fm_callback);

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
