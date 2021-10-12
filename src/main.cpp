#include "./df_player.hpp"
#include "./gui.hpp"
#include "./logo/ascii_logo.hpp"
#include "./parsers/command_machine.hpp"

#include <zoal/arch/avr/stream.hpp>

FUSES = {.low = 0xFF, .high = 0xD7, .extended = 0xFC};

constexpr uint8_t fps = 30;
constexpr uint32_t display_fresh_delay = 1000 / fps;
bool pending_refresh_frame = false;
char command_history[tty_terminal_str_size] = {0};

using scheduler_type = zoal::utils::function_scheduler<uint32_t, 8, void *>;
scheduler_type general_scheduler;

gui user_interface;

void scan_i2c() {
    tty_stream << "Scanning I2C devices..."
               << "\r\n";
    scanner.device_found = [](uint8_t addr) {
        tty_stream << "\033[2K\r"
                   << "i2c device: " << addr << "\r\n";
    };
    scanner.scan(i2c_req_dispatcher)([](int) {
        tty_stream << "\033[2K\r"
                   << "I2C scanning complete"
                   << "\r\n";
        terminal.sync();
    });
}

void vt100_callback(const zoal::misc::terminal_input *, const char *s, const char *e) {
    transport.send_data(s, e - s);
}

static void handle_v100(const zoal::misc::terminal_input *, zoal::misc::terminal_machine_event e) {
    switch (e) {
    case zoal::misc::terminal_machine_event::up_key:
        terminal.value(command_history);
        break;
    case zoal::misc::terminal_machine_event::down_key:
        terminal.value("");
        break;
    default:
        break;
    }
}

void command_callback(zoal::misc::command_machine *, command_type cmd, int argc, zoal::misc::cmd_arg *argv) {
    switch (argc) {
    case 0:
        send_command(cmd);
        break;
    case 1:
        send_command(cmd, (int)*argv);
        break;
    default:
        break;
    }
}

void input_callback(const zoal::misc::terminal_input *, const char *s, const char *e) {
    tty_stream << "\r\n";
    if (s < e) {
        auto src = s;
        auto dst = command_history;
        while (src < e) {
            *dst++ = *src++;
        }
        *dst = 0;

        zoal::misc::command_machine cm;
        cm.callback(command_callback);
        cm.run_machine(s, e, e);
    }
    terminal.sync();
}

void render_frame(void * = nullptr) {
    pending_refresh_frame = false;
    i2c_req_dispatcher.handle_until_finished();
    user_interface.render();
    screen.display(i2c_req_dispatcher)([](int) {});
}

void process_command(command &cmd) {
    auto type = cmd.type;

    switch (type) {
    case command_type::show_help:
        tty_stream << "\033[2K\r";
        tty_stream << zoal::io::progmem_str(help_msg);
        terminal.sync();
        break;
    case command_type::show_adc: {
        tty_stream << "\033[2K\r";
        tty_stream << "hall:\t" << hall_channel::read() << "\r\n";
        tty_stream << "ir:\t" << ir_channel::read() << "\r\n";
        terminal.sync();
        break;
    }
    case command_type::calibrate:
        player.enqueue_track(voice::calibration);
        bartender.calibrate();
        send_event(event_type::calibration_started);
        send_command(command_type::render_screen);
        break;
    case command_type::stop:
        bartender.stop_machine();
        break;
    case command_type::go:
        bartender.start();
        break;
    case command_type::scan_i2c:
        scan_i2c();
        break;
    case command_type::pump:
        bartender.pump(cmd.value);
        break;
    case command_type::valve:
        bartender.valve(cmd.value);
        break;
    case command_type::next_segment:
        bartender.next_segment();
        break;
    case command_type::render_screen:
        render_frame();
        break;
    case command_type::request_render_screen:
        if (!pending_refresh_frame) {
            pending_refresh_frame = true;
            general_scheduler.schedule(0, display_fresh_delay, render_frame);
        }
        break;
    case command_type::request_render_screen_500ms:
        general_scheduler.schedule(0, 500, render_frame);
        break;
    case command_type::play:
        player.enqueue_track(cmd.value);
        break;
    case command_type::logo:
        user_interface.current_screen(&user_interface.logo_screen_);
        break;
    case command_type::rotate:
        bartender.rotate(cmd.value);
        break;
    case command_type::settings: {
        auto &s = global_app_state.settings;
        auto &rs = global_app_state.settings.revolver_settings_[s.segments_];
        tty_stream << "\r\n";
        tty_stream << " Segments\t\t: " << s.segments_ << "\r\n";
        tty_stream << " Pump power\t\t: " << s.pump_power_ << "\r\n";
        tty_stream << " Hall rising threshold\t: " << s.hall_rising_threshold_ << "\r\n";
        tty_stream << " Hall falling threshold\t: " << s.hall_falling_threshold_ << "\r\n";
        tty_stream << " Current portion\t\t: " << s.current_portion_ << "\r\n";
        tty_stream << " Portion time\t\t: " << rs.portion_time_ << "\r\n";
        tty_stream << " Portion delay\t\t: " << rs.portion_delay_ << "\r\n";
        tty_stream << " IR min\t\t\t: " << rs.ir_min_value_ << "\r\n";
        tty_stream << " IR max\t\t\t: " << rs.ir_max_value_ << "\r\n";
        tty_stream << " Adjustment\t\t: " << rs.sector_adjustment_ << "\r\n";
        terminal.sync();
        break;
    }
    default:
        break;
    }
}

void process_event(event &e) {
    switch (e.type) {
    case event_type::calibration_finished:
        user_interface.current_screen(&user_interface.calibration_screen_);
        break;
    default:
        user_interface.process_event(e);
        bartender.process_event(e);
        break;
    }
}

void process_message() {
    message msg{};
    while (pop_message(msg)) {
        switch (msg.type) {
        case message_type::event:
            process_event(msg.e);
            break;
        case message_type::command:
            process_command(msg.c);
            break;
        }
    }
}

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

void process_encoder() {
    encoder.handle([](zoal::io::rotary_event e) {
        if (e == zoal::io::rotary_event::direction_1) {
            send_event(event_type::encoder_ccw);
        } else {
            send_event(event_type::encoder_cw);
        }
    });

    encoder_button.handle(milliseconds, [](zoal::io::button_event event) {
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

    terminal.vt100_feedback(&vt100_callback);
    terminal.input_callback(&input_callback);
    terminal.handle_v100(&handle_v100);
    terminal.greeting(terminal_greeting);
    terminal.clear();
    tty_stream << zoal::io::progmem_str(ascii_logo) << zoal::io::progmem_str(help_msg);
    terminal.sync();

    global_app_state.load_settings();

    send_command(command_type::render_screen);

    uint8_t events;
    while (true) {
        {
            zoal::utils::interrupts_off off;
            events = hardware_events;
            hardware_events = 0;
        }

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
                bartender.handle(milliseconds);
                general_scheduler.handle(milliseconds);
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
