#include "./df_player.hpp"
#include "./event_manager.hpp"
#include "./gui.hpp"
#include "./hardware.hpp"
#include "./message_processor.hpp"

FUSES = {.low = 0xFF, .high = 0xD7, .extended = 0xFC};

void process_terminal_rx() {
    while (true) {
        uint8_t rx_byte;
        bool result;
        {
            zoal::utils::interrupts_off scope_off;
            result = tty_rx_buffer.pop_front(rx_byte);
        }
        if (!result) {
            return;
        }
        terminal.push_and_scan(&rx_byte, 1);
    }
}

void process_player_rx() {
    while (true) {
        uint8_t rx_byte;
        bool result;
        {
            zoal::utils::interrupts_off scope_off;
            result = df_player_rx_buffer.pop_front(rx_byte);
        }
        if (!result) {
            return;
        }

        player.push_byte(rx_byte);
    }
}

void process_buttons() {
    auto ms = counter::now();
    start_button.handle(ms, [](zoal::io::button_event e) {
        if (e == zoal::io::button_event::press) {
            send_command(command_type::go);
        }
    });
    stop_button.handle(ms, [](zoal::io::button_event e) {
        if (e == zoal::io::button_event::press) {
            send_command(command_type::stop);
        }
    });
}

void process_encoder() {
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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main() {
    initialize_hardware();
    initialize_i2c_devices();
    initialize_terminal();

    global_app_state.load_settings();
    user_interface.current_screen(&user_interface.logo_screen_);

    send_command(command_type::render_screen);

    while (true) {
        uint8_t events = event_manager::get();
        if (events) {
            if (events & hardware_event_player_rx) {
                process_player_rx();
            }
            if (events & hardware_event_tty_rx) {
                process_terminal_rx();
            }
            if (events & hardware_event_i2c) {
                i2c_req_dispatcher.handle();
            }
            if (events & hardware_event_tick) {
                auto ms = counter::now();
                process_buttons();
                bartender.handle(ms);
                general_scheduler.handle(ms);
            }
            if (events & hardware_event_msg) {
                process_message();
            }
        }

        process_encoder();
    }

    return 0;
}

#pragma clang diagnostic pop
