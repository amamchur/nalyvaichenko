//
// Created by andrii on 14.08.21.
//

#ifndef NALYVAICHENKO_DF_PLAYER_HPP
#define NALYVAICHENKO_DF_PLAYER_HPP

#include "./config.hpp"

#include <zoal/arch/avr/utils/usart_transmitter.hpp>
#include <zoal/io/output_stream.hpp>
#include <zoal/func/function.hpp>

constexpr size_t df_player_rx_buffer_size = 16;

using df_player_transport = zoal::utils::usart_transmitter<df_player_usart, 16, zoal::utils::interrupts_off>;
using df_player_tx_stream_type = zoal::io::output_stream<df_player_transport>;

extern df_player_tx_stream_type df_player_tx_stream;
extern zoal::data::ring_buffer<uint8_t, df_player_rx_buffer_size> df_player_rx_buffer;

class df_player_track {
public:
    int file{0};
};

class df_player {
public:
    static constexpr uint8_t df_player_message_size = 10;
    static constexpr uint8_t msg_header = 0;
    static constexpr uint8_t msg_version = 1;
    static constexpr uint8_t msg_length = 2;
    static constexpr uint8_t msg_command = 3;
    static constexpr uint8_t msg_ack = 4;
    static constexpr uint8_t msg_parameter = 5;
    static constexpr uint8_t msg_checksum = 7;
    static constexpr uint8_t msg_end = 9;

    static constexpr uint8_t cmd_init_params= 0x3F;

    df_player();

    void reset();
    void send();

    zoal::func::function<16, void, uint8_t, uint16_t> callback_;

    static uint16_t calculate_check_sum(const uint8_t *buffer);
    static void uint16ToArray(uint16_t value, uint8_t *array);
    void send_command(uint8_t command, uint16_t argument = 0);
    void play(int fileNumber);
    void enqueue_track(int fileNumber);

    void push_byte(uint8_t byte);
    void process_response();

private:
    uint16_t arrayToUint16(const uint8_t *array);
    zoal::data::ring_buffer<df_player_track, 5> queue_;
    uint8_t request_[df_player_message_size]{0x7E, 0xFF, 06, 00, 01, 00, 00, 00, 00, 0xEF};
    uint8_t response_[df_player_message_size]{0x7E, 0xFF, 06, 00, 01, 00, 00, 00, 00, 0xEF};
    int response_bytes_{0};
    bool waiting_ack_{false};
    void play_next_track();
    bool playing_{false};
};

#endif
