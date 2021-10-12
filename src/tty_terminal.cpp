#include "./tty_terminal.hpp"

#include "./logo/ascii_logo.hpp"

const char terminal_greeting[] = "\033[0;32mmcu\033[m$ ";

zoal::data::ring_buffer<uint8_t, tty_rx_buffer_size> tty_rx_buffer;

tty_transport transport;
tty_tx_stream_type tty_stream(transport);

static char terminal_buffer[tty_terminal_str_size];
zoal::misc::terminal_input terminal(terminal_buffer, sizeof(terminal_buffer));

const char help_msg[] PROGMEM = "Commands: \r\n"
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
                                "\tsettings\tprint current settings\r\n";

ISR(USART0_RX_vect) {
    hardware_events |= hardware_event_tty_rx;
    tty_usart::rx_handler<>([](uint8_t value) { tty_rx_buffer.push_back(value); });
}

ISR(USART0_UDRE_vect) {
    tty_usart::tx_handler([](uint8_t &value) { return tty_transport::tx_buffer.pop_front(value); });
}
