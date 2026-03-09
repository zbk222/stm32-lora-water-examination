#include "stm32g4xx_hal.h"
#include "FreeRTOS.h"

void vApplicationTickHook(void)
{
    HAL_IncTick();
}
