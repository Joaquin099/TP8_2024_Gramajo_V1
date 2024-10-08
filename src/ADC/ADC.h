#ifndef ADC_ADC_H_
#define ADC_ADC_H_
#include "stm32f4xx.h"
#include "main.h"
#include "Definiciones.h"

void Init_ADC(void);
void ADC_Muestreo(General *Gen);
uint16_t ADC_Lectura(General *Gen);

#endif /* ADC_ADC_H_ */
