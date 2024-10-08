/*-----------------------------------Includes-----------------------------------*/
#include "ADC.h"
#include "Menu.h"
#include "Definiciones.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_rcc.h"

/*-----------------------------------Variables----------------------------------*/
uint16_t Promedio 		  = 0;
uint16_t Lectura_ADC	  = 0;
uint16_t temperatura 	  = 0;
uint16_t Conversor_Valor  = 0;
uint16_t u16_L_SystickADC = 0;

/*-----------------------------------Functions----------------------------------*/
void Init_ADC(void){
	static ADC_InitTypeDef ADC_InitStructure;
	static GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);							//Clock Enable

	GPIO_StructInit(&GPIO_InitStructure);											// Configurar pines ADC
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);							// Habilitar el reloj del ADC

	ADC_StructInit(&ADC_InitStructure);												// Configurar parametros del ADC
	ADC_InitStructure.ADC_Resolution 		   = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode 		   = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode   = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign 		   = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion 	   = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_3Cycles);		// Configurar canal del ADC (canal 0)
	ADC_Cmd(ADC1, ENABLE);															// Habilitar ADC
}

uint16_t ADC_Lectura(General *Gen){
	ADC_SoftwareStartConv(ADC1);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}

void ADC_Muestreo(General *Gen){
	if (Gen->u32_Systick - u16_L_SystickADC > 1000){		//Repito cada un segundo
		u16_L_SystickADC = Gen->u32_Systick;
		for(uint8_t i = 0; i < 50; i++){
			Lectura_ADC = ADC_Lectura(Gen);
			Promedio += Lectura_ADC;
		}
		Conversor_Valor = Promedio / 50;
		Gen->Temp = (((Conversor_Valor * 3300.0) / 4095.0) / 10);
	}
	Promedio = 0;
}
