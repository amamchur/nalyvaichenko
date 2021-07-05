//
// Created by andrii on 03.07.21.
//

#include "tty_terminal.hpp"

#include "ascii_logo.hpp"

const char terminal_greeting[] = "\033[0;32mmcu\033[m$ ";

zoal::data::ring_buffer<uint8_t, tty_rx_buffer_size> tty_rx_buffer;

tty_transport transport;
tty_tx_stream_type tty_stream(transport);

static char terminal_buffer[tty_terminal_str_size];
zoal::misc::terminal_input terminal(terminal_buffer, sizeof(terminal_buffer));

const char help_msg[] PROGMEM = "Commands: \r\n"
                                "\ti2c-scan\tscan i2c devises\r\n"
                                "\tcalibrate\tcalibrate revolver\r\n"
                                "\tnext\t\tnext segment\r\n"
                                "\tgo\t\tgo\r\n"
                                "\tadc\t\tadc\r\n"
                                "\tpump\t\tpump\r\n";

ISR(USART0_RX_vect) {
    tty_usart::rx_handler<>([](uint8_t value) { tty_rx_buffer.push_back(value); });
}

ISR(USART0_UDRE_vect) {
    tty_usart::tx_handler([](uint8_t &value) { return tty_transport::tx_buffer.pop_front(value); });
}
