#ifndef DAC_DAC_H_
#define DAC_DAC_H_
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_gpio.h"

void Init_DAC(void);
void writeDAC(uint16_t value);

#endif /* DAC_DAC_H_ */
