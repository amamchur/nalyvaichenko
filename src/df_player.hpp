#ifndef NALYVAICHENKO_DF_PLAYER_HPP
#define NALYVAICHENKO_DF_PLAYER_HPP

#include "./config.hpp"
#include "./parsers/df_player_machine.hpp"

#include <zoal/data/ring_buffer.hpp>
#include <zoal/func/function.hpp>
#include <zoal/io/output_stream.hpp>

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
    static constexpr uint8_t cmd_init_params = 0x3F;

    df_player() noexcept;

    void reset();
    void send();
    void status();

    zoal::func::function<16, void, uint8_t, uint16_t> callback_;

    void send_command(uint8_t command, uint16_t argument = 0);
    void play(int file_number);
    void volume();
    void volume(int value);
    void enqueue_track(int file_number);

    void push_data(const void *data, size_t size);

    //private:
    static uint16_t calculate_check_sum(const uint8_t *buffer);
    static void uint16_to_array(uint16_t value, uint8_t *array);
    static uint16_t array_to_uint16(const uint8_t *array);

    zoal::data::ring_buffer<df_player_track, 5> queue_;
    uint8_t request_[df_player_message_size]{0x7E, 0xFF, 06, 00, 01, 00, 00, 00, 00, 0xEF};
    int response_bytes_{0};
    bool waiting_ack_{false};
    void play_next_track();
    bool playing_{false};

    static void handler(const zoal::misc::df_player_scanner &m);
    zoal::misc::df_player_parser parser_;
    uint8_t parse_buffer[32]{0};
    void process_response_(const zoal::misc::df_player_scanner &scanner);
};

#endif
