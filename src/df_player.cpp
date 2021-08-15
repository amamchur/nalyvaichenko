//
// Created by andrii on 14.08.21.
//

#include "./df_player.hpp"

#include "./hardware.hpp"

#include <avr/interrupt.h>

zoal::data::ring_buffer<uint8_t, df_player_rx_buffer_size> df_player_rx_buffer;
df_player_transport df_player_rx;
df_player_tx_stream_type df_player_tx_stream(df_player_rx);

ISR(USART1_RX_vect) {
    df_player_usart::rx_handler<>([](uint8_t value) { df_player_rx_buffer.push_back(value); });
}

ISR(USART1_UDRE_vect) {
    df_player_usart::tx_handler([](uint8_t &value) { return df_player_transport::tx_buffer.pop_front(value); });
}

df_player::df_player() = default;

void df_player::send() {
    df_player_rx.send_data(message_, sizeof(message_));
    delay::ms(10);
}

uint16_t df_player::calculate_check_sum(const uint8_t *buffer) {
    uint16_t sum = 0;
    for (int i = msg_version; i < msg_checksum; i++) {
        sum += buffer[i];
    }
    return -sum;
}

void df_player::uint16ToArray(uint16_t value, uint8_t *array) {
    *array = (uint8_t)(value >> 8);
    *(array + 1) = (uint8_t)(value);
}

void df_player::send_command(uint8_t command, uint16_t argument) {
    message_[msg_command] = command;
    uint16ToArray(argument, message_ + msg_parameter);
    uint16ToArray(calculate_check_sum(message_), message_ + msg_checksum);
    send();
}

void df_player::reset() {
    send_command(0x0C);
}

void df_player::play(int fileNumber) {
    send_command(0x03, fileNumber);
}
