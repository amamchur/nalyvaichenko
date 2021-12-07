#ifndef NALYVAICHENKO_MESSAGE_HPP
#define NALYVAICHENKO_MESSAGE_HPP

#include <zoal/data/ring_buffer.hpp>

enum class event_type {
    //
    none,
    settings_changed,
    //
    encoder_cw,
    encoder_ccw,
    encoder_press,
    //
    machine_stop,
    calibration_started,
    calibration_finished,
};

class event {
public:
    event_type type;
    int value;
};

class event_handler {
public:
    virtual void process_event(event &e) = 0;
};

enum class command_type {
    //
    none,
    stop,
    go,
    show_help,
    show_adc,
    pump,
    calibrate,
    scan_i2c,
    next_segment,
    render_screen,
    request_render_screen,
    request_render_screen_500ms,
    play,
    logo,
    valve,
    enc,
    rotate,
    settings,
    press,

    flash,
    read_image_from_flash,

    enable_motor,
    disable_motor,
    direction_a,
    direction_b,
    rpm
};

class command {
public:
    command_type type;
    int value;
};

enum class message_type {
    //
    event,
    command
};

class message {
public:
    message_type type;
    union {
        event e;
        command c;
    };
};

void send_message(message &msg);
void send_message_isr(message &msg);
void send_event(event_type type);
void send_event(const event &cmd);
void send_command(command_type type);
void send_command(command_type type, int value);
void send_command(const command &cmd);
void send_command_isr(command_type type);
bool pop_message(message &msg);

#endif
