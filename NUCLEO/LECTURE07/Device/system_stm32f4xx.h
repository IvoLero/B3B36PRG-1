#ifndef __SYSTEM_STM32F4XX_H
#define __SYSTEM_STM32F4XX_H

#include "stm32f4xx_hal.h"

void SystemInit(void);
void SystemCoreClockUpdate(void);
void SystemClock_Config(void);

#endif /* __SYSTEM_STM32F4XX_H */