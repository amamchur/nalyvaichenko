#ifndef NALYVAICHENKO_CONFIG_HPP
#define NALYVAICHENKO_CONFIG_HPP

#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

#if defined (__AVR_ATmega2560__)
#include "config_atmega2560.hpp"
#elif defined(STM32F401xC)
#include "config_stm32f401.hpp"
#else
#include "config_atmega2560.hpp"
#endif

using scheduler_type = zoal::utils::function_scheduler<uint32_t, 8, void *>;
extern scheduler_type general_scheduler;

#endif
