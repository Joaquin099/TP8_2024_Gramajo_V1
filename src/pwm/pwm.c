/*-----------------------------------Functions----------------------------------*/
#include "LCD.h"
#include "DAC.h"
#include "ADC.h"
#include "pwm.h"
#include "Menu.h"
#include "main.h"
#include "stdio.h"
#include "Teclado.h"
#include "Display_LCD.h"
#include "Definiciones.h"

#include "stm32f4xx.h"
#include <stm32f4xx_adc.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_dac.h>
#include "stm32f4xx_exti.h"
#include "stm32f4xx_tim.h"

/*-----------------------------------Variables----------------------------------*/
char Mensaje[30];

volatile uint32_t T_actual=0;					//Intente utilizar las variables desde
volatile uint32_t Desborde=0;					//la estructura "General" para poder manejarlas
static uint32_t u32_Sys_PWM=0;					//desde un mismo lugar, pero tuve problema con
static uint32_t Diferencia = 0;					//las interrupciones.
volatile uint32_t T_anterior=0;					//Solucion: usar variables globales.
volatile uint32_t Frecuencia=0;
volatile uint32_t T_diferencia=0;

typedef enum{
	Inicio,
	Edicion,
	Confirmar,
	Salir
}Estados_pwm;

/*-----------------------------------Configuration Functions----------------------------------*/
void EXTILine10_config(void) {

	GPIO_InitTypeDef GPIO_InitStructure;							// Estructuras de configuraci�n para GPIO, NVIC y EXTI
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);			 // Habilitaci�n del reloj para el puerto GPIOB
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);			 // Habilitaci�n del reloj para el sistema de configuraci�n SYSCFG

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;						 // Configuraci�n del pin 10 del puerto GPIOB como entrada con pull-up
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource10);	// Selecci�n del pin 10 de GPIOB como fuente de la l�nea de interrupci�n EXTI

	EXTI_InitStructure.EXTI_Line = EXTI_Line10;						// Configuraci�n de la l�nea de interrupci�n EXTI para el pin 10
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			// Configuraci�n del NVIC para habilitar la interrupci�n de la l�nea EXTI 10-15
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM2_config(void) {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;					// Estructuras de configuraci�n para el temporizador TIM2 y el NVIC
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			// Habilitaci�n del reloj para el temporizador TIM2

	// Configuraci�n de la base de tiempo del temporizador TIM2
	//uint16_t PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 100000); //Frecuencia = Frec / TIM_Period -> (200KHz/400) = 500 Hz para la salida de este timer
	TIM_TimeBaseStructure.TIM_Prescaler = 1800 - 1;             	// Preescalador para dividir la frecuencia del reloj (180 MHz / 1800 = 100 kHz)
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 	// Modo de conteo ascendente
	TIM_TimeBaseStructure.TIM_Period = 65535;                   	// Per�odo m�ximo del temporizador (16 bits)
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     	// Divisi�n del reloj a la entrada del temporizador
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;            	// Sin repetici�n, cuenta una vez por cada per�odo
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);             	// Inicializa TIM2 con la configuraci�n anterior

	// Configuraci�n del NVIC para habilitar la interrupci�n de TIM2
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              	// Canal de la interrupci�n para TIM2
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   	// Prioridad de preempci�n m�s alta
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;          	// Subprioridad m�s alta
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             	// Habilitar el canal de la IRQ para TIM2
	NVIC_Init(&NVIC_InitStructure);


	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);						// Habilitaci�n de la interrupci�n de actualizaci�n (overflow) para TIM2
	TIM_Cmd(TIM2, ENABLE);											// Habilitaci�n del temporizador TIM2
}

void EXTI15_10_IRQHandler() {
	if (EXTI_GetITStatus(EXTI_Line10) != RESET) {								// Verifica si la interrupci�n en la l�nea EXTI 10 est� activa
		T_actual = TIM_GetCounter(TIM2) + Desborde * 65535; 					// Capturar el tiempo actual
		T_diferencia = T_actual - T_anterior;
		if (T_diferencia > 0) {													// Calcular la frecuencia (en Hz) si la diferencia de tiempo es v�lida
			Frecuencia = SystemCoreClock / ((TIM2->PSC + 1) * T_diferencia);	// SystemCoreClock y PSC son constantes, pueden ser precalculadas en la inicializaci�n
		}
		Diferencia = (Frecuencia/2);											//En este caso la frecuencia se encuientra dividida dos ya que el cooler posee dos imanes, entonces el sensor de efecto hall detectara 2 lecturas en un giro completo
		T_anterior = T_actual;													// Actualizar el tiempo anterior con el actual
		Desborde = 0;															// Reiniciar el contador de desbordesos
		EXTI_ClearITPendingBit(EXTI_Line10);									// Limpia el flag de la interrupci�n para la l�nea EXTI 10
	}
}

void TIM2_IRQHandler(void) {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {					  	// Verifica si la interrupci�n por actualizaci�n (overflow) de TIM2 est� activa
		Desborde++;																// Incrementa la variable global 'Desborde' cada vez que ocurre un overflow
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);								// Limpia el flag de la interrupci�n para evitar que se vuelva a disparar inmediatamente
	}
}

