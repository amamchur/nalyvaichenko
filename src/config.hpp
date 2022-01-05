#ifndef NALYVAICHENKO_CONFIG_HPP
#define NALYVAICHENKO_CONFIG_HPP

#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

#if defined(STM32F401xC)
#include "config_stm32f401.hpp"
#else
#include "config_host.hpp"
#endif

#include "./bartender_machine_v2.hpp"

using scheduler_type = zoal::utils::function_scheduler<uint32_t, 8, void *>;
extern scheduler_type general_scheduler;

extern bartender_machine_v2 machine;

extern volatile uint16_t sensors_values[2];
inline uint16_t hall_sensor_value() {
    return sensors_values[0];
}

inline uint16_t ir_sensor_value() {
    return sensors_values[1];
}

#endif
