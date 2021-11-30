#ifndef NALYVAICHENKO_EVENT_MANAGER_HPP
#define NALYVAICHENKO_EVENT_MANAGER_HPP

#include <stdint.h>

constexpr uint8_t hardware_event_tick = 1 << 0;
constexpr uint8_t hardware_event_tty_rx = 1 << 1;
constexpr uint8_t hardware_event_player_rx = 1 << 2;
constexpr uint8_t hardware_event_i2c = 1 << 3;
constexpr uint8_t hardware_event_msg = 1 << 4;

class event_manager {
public:
    static void set(uint8_t e);
    static void set_isr(uint8_t e);
    static uint8_t get();
};

#endif
