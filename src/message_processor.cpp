#include "message_processor.hpp"

#include "config.hpp"
#include "flash_manager.hpp"
#include "gui.hpp"
#include "hardware.hpp"
#include "tty_terminal.hpp"

constexpr uint8_t fps = 30;
constexpr uint32_t display_fresh_delay = 1000 / fps;
constexpr int refresh_frame_id = 1000;
bool pending_refresh_frame = false;

static void scan_i2c() {
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

static void render_frame(void * = nullptr) {
    pending_refresh_frame = false;
    user_interface.render();
    screen.display();
}

static void process_command(command &cmd) {
    auto type = cmd.type;

    switch (type) {
    case command_type::show_help:
        tty_stream << "\033[2K\r";
        tty_stream << help_msg;
        terminal.sync();
        break;
    case command_type::show_adc: {
        tty_stream << "\033[2K\r";
        tty_stream << "hall:\t" << sensors_values[0] << "\r\n";
        tty_stream << "ir:\t" << sensors_values[1] << "\r\n";
        terminal.sync();
        break;
    }
    case command_type::stop:
        machine.stop_machine();
        break;
    case command_type::go:
        machine.go();
        break;
    case command_type::scan_i2c:
        scan_i2c();
        break;
    case command_type::pump:
        machine.pump(cmd.value);
        break;
    case command_type::valve:
        machine.valve(cmd.value);
        break;
    case command_type::next_segment:
        machine.next_segment();
        break;
    case command_type::render_screen:
        render_frame();
        break;
    case command_type::request_render_screen:
        if (!pending_refresh_frame) {
            pending_refresh_frame = true;
            general_scheduler.schedule(refresh_frame_id, display_fresh_delay, render_frame);
        }
        break;
    case command_type::request_render_screen_ms:
        if (!pending_refresh_frame) {
            general_scheduler.remove(refresh_frame_id);
            general_scheduler.schedule(refresh_frame_id, cmd.value, render_frame);
        }
        break;
    case command_type::request_render_screen_500ms:
        general_scheduler.schedule(refresh_frame_id, 500, render_frame);
        break;
    case command_type::play:
        player.enqueue_track(cmd.value);
        break;
    case command_type::settings: {
        auto &s = global_app_state.settings;
        auto &rs = s.revolver_settings_[s.segments_];
        auto &ps = s.portion_settings_[s.current_portion_];
        tty_stream << "\r\n";
        tty_stream << " Segments\t\t: " << s.segments_ << "\r\n";
        tty_stream << " Pump power\t\t: " << s.pump_power_ << "\r\n";
        tty_stream << " Hall rising threshold\t: " << s.hall_rising_threshold_ << "\r\n";
        tty_stream << " Hall falling threshold\t: " << s.hall_falling_threshold_ << "\r\n";
        tty_stream << " Current portion\t: " << s.current_portion_ << "\r\n";
        tty_stream << " Portion time\t\t: " << ps.time_ << "\r\n";
        tty_stream << " Portion mg\t\t: " << ps.mg_ << "\r\n";
        tty_stream << " IR min\t\t\t: " << rs.ir_min_value_ << "\r\n";
        tty_stream << " IR max\t\t\t: " << rs.ir_max_value_ << "\r\n";
        tty_stream << " Adjustment\t\t: " << rs.sector_adjustment_ << "\r\n";
        terminal.sync();
        break;
    }
    case command_type::enc: {
        auto count = abs(cmd.value);
        auto e = cmd.value > 0 ? event_type::encoder_cw : event_type::encoder_ccw;
        for (int i = 0; i < count; i++) {
            send_event(e);
        }
        break;
    }
    case command_type::press:
        send_event(event_type::encoder_press);
        break;
    case command_type::flash:
        global_app_state.flash_editor = true;
        tty_stream << "\r\n";
        tty_stream << "Flash Editor\r\n";
        break;
    case command_type::motor_enable:
        //        machine.motor_test();
        motor_enable::on();
        break;
    case command_type::motor_disable:
        motor_enable::off();
        break;
    case command_type::motor_direction_cw:
        motor_dir_cw::on();
        break;
    case command_type::motor_direction_ccw:
        motor_dir_cw::off();
        break;
    case command_type::motor_rpm: {
        machine.rpm(cmd.value);
        terminal.sync();
        break;
    }
    case command_type::motor_accel: {
        machine.acceleration(static_cast<float>(cmd.value));
        terminal.sync();
        break;
    }
    case command_type::motor_step:
        machine.rotate(static_cast<float>(cmd.value));
        terminal.sync();
        break;
    case command_type::motor_deg:
        machine.rotate(steps_per_revolution / 360.0f * static_cast<float>(cmd.value));
        terminal.sync();
        break;
    case command_type::motor_info:
        tty_stream << "\r\n";
        tty_stream << " Acceleration\t: " << machine.acceleration() << " steps/s^2\r\n";
        tty_stream << " Max speed\t: " << machine.speed() << " steps/s\r\n";
        terminal.sync();
        break;

    case command_type::ui_anim:
        user_interface.animation_screen_.animation(cmd.value);
        user_interface.push_screen(&user_interface.animation_screen_);
        send_command(command_type::render_screen);
        break;
    case command_type::ui_image:
        fm.read_frame(cmd.value, 0, &screen.buffer.canvas, sizeof(screen.buffer.canvas));
        screen.display();
        break;

    case command_type::ui_logo:
        user_interface.push_screen(&user_interface.logo_screen_);
        break;

    case command_type::df_volume_read:
        player.volume();
        break;
    case command_type::df_volume_write:
        player.volume(cmd.value);
        break;
    case command_type::df_reset:
        player.reset();
        break;
    case command_type::df_status:
        player.status();
        break;
    default:
        break;
    }
}

static void process_event(event &e) {
    switch (e.type) {
    case event_type::calibration_finished:
        user_interface.push_screen(&user_interface.calibration_screen_);
        break;
    default:
        user_interface.process_event(e);
        machine.process_event(e);
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
