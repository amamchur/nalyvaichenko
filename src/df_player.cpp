#include "./df_player.hpp"

#include "event_manager.hpp"

static df_player_tx_transport df_player_rx;

void df_player::send() {
    waiting_ack_ = true;
    df_player_rx.send_data(request_, sizeof(request_));
    delay::ms(10);
}

uint16_t df_player::calculate_check_sum(const uint8_t *buffer) {
    uint16_t sum = 0;
    for (int i = msg_version; i < msg_checksum; i++) {
        sum += buffer[i];
    }
    return -sum;
}

void df_player::uint16_to_array(uint16_t value, uint8_t *array) {
    *array = (uint8_t)(value >> 8);
    *(array + 1) = (uint8_t)(value);
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
    send_command(0x0C);
}

void df_player::play(int fileNumber) {
    playing_ = true;
    send_command(0x03, fileNumber);
}

void df_player::push_byte(uint8_t byte) {
    response_[response_bytes_++] = byte;
    if (response_bytes_ == sizeof(response_)) {
        process_response();
        response_bytes_ = 0;
    }
}

void df_player::process_response() {
    auto cs1 = calculate_check_sum(response_);
    auto cs2 = array_to_uint16(response_ + msg_checksum);

    if (cs1 != cs2) {
        waiting_ack_ = false;
        return;
    }

    auto params = array_to_uint16(response_ + msg_parameter);
    auto cmd = response_[msg_command];
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

    //    using hex = zoal::io::hexadecimal_functor<uint8_t>;
    //    tty_stream << "\033[2K\r";
    //    for (unsigned char i : request_) {
    //        tty_stream << hex(i) << " ";
    //    }
    //    tty_stream << "\r\n";
    //
    //    for (unsigned char i : response_) {
    //        tty_stream << hex(i) << " ";
    //    }
    //    tty_stream << "\r\n";
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

void df_player::enqueue_track(int fileNumber) {
    df_player_track track;
    track.file = fileNumber;
    queue_.push_back(track);
    play_next_track();
}

df_player::df_player() noexcept {}
