#ifndef NALYVAICHENKO_TTY_TERMINAL_HPP
#define NALYVAICHENKO_TTY_TERMINAL_HPP

#include "./config.hpp"
#include "./terminal_input.hpp"

#include <zoal/arch/avr/utils/usart_transmitter.hpp>
#include <zoal/io/output_stream.hpp>

constexpr size_t tty_terminal_str_size = 64;
constexpr size_t tty_rx_buffer_size = 16;

using tty_transport = zoal::utils::usart_transmitter<tty_usart, 32, zoal::utils::interrupts_off>;
using tty_tx_stream_type = zoal::io::output_stream<tty_transport>;

extern tty_transport transport;
extern tty_tx_stream_type tty_stream;
extern zoal::misc::terminal_input terminal;
extern const char terminal_greeting[];
extern zoal::data::ring_buffer<uint8_t, tty_rx_buffer_size> tty_rx_buffer;
extern const char help_msg[];

#endif
