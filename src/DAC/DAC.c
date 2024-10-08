#include "DAC.h"
#include "Menu.h"
#include "main.h"
#include "Definiciones.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_rcc.h"

void Init_DAC(void){
	static DAC_InitTypeDef DAC_InitStructure;
	static GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);							//Clock Enable

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;										//Configuracion en el pin 4 - PORT A como salida Analogica (es la DAC OUT1)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);							//DAC: Activar Clock

	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;							//DAC configuracion
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	DAC_Cmd(DAC_Channel_1, ENABLE);												//DAC ON Channel 1
	writeDAC(0);
}

void writeDAC(uint16_t value){
	DAC_SetChannel1Data(DAC_Align_12b_R, value);
}


