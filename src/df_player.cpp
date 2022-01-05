#include "./df_player.hpp"

#include "event_manager.hpp"
#include "tty_terminal.hpp"

constexpr uint8_t df_cmd_next = 0x01;
constexpr uint8_t df_cmd_prev = 0x02;
constexpr uint8_t df_cmd_track = 0x03;
constexpr uint8_t df_cmd_inc_volume = 0x04;
constexpr uint8_t df_cmd_dec_volume = 0x05;
constexpr uint8_t df_cmd_reset = 0x0C;
constexpr uint8_t df_cmd_volume_set = 0x06;
constexpr uint8_t df_cmd_volume_get = 0x43;
constexpr uint8_t df_cmd_status = 0x42;

#define DF_DEBUG 1

void df_player::send() {
    using hex = zoal::io::hexadecimal_functor<uint8_t>;
    waiting_ack_ = true;

#if DF_DEBUG
    tty_stream << "\r\n<< ";
    for (size_t i = 0; i < sizeof(request_); i++) {
        tty_stream << hex(request_[i]) << " ";
    }
    tty_stream << "\r\n";
#endif
    df_player_tx_transport::send_data(request_, sizeof(request_));
}

uint16_t df_player::calculate_check_sum(const uint8_t *buffer) {
    uint16_t sum = 0;
    for (int i = msg_version; i < msg_checksum; i++) {
        sum += buffer[i];
    }
    return ~sum + 1;
}

void df_player::uint16_to_array(uint16_t value, uint8_t *array) {
    *array = (uint8_t)(value >> 8);
    *(array + 1) = (uint8_t)(value & 0xFF);
}

uint16_t df_player::array_to_uint16(const uint8_t *array) {
    uint16_t value = *array;
    value <<= 8;
    value += *(array + 1);
    return value;
}

void df_player::send_command(uint8_t command, uint16_t argument) {
    request_[msg_command] = command;
    uint16_to_array(argument, request_ + msg_parameter);
    uint16_to_array(calculate_check_sum(request_), request_ + msg_checksum);
    send();
}

void df_player::reset() {
    playing_ = false;
    send_command(df_cmd_reset);
}

void df_player::status() {
    send_command(df_cmd_status);
}

void df_player::play(int file_number) {
    playing_ = true;
    send_command(df_cmd_track, file_number);
}

void df_player::volume() {
    send_command(df_cmd_volume_get);
}

void df_player::volume(int volume) {
    send_command(df_cmd_volume_set, volume);
}

void df_player::push_data(const void *data, size_t size) {
    parser_.push_and_scan(data, size);
}

void df_player::process_response_(const zoal::misc::df_player_scanner &scanner) {
#if DF_DEBUG
    {
        using hex = zoal::io::hexadecimal_functor<uint8_t>;
        auto s = reinterpret_cast<const uint8_t *>(scanner.token_start());
        auto e = reinterpret_cast<const uint8_t *>(scanner.token_end());
        tty_stream << "\r\n>> ";
        while (s < e) {

            tty_stream << hex(*s) << " ";
            s++;
        }
        tty_stream << "\r\n";
    }
#endif

    auto resp = reinterpret_cast<const uint8_t *>(scanner.token_start());
    auto cs1 = calculate_check_sum(resp);
    auto cs2 = array_to_uint16(resp + msg_checksum);

    if (cs1 != cs2) {
        waiting_ack_ = false;
        return;
    }

    auto params = array_to_uint16(resp + msg_parameter);
    auto cmd = resp[msg_command];
    switch (cmd) {
    case 0x03:
        playing_ = true;
        break;
    case 0x41:
        waiting_ack_ = false;
        break;
    case 0x3D:
        playing_ = false;
        play_next_track();
        break;
    default:
        if (callback_) {
            callback_(cmd, params);
        }
        break;
    }
}

void df_player::play_next_track() {
    if (waiting_ack_ || playing_) {
        return;
    }

    df_player_track track;
    if (!queue_.pop_front(track)) {
        return;
    }
    play(track.file);
}

void df_player::enqueue_track(int file_number) {
    df_player_track track;
    track.file = file_number;
    queue_.push_back(track);
    play_next_track();
}

void df_player::handler(const zoal::misc::df_player_scanner &m) {
    auto me = reinterpret_cast<df_player*>(m.context);
    me->process_response_(m);
}

df_player::df_player() noexcept : parser_(parse_buffer, sizeof(parse_buffer)) {
    parser_.callback(&handler);
    parser_.context = this;
}
