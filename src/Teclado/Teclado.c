#include "main.h"
#include "stm32f4xx.h"
#include "Definiciones.h"

#define FILA 4																		//DEFINICION MATRIZ TECLAS
#define COLUMNA 4

char KEYPAD[FILA][COLUMNA] = {
		{'1', '2', '3', 'A'},
		{'4', '5', '6', 'B'},
		{'7', '8', '9', 'C'},
		{'*', '0', '#', 'D'}
};

typedef struct{																		//ESTRUCTURA CON DATOS DE LOS PINES
	GPIO_TypeDef* port;
	uint16_t pin;
}DATO;

DATO GPIO_OUT[] = {																	//FILAS
		{GPIOC, GPIO_Pin_0},
		{GPIOC, GPIO_Pin_3},
		{GPIOC, GPIO_Pin_2},
		{GPIOC, GPIO_Pin_1},
		{GPIOA, GPIO_Pin_5}
};

DATO GPIO_IN[] = {																	//COLUMNAS
		{GPIOB, GPIO_Pin_0},
		{GPIOA, GPIO_Pin_8},
		{GPIOB, GPIO_Pin_7},
		{GPIOA, GPIO_Pin_15}
};

typedef enum {																		//MAQUINAS DE ESTADOS TECLADO
	INACTIVO,
	TIEMPO,
	PRESIONADO
}Keypad_State;

void Init_Keypad(void) {															//DECLARACION E INICIO PINES GPIO

	GPIO_InitTypeDef GPIO_InitStruct;
	uint8_t i = 0;
	uint8_t j = 0;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);							//HABILITACION DEL RELOJ
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;										//PINES DE SALIDA
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Fast_Speed;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	for (i = 0; i < 5; i++) {
		GPIO_InitStruct.GPIO_Pin = GPIO_OUT[i].pin;
		GPIO_Init(GPIO_OUT[i].port, &GPIO_InitStruct);
	}

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;										//PINES DE ENTRADA
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	for (j = 0; j < 4; j++) {
		GPIO_InitStruct.GPIO_Pin = GPIO_IN[j].pin;
		GPIO_Init(GPIO_IN[j].port, &GPIO_InitStruct);
	}
}

static uint32_t u32_Espera=0;
static Keypad_State STATE = INACTIVO;												//VARIBLES GLOBALES DONDE SE INICIALIZAN UNA SOLA VEZ PARA
uint8_t ROW = 0;																	//QUE LA MAQUINA DE ESTADOS NO TENGA QUE ESTA SETEANDO
uint8_t COL = 0;																	//CONTINUAMENTE LAS VARIBLES QUE SE USAN PARA SU RECORRIDO.

char KEY_T = '\0';																	//ALMACENAMIENTO VALOR TECLA

char Read_Keypad(General *Gen) {																//LECTURA DEL TECLADO

	switch(STATE){

	case INACTIVO:
		for (ROW = 0; ROW < FILA; ROW++) {											//INICIO RECORRIENDO LA PRIMER FILA
			GPIO_SetBits(GPIO_OUT[ROW].port, GPIO_OUT[ROW].pin);					//LECTURA DE LA FILA ACTIVA
			for (COL = 0; COL < COLUMNA; COL++) {									//INICIO RECORRIENDO LA PRIMER COLUMNA
				if(GPIO_ReadInputDataBit(GPIO_IN[COL].port, GPIO_IN[COL].pin)){
					STATE = TIEMPO;
					KEY_T='\0';
					return KEY_T;
				}
			}
			GPIO_ResetBits(GPIO_OUT[ROW].port, GPIO_OUT[ROW].pin);		//CASO DE NO ESTAR PRESIONADA NINGUNA TECLA EN LA FILA SE RESETEA PARA ACTIVAR LA SIGUIEN
		}
		break;

	case TIEMPO:
		if(Gen->u32_Systick - u32_Espera >20)
		{
			u32_Espera = Gen->u32_Systick;
			STATE = PRESIONADO;
		}															//DEBOUNCE PARA CHEQUEO DE CONFIRMACION
		break;

	case PRESIONADO:
		if(GPIO_ReadInputDataBit(GPIO_IN[COL].port, GPIO_IN[COL].pin)){				//LECTURA SOBRE LA TECLA PRESIONADA
			KEY_T = KEYPAD[ROW][COL];
		}
		else{
			KEY_T = '\0';
			STATE = INACTIVO;
		}
		break;
	}
	return KEY_T;
}
