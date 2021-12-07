#include "event_manager.hpp"

#include <zoal/utils/interrupts.hpp>

#if defined(STM32F401xC)

#include <zoal/freertos/event_group.hpp>
#include <zoal/mem/reserve_mem.hpp>

zoal::freertos::event_group<zoal::freertos::freertos_allocation_type::static_mem> hardware_os_events;

void event_manager::set(uint8_t e) {
    hardware_os_events.set(e);
}

void event_manager::set_isr(uint8_t e) {
    hardware_os_events.set_isr(e);
}

uint8_t event_manager::get() {
    return hardware_os_events.wait(0xFF);;
}

#else

static volatile uint8_t hardware_events;

void event_manager::set(uint8_t e) {
    hardware_events |= e;
}

void event_manager::set_isr(uint8_t e) {
    hardware_events |= e;
}

uint8_t event_manager::get() {
    zoal::utils::interrupts_off off;
    auto events = hardware_events;
    hardware_events = 0;
    return events;
}

#endif
