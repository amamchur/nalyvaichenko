#include "message.hpp"

#include "volatile_data.hpp"

#include <zoal/utils/interrupts.hpp>

zoal::data::ring_buffer<message, 8> message_queue;

void send_message(message &msg) {
    zoal::utils::interrupts_off scope_off;
    message_queue.push_back(msg);
    hardware_events |= hardware_event_msg;
}

void send_event(event_type type) {
    message msg{};
    msg.type = message_type::event;
    msg.e.type = type;
    send_message(msg);
}

void send_event(const event &e) {
    message msg{};
    msg.type = message_type::event;
    msg.e = e;
    send_message(msg);
}

void send_command(command_type type) {
    message msg{};
    msg.type = message_type::command;
    msg.c.type = type;
    send_message(msg);
}

void send_command(const command &cmd) {
    message msg{};
    msg.type = message_type::command;
    msg.c = cmd;
    send_message(msg);
}

bool pop_message(message &msg) {
    zoal::utils::interrupts_off scope_off;
    return message_queue.pop_front(msg);
}
