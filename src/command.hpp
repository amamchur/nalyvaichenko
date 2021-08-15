//
// Created by andrii on 03.07.21.
//

#ifndef NALYVAICHENKO_COMMAND_HPP
#define NALYVAICHENKO_COMMAND_HPP

#include <zoal/data/ring_buffer.hpp>

enum class command_type {
    //
    none,
    show_help,
    show_adc,
    pump,
    calibrate,
    scan_i2c,
    next_segment,
    request_render_frame,
    request_next_render_frame,
    next_item,
    prev_item,
    exec_item,
    clear_error,
    play
};

class command {
public:
    command() = default;

    explicit command(command_type t)
        : type(t) {}

    command_type type{command_type::none};
};

void send_command(command_type type);
void send_command(const command &cmd);
bool pop_command(command &cmd);

extern zoal::data::ring_buffer<command, 4> command_queue;

#endif //NALYVAICHENKO_COMMAND_HPP
