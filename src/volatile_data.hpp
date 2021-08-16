#ifndef NALYVAICHENKO_VOLATILE_DATA_HPP
#define NALYVAICHENKO_VOLATILE_DATA_HPP

#include <stdint.h>

constexpr uint8_t hardware_event_tick = 1 << 0;
constexpr uint8_t hardware_event_tty_rx = 1 << 1;
constexpr uint8_t hardware_event_player_rx = 1 << 2;
constexpr uint8_t hardware_event_i2c = 1 << 3;
constexpr uint8_t hardware_event_msg = 1 << 4;

extern volatile uint8_t hardware_events;
extern volatile uint32_t milliseconds;

#endif
