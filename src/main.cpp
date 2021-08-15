#include "./ascii_logo.hpp"
#include "./command.hpp"
#include "./df_player.hpp"
#include "./ecafe_logo.hpp"
#include "./gui.hpp"
#include "./voice.hpp"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <zoal/arch/avr/stream.hpp>

FUSES = {.low = 0xFF, .high = 0xDF, .extended = 0xFC};

constexpr uint8_t fps = 20;
constexpr uint32_t display_fresh_delay = 1000 / fps;

using scheduler_type = zoal::utils::function_scheduler<uint32_t, 8, void *>;
scheduler_type general_scheduler;

gui user_interface{global_app_state};

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

void cmd_select_callback(zoal::misc::command_line_machine *p, zoal::misc::command_line_event e) {
    static const char help_cmd[] PROGMEM = "help";
    static const char i2c_scan_cmd[] PROGMEM = "i2c-scan";
    static const char calibrate_cmd[] PROGMEM = "calibrate";
    static const char next_cmd[] PROGMEM = "next";
    static const char go_cmd[] PROGMEM = "go";
    static const char adc_cmd[] PROGMEM = "adc";
    static const char pump_cmd[] PROGMEM = "pump";
    static const char next_item[] PROGMEM = "next-item";
    static const char prev_item[] PROGMEM = "prev-item";
    static const char exec_item[] PROGMEM = "exec-item";
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
        bartender.stop_machine();
        bartender.go_to_next_segment();
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

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(next_item), ts, te)) {
        send_command(command_type::next_item);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(prev_item), ts, te)) {
        send_command(command_type::prev_item);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(exec_item), ts, te)) {
        send_command(command_type::exec_item);
    }

    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(play1_cmd), ts, te)) {
        command cmd(command_type::play);
        cmd.value = 1;
        send_command(cmd);
    }
    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(play2_cmd), ts, te)) {
        command cmd(command_type::play);
        cmd.value = 2;
        send_command(cmd);
    }
    if (cmp_progmem_str_token(zoal::io::progmem_str_iter(play3_cmd), ts, te)) {
        command cmd(command_type::play);
        cmd.value = 3;
        send_command(cmd);
    }
}

void input_callback(const zoal::misc::terminal_input *, const char *s, const char *e) {
    command_line_parser cmd_parser(nullptr, 0);
    cmd_parser.callback(cmd_select_callback);
    cmd_parser.scan(s, e, e);
    terminal.sync();
}

void send_render_cmd(void *) {
    send_command(command_type::request_render_frame);
}

void process_command() {
    command cmd;
    if (!pop_command(cmd)) {
        return;
    }

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
        send_command(command_type::request_render_frame);
        break;
    case command_type::go:
        bartender.portions_left_ = bartender.total_segments_;
        bartender.go_to_next_segment();
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
        bartender.go_to_next_segment();
        break;
    case command_type::request_render_frame:
        i2c_req_dispatcher.handle_until_finished();
        user_interface.render();
        screen.display(i2c_req_dispatcher)([](int) {});
        break;
    case command_type::request_next_render_frame:
        general_scheduler.schedule(0, display_fresh_delay, send_render_cmd);
        break;
    case command_type::next_item:
        user_interface.next_item();
        break;
    case command_type::prev_item:
        user_interface.prev_item();
        break;
    case command_type::exec_item:
        user_interface.exec_item();
        break;
    case command_type::clear_error:
        global_app_state.flags = app_state_flags_idle;
        send_command(command_type::request_render_frame);
        break;
    case command_type::play:
        player.play(cmd.value);
        break;
    case command_type::logo:
        memcpy_P(screen.buffer.canvas, ecafe_logo, sizeof(screen.buffer.canvas));
        screen.display(i2c_req_dispatcher)([](int) {});
        break;
    default:
        break;
    }
};

void process_terminal_rx() {
    uint8_t rx_byte = 0;
    bool result;
    {
        zoal::utils::interrupts_off scope_off;
        result = tty_rx_buffer.pop_front(rx_byte);
    }

    if (result) {
        terminal.push(&rx_byte, 1);
    }
}

void process_player_rx() {
    uint8_t rx_byte = 0;
    bool result;
    {
        zoal::utils::interrupts_off scope_off;
        result = df_player_rx_buffer.pop_front(rx_byte);
    }

    if (result) {
        player.push_byte(rx_byte);
    }
}

void process_encoder() {
    encoder.handle([](zoal::io::rotary_event e) {
        if (e == zoal::io::rotary_event::direction_1) {
            send_command(command_type::prev_item);
        } else {
            send_command(command_type::next_item);
        }
    });

    encoder_button.handle(milliseconds, [](zoal::io::button_event event) {
        if (event == zoal::io::button_event::press) {
            send_command(command_type::exec_item);
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

    player.callback_.assign([](uint8_t cmd, uint16_t param) {
        if (cmd == df_player::cmd_init_params && param == 2) {
            player.callback_.reset();
            player.play(voice::hello);
        }
    });
    player.reset();

    send_command(command_type::logo);
    while (true) {
        process_terminal_rx();
        process_command();
        process_encoder();

        bartender.handle();
        i2c_req_dispatcher.handle();
        general_scheduler.handle(milliseconds);
        process_player_rx();
    }

    return 0;
}

#pragma clang diagnostic pop
