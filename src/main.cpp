#include "./event_manager.hpp"
#include "./flash_manager.hpp"
#include "./gui.hpp"
#include "./hardware.hpp"
#include "./message_processor.hpp"
#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "stm32f4xx_hal.h"

[[noreturn]] void zoal_main_task(void *);
[[noreturn]] void zoal_scheduler_task(void *);
[[noreturn]] void zoal_machine_task(void *);
[[noreturn]] void zoal_adc_task(void *);

using task_type = zoal::freertos::task<zoal::freertos::freertos_allocation_type::static_mem>;
__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> main_task(zoal_main_task, "main");
__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> schedule_task(zoal_scheduler_task, "scheduler");
__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> machine_task(zoal_machine_task, "machine");
//__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> adc_task(zoal_adc_task, "adc");

extern "C" void SystemClock_Config(void);

uint32_t hall = 0, min_h = 0xFFFFFF, max_h = 0;
int skip = 0;
sector_detector sd;

float _err_measure = 0.8; // примерный шум измерений
float _q = 0.1; // скорость изменения значений 0.001-1, варьировать самому
float simpleKalman(float newVal) {
    float _kalman_gain, _current_estimate;
    static float _err_estimate = _err_measure;
    static float _last_estimate = 0;
    _kalman_gain = (float)_err_estimate / (_err_estimate + _err_measure);
    _current_estimate = _last_estimate + (float)_kalman_gain * (newVal - _last_estimate);
    _err_estimate = (1.0 - _kalman_gain) * _err_estimate + fabs(_last_estimate - _current_estimate) * _q;
    _last_estimate = _current_estimate;
    return _current_estimate;
}

int qqq = 0;

[[noreturn]] void zoal_adc_task(void *) {
    bool high = true;

    delay::ms(1000);
    motor_en::low();

    int dms = 1;
    for (;;) {
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sensors_values, 2);
        delay::ms(dms);

        auto result = sd.handle_v2(sensors_values[0]);
        if (result == detection_result::changed) {
            switch (sd.sector_state_) {
            case sector_state::unknown:
                tty_stream << "unknown: ";
                break;
            case sector_state::entering_sector:
                tty_stream << "entering_sector: ";
                break;
            case sector_state::sector:
                tty_stream << "sector: ";
                min_h = 0xFFFFFF;
                max_h = 0;
                skip = 2000;
                break;
            case sector_state::leaving_sector:
                tty_stream << "leaving_sector: ";
                break;
            }
        }

        if (skip > 0) {
            skip--;
        } else {
            if (high) {
                motor_step::low();
                high = false;
            } else {
                motor_step::high();
                high = true;
            }
        }
    }
}

[[noreturn]] void zoal_machine_task(void *) {
    machine.main_task();
}

[[noreturn]] void zoal_scheduler_task(void *) {
    for (;;) {
        delay::ms(1);
        auto ms = counter::now();

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

static void flash_callback(flash_command_result r, uint32_t address, uint32_t size) {
    switch (r) {
    case flash_command_result::chip_erased:
        tty_stream << "Chip erased"
                   << "\r\n";
        break;
    case flash_command_result::sector_erased:
        tty_stream << "Sector " << address << " erased"
                   << "\r\n";
        break;
    case flash_command_result::page_programed:
        tty_stream << "Written " << size << " bytes at " << address << "\r\n";
        break;
    case flash_command_result::finished:
        tty_stream << "Finished\r\n";
        terminal.sync();
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
                fm.process_command(rx_buffer, size);
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
    fm.read_records();
    fm.status_callback = flash_callback;

    user_interface.push_screen(&user_interface.menu_screen_);
    user_interface.push_screen(&user_interface.animation_screen_);
    user_interface.animation_screen_.animation(2);
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
    MX_DMA_Init();
    MX_ADC1_Init();

    initialize_hardware();
    vTaskStartScheduler();
    return 0;
}