void PWM_config(void) {

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);						// Habilitar el reloj del timer utilizado TIM3
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);						// Habilitar el reloj para el puerto GPIOB, necesario para configurar el pin de salida PWM

	// Configurar el pin GPIOB1 para la salida PWM
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3);

	//uint16_t prescalerValue = (uint16_t) ((SystemCoreClock / 2) / 200000);	// Calcular los valores para la frecuencia deseada (1000 Hz) y el ciclo de trabajo del 50%
	//uint16_t periodValue = (uint16_t) (prescalerValue / 2);				// Prescaler = 90MHz/200KHz = 450

	// Configuraci�n de la base de tiempo del temporizador TIM3 en modo PWM
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 90-1; 					// Configura el preescalador (reduce la frecuencia del reloj)
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;					// Modo de conteo ascendente
	TIM_TimeBaseStructure.TIM_Period = 1000-1; 									// Periodo del PWM (determina la frecuencia de la se�al PWM) 1 KHz
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;						// Sin divisi�n adicional del reloj
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);								// Inicializa el temporizador TIM3 con esta configuraci�n

	// Configuraci�n del canal 4 del TIM3 para generar la se�al PWM
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;							// Configura el canal en modo PWM1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0xFFFF; 									 // Ciclo de trabajo del PWM (0xFFFF es el valor m�ximo)
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 					// Polaridad alta
	TIM_OC4Init(TIM3, &TIM_OCInitStructure); 									// Aplica la configuraci�n al canal 4 del TIM3
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);							// Habilita la precarga para asegurar la actualizaci�n del registro al final del periodo
	TIM_Cmd(TIM3, ENABLE);														// Habilitar el timer
}

void PWM_Ciclo(uint16_t Porcentaje) {
	uint16_t Pulso;
	if (Porcentaje > 100) {
		Porcentaje = 100;
	}
	Pulso = 1000- (Porcentaje *10);

	TIM_SetCompare4(TIM3, Pulso);												// Establece el valor del pulso en el canal 4 del TIM3 para ajustar el ciclo de trabajo PWM
}

/*-----------------------------------Main Function----------------------------------*/
void Menu_PWM(General *Gen){
	char str_texto[20];
	static uint16_t V_actual = 0;
	static uint16_t V_edicion = 0;
	static uint32_t T_inactivo = 0;
	static Estados_pwm EST = Inicio;
	static uint8_t tecla_presionada = 0;

	if (Gen->u32_Systick - u32_Sys_PWM > 500) {
		if (Diferencia > 20) {
			sprintf(Mensaje, "Frec: %lu Hz", Diferencia);
			LCD_WriteString(0, 1, Mensaje);
			u32_Sys_PWM  = Gen->u32_Systick;
		} else {
			LCD_gotoxy(1, 0);
			for (int i = 12; i < 16; i++) {
				LCD_WriteString(0, 1, " ");
			}
			sprintf(Mensaje, "Frec: ---Hz");
			LCD_WriteString(0, 1, Mensaje);
			u32_Sys_PWM  = Gen->u32_Systick;
		}
	}

	switch(EST){
	case Inicio:
		sprintf(str_texto, "PWM: %% %3u", V_actual);
		LCD_WriteString(0, 0, str_texto);
		if (Gen->Key == 'C') {
			EST = Edicion;
			V_edicion = V_actual;
		}
		T_inactivo = Gen->u32_Systick;
		break;

	case Edicion:
		if ((Gen->u32_Systick / 500) % 2 == 0) {								// Parpadeo del valor en edici�n
			sprintf(str_texto, "PWM: %% %3u", V_edicion);
		} else {
			sprintf(str_texto, "PWM: %%    ");
		}
		LCD_WriteString(0, 0, str_texto);

		if (Gen->Key >= '0' && Gen->Key <= '9') {
			if(Gen->Key != tecla_presionada){
				if(tecla_presionada == 0){
					tecla_presionada=Gen->Key;
					uint8_t digito = Gen->Key - '0';
					if(V_edicion == V_actual){
						V_edicion = digito;
					}else{
						uint16_t nuevo_pwm = V_edicion * 10 + digito;
						if (nuevo_pwm <= 100) {
							V_edicion = nuevo_pwm;
						} else {
							V_edicion = digito;
						}
					}
					T_inactivo = Gen->u32_Systick;
				}
			}
		} else{
			if (tecla_presionada != 0) {
				tecla_presionada = 0;  											// Reinicia el estado de la tecla presionada
			}
		}
		if (Gen->Key == 'C') {
			V_actual = V_edicion;
			EST = Confirmar;
			tecla_presionada = 0;
		} else if (Gen->Key == 'D' || Gen->u32_Systick - T_inactivo > 10000) {
			EST = Inicio;
			tecla_presionada = 0;
			T_inactivo = Gen->u32_Systick;
		}
		break;

	case Confirmar:
		PWM_Ciclo(V_actual);
		EST = Inicio;
		break;

	case Salir:
		EST = Inicio;
		break;
	}
}
