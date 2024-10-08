#ifndef MAIN_H_
#define MAIN_H_
#define delayms 0x35E8

#include "stdio.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"

void SysTick_Handler(void);
void Init_BackL(void);
void delay_ms(uint16_t timeDelay);
void delay_us(uint16_t timeDelay);

#endif /* MAIN_H_ */
