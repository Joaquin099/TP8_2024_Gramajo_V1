/*-----------------------------------Includes-----------------------------------*/
#include "sd.h"
#include "LCD.h"
#include "DAC.h"
#include "ADC.h"
#include "pwm.h"
#include "UART.h"
#include "Menu.h"
#include "stdio.h"
#include "Teclado.h"
#include "Display_LCD.h"
#include "Definiciones.h"

#include "SPI.h"
#include "ff.h"

/*-----------------------------------Functions----------------------------------*/
void SysTick_Handler(void);
void delay_ms (uint16_t timeDelay);
void delay_us (uint16_t timeDelay);
void Init_BackL(void);

/*-----------------------------------Variables----------------------------------*/

uint32_t u32_Sys_Display;
uint32_t u32_Sys_Keyboard;
uint32_t u32_Sys_Contador;

General Gen;

/*------------------------------------Main-------------------------------------*/
int main(void)
{
	Gen.u32_Systick = 0;
	Gen.Key_Press   = 0;
	Gen.Baudrate 	= 9600;

	u32_Sys_Display  = Gen.u32_Systick;
	u32_Sys_Keyboard = Gen.u32_Systick;
	u32_Sys_Contador = Gen.u32_Systick;

	Init_Keypad();
	Init_BackL();
	Init_Display();
	Init_DAC();
	Init_ADC();

	TIM2_config();
	PWM_config();
	EXTILine10_config();
	UART4_Config(&Gen);
	initSPI();
	initSD();

	while(1){
		Init_MenuUART(&Gen);
		if (Gen.u32_Systick - u32_Sys_Contador > 1000) {				//CONTADOR SEGUNDOS
			u32_Sys_Contador = Gen.u32_Systick;
			Gen.u16_Sys_Time++;
		}

		if (Gen.u32_Systick - u32_Sys_Keyboard > 20) {					//TECLADO MATRICIAL
			u32_Sys_Keyboard = Gen.u32_Systick;
			Gen.Key = Read_Keypad(&Gen);
		}

		if (Gen.u32_Systick - u32_Sys_Display > 250) {				//REFRESCO LCD
			u32_Sys_Display = Gen.u32_Systick;
			Init_Menu(&Gen);
			if(Gen.Key == '#'){
				GPIO_SetBits(GPIOB, GPIO_Pin_13);
			}else if(Gen.Key == '*'){
				GPIO_ResetBits(GPIOB, GPIO_Pin_13);
			}
		}
	}

}

/*-----------------------------------Functions---------------------------------*/
void SysTick_Handler(void) {
	if (Gen.u32_Systick > 0xFFFF){
		Gen.u32_Systick = 0;
	}
	Gen.u32_Systick++;
}

void delay_ms (uint16_t timeDelay){
	uint16_t LocalSystick = Gen.u32_Systick;
	while (1)
		if (Gen.u32_Systick - LocalSystick > timeDelay){
			return;
		}
}

void delay_us (uint16_t timeDelay){
	uint16_t u16i;
	while (timeDelay){
		for (u16i = 0; u16i < 14; u16i++);								//DELAY DE 1us
		timeDelay--;
	}
}

void Init_BackL(void){													//INICIO BL Y CK
	SysTick_Config(SystemCoreClock / 1000);
	NVIC_SetPriority (SysTick_IRQn, 0);

	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_SetBits(GPIOB, GPIO_Pin_13); 								//ENCENDIDO PRINCIPAL BACKLIGHT
}
