#include "gpio.h"
#include "hardware.hpp"
#include "spi.h"
#include "stm32f4xx_hal.h"
#include "usart.h"

#include <zoal/freertos/task.hpp>
#include <zoal/mem/reserve_mem.hpp>

[[noreturn]] void zoal_main_task(void *);

using task_type = zoal::freertos::task<zoal::freertos::freertos_allocation_type::static_mem>;

__attribute__((unused)) zoal::mem::reserve_mem<task_type, 256, StackType_t> main_task(zoal_main_task, "main");

extern "C" void SystemClock_Config(void);

[[noreturn]] void zoal_main_task(void *) {
    for (;;) {
    }
}

int main() {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_USART1_UART_Init();

//    zoal_init_hardware();

    vTaskStartScheduler();
    return 0;
}