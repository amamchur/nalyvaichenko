#include "./tty_terminal.hpp"

#include "./logo/ascii_logo.hpp"
#include "hardware.hpp"
#include "message.hpp"
#include "parsers/command_machine.hpp"

const char terminal_greeting[] = "\033[0;32mmcu\033[m$ ";

zoal::data::ring_buffer<uint8_t, tty_rx_buffer_size> tty_rx_buffer;

tty_tx_transport transport;
tty_tx_stream_type tty_stream(transport);

static char terminal_buffer[tty_terminal_str_size];
zoal::misc::terminal_input terminal(terminal_buffer, sizeof(terminal_buffer));
char command_history[tty_terminal_str_size] = {0};
const char help_msg[] = "Commands: \r\n"
                        "\thelp\t\tdisplay help\r\n"
                        "\ti2c\t\tscan i2c devices\r\n"
                        "\tcalibrate\tcalibrate revolver\r\n"
                        "\tnext\t\tnext segment\r\n"
                        "\tgo\t\trun machines\r\n"
                        "\tadc\t\tadc\r\n"
                        "\tvalve [ms]\tvalve\r\n"
                        "\tstop\t\tstop machine\r\n"
                        "\tpump [ms]\tpump\r\n"
                        "\tplay [track]\tplay track\r\n"
                        "\tsettings\tprint current settings\r\n"
                        "\tenc [steps]\tencoder\r\n"
                        "\tpress\t\tpress\r\n";

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

void handle_v100(const zoal::misc::terminal_input *, zoal::misc::terminal_machine_event e) {
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

void vt100_callback(const zoal::misc::terminal_input *, const char *s, const char *e) {
    transport.send_data(s, e - s);
}

void initialize_terminal() {
    terminal.vt100_feedback(&vt100_callback);
    terminal.input_callback(&input_callback);
    terminal.handle_v100(&handle_v100);
    terminal.greeting(terminal_greeting);
    terminal.clear();
    tty_stream << ascii_logo << help_msg;
    terminal.sync();
}
