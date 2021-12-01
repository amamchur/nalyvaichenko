#ifndef NALYVAICHENKO_CONFIG_HPP
#define NALYVAICHENKO_CONFIG_HPP

#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

#if defined(STM32F401xC)
#include "config_stm32f401.hpp"
#else
#include "config_host.hpp"
#endif

using scheduler_type = zoal::utils::function_scheduler<uint32_t, 8, void *>;
extern scheduler_type general_scheduler;

#endif
