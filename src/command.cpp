#include "command.hpp"

#include <zoal/utils/interrupts.hpp>

zoal::data::ring_buffer<command, 4> command_queue;

void send_command(const command& cmd) {
    zoal::utils::interrupts_off scope_off;
    command_queue.push_back(cmd);
}

void send_command(command_type type) {
    send_command(command(type));
}

bool pop_command(command &cmd) {
    zoal::utils::interrupts_off scope_off;
    return command_queue.pop_front(cmd);
}
