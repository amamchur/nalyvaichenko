#include "./df_player.hpp"
#include "./gui.hpp"
#include "./hardware.hpp"
#include "./logo/ascii_logo.hpp"
#include "./voice.hpp"
#include "./volatile_data.hpp"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <zoal/arch/avr/stream.hpp>
#include <zoal/utils/scheduler.hpp>

FUSES = {.low = 0xFF, .high = 0xD7, .extended = 0xFC};

constexpr uint8_t fps = 30;
constexpr uint32_t display_fresh_delay = 1000 / fps;
volatile bool pending_refresh_frame = false;

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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

void cmd_select_callback(zoal::misc::command_line_machine *p, zoal::misc::command_line_event e) {
    static const char help_cmd[] PROGMEM = "help";
    static const char i2c_scan_cmd[] PROGMEM = "i2c-scan";
    static const char calibrate_cmd[] PROGMEM = "calibrate";
    static const char next_cmd[] PROGMEM = "next";
    static const char go_cmd[] PROGMEM = "go";
    static const char adc_cmd[] PROGMEM = "adc";
    static const char pump_cmd[] PROGMEM = "pump";
    static const char enc_cw[] PROGMEM = "enc-cc";
    static const char enc_ccw[] PROGMEM = "enc-ccw";
    static const char enc_press[] PROGMEM = "enc-press";
    static const char play1_cmd[] PROGMEM = "play1";
    static const char play2_cmd[] PROGMEM = "play2";
    static const char play3_cmd[] PROGMEM = "play3";

    if (e == zoal::misc::command_line_event::line_end) {
        return;
    }

    auto ts = p->token_start();
    auto te = p->token_end();
    p->callback(&command_line_parser::empty_callback);

    tty_stream << "\r\n";

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(help_cmd), ts, te)) {
        send_command(command_type::show_help);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(i2c_scan_cmd), ts, te)) {
        send_command(command_type::scan_i2c);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(calibrate_cmd), ts, te)) {
        send_command(command_type::calibrate);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(next_cmd), ts, te)) {
        send_command(command_type::next_segment);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(go_cmd), ts, te)) {
        send_command(command_type::go);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(pump_cmd), ts, te)) {
        send_command(command_type::pump);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(adc_cmd), ts, te)) {
        send_command(command_type::show_adc);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(enc_cw), ts, te)) {
        send_event(event_type::encoder_cw);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(enc_ccw), ts, te)) {
        send_event(event_type::encoder_ccw);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(enc_press), ts, te)) {
        send_event(event_type::encoder_press);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(play1_cmd), ts, te)) {
        command cmd{};
        cmd.type = command_type::play;
        cmd.value = 1;
        send_command(cmd);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(play2_cmd), ts, te)) {
        command cmd{};
        cmd.type = command_type::play;
        cmd.value = 2;
        send_command(cmd);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(play3_cmd), ts, te)) {
        command cmd{};
        cmd.type = command_type::play;
        cmd.value = 3;
        send_command(cmd);
    }
}

#pragma clang diagnostic pop

void input_callback(const zoal::misc::terminal_input *, const char *s, const char *e) {
    command_line_parser cmd_parser(nullptr, 0);
    cmd_parser.callback(cmd_select_callback);
    cmd_parser.scan(s, e, e);
    terminal.sync();
}

void render_frame(void *ptr = nullptr) {
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
        player.play(voice::calibration);
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
        pump_signal::mode<zoal::gpio::pin_mode::output>();
        pump_signal::_1();
        delay::ms(500);
        pump_signal::_0();
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
        player.play(cmd.value);
        break;
    case command_type::logo:
        user_interface.current_screen(&user_interface.logo_screen_);
        break;
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
        break;
    }
    user_interface.process_event(e);
    bartender.process_event(e);
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
        terminal.push(&rx_byte, 1);
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
    terminal.greeting(terminal_greeting);
    terminal.clear();
    tty_stream << zoal::io::progmem_str(ascii_logo) << zoal::io::progmem_str(help_msg);
    terminal.sync();

    player.callback_ = [](uint8_t cmd, uint16_t param) {
        if (cmd == df_player::cmd_init_params && param == 2) {
            player.callback_.reset();
            player.play(voice::hello);
        }
    };
    player.reset();

    global_app_state.load_settings();

    send_command(command_type::render_screen);

    int events;
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
