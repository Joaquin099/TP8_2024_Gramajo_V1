#include "main.h"
#include "DAC.h"
#include "LCD.h"
#include "UART.h"
#include "Menu.h"
#include "Teclado.h"
#include "Definiciones.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
	Inicio,
	Verifico,
	Menu_UART,
	Lectura_Menu,
	Lectura_Tecla,
	Variacion_DAC,
	Variacion_Baudrate,
	Verificacion_soltar
} Estado;

Estado ESTs = Inicio;

volatile uint8_t IndexRX = 0;
volatile uint8_t PendingMessage = 0;
uint8_t bandera = 0;



void UART4_Config(General *Gen) {								// Inicialización de GPIO y USART4

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);		// Habilitar el reloj de GPIOA
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, DISABLE);		// Habilitar el reloj de UART4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);		// Habilitar el reloj de UART4

	GPIO_InitTypeDef GPIO_InitStructure;						// Configuración de los pines GPIOA (PA0 - TX, PA1 - RX)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4); // PA0 -> TX	// Asignar los pines PA0 y PA1 a UART4 (AF8)
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4); // PA1 -> RX

	USART_DeInit(UART4);
	USART_Cmd(UART4, DISABLE);

	USART_InitTypeDef USART_InitStructure;							// Configuración de UART4
	USART_InitStructure.USART_BaudRate = Gen->Baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;							// Configuración NVIC para UART4
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(UART4, ENABLE);										// Habilitar UART4

	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);					// Habilitar interrupciones para UART4
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
}

void UART4_IRQHandler(General *Gen) {
	if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET) {				// Si se ha recibido un dato
		Gen->RX_Buffer[IndexRX++] = USART_ReceiveData(UART4);			// Almacenar el dato recibido en el buffer

		if (IndexRX >= BufferDimension) {								// Evitar desbordamiento del buffer
			IndexRX = 0;
		}
		USART_ClearITPendingBit(UART4, USART_IT_IDLE);
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);					// Limpiar el flag de la interrupción
	}

	if (USART_GetITStatus(UART4, USART_IT_IDLE) == SET) {				// Si la línea está en reposo (no hay más datos)
		PendingMessage = 1;  											// Señalar que hay un mensaje pendiente
		Gen->RX_Buffer[IndexRX] = '\0';  								// Terminar la cadena recibida
		Gen->str_length = IndexRX;
		IndexRX = 0;  													// Reiniciar el índice del buffer
		USART_ReceiveData(UART4); 										// Limpiar la línea en reposo
		USART_ClearITPendingBit(UART4, USART_IT_IDLE);
	}
}

void usart2SendData(uint8_t *Buff, uint8_t Length){
	uint8_t i;
	for(i = 0; i < Length; i++){
		USART_SendData(UART4, (uint8_t) Buff[i]);
		while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
	}
}

void readMessage(General *Gen) {
	uint8_t i = 0;
	uint8_t texto = 0;  										// Variable para determinar si el mensaje contiene letras

	for(i = 0; i < Gen->str_length; i++) {								// Recorrer y copiar el mensaje recibido
		Gen->Mensaje[i] = Gen->RX_Buffer[i];
		if(isalpha(Gen->Mensaje[i])) {							// Verificar si el carácter es una letra
			texto = 1;
		}
	}
	Gen->Mensaje[Gen->str_length] = '\0';  								// Agregar el terminador nulo al final del mensaje

	if(!texto) {												// Si el mensaje no contiene letras, convertir a entero
		bandera = atoi(Gen->Mensaje);  							// Convertir a entero si es válido
	} else {
		bandera = 0;  											// En caso contrario, asignar 0
	}
	// Reiniciar buffers e índices
	//Gen->RX_Buffer[IndexRX] = '\0';  							// Asegurarse de que el buffer está correctamente terminado
	Gen->Index_Buffer = 0;               						// Reiniciar el índice
	PendingMessage = 0;        									// Reiniciar el flag de mensaje pendiente
}

void Init_MenuUART(General *Gen) {
	switch (ESTs) {

	case Inicio:											// Estado inicial, presentacion del menu
		usart2SendData((uint8_t *)" \r\nPara acceder al Menu presione ENTER.\r\n", 41);
		ESTs = Verifico;
		break;

	case Verifico:											// Ingreso con Eenter
		if (PendingMessage) {
			readMessage(Gen);
			if (Gen->Mensaje[0] == '\r') {
				ESTs = Menu_UART;
			}
		}
		break;

	case Menu_UART:											// Mostrando el menú principal
		usart2SendData((uint8_t *)
				"Menu Principal:\r\n"
				"1) Leer Tecla Presionada\r\n"
				"2) Generador DAC\r\n"
				"3) Modificacion de Baudrate\r\n", 92);
		ESTs = Lectura_Menu;
		break;

	case Lectura_Menu:										// Procesando la selección del menú principal
		if (PendingMessage) {
			readMessage(Gen);

			if (Gen->Mensaje[1] == '\r') {  				// Verificar que la opción está bien ingresada
				switch (bandera) {
				case 1:
					usart2SendData((uint8_t *)"\nPresione una tecla:\r\n", 25);
					ESTs = Lectura_Tecla;
					break;
				case 2:
					usart2SendData((uint8_t *)"\nIngrese un valor en mV (0 a 3300):\r\n", 40);
					ESTs = Variacion_DAC;
					break;
				case 3:
					sprintf(Gen->RX_Buffer, "\nValor actual de baudrate: %d \r\nIngrese nuevo baudrate:\r\n", Gen->Baudrate);
					usart2SendData((uint8_t *)Gen->RX_Buffer, strlen(Gen->RX_Buffer));
					ESTs = Variacion_Baudrate;
					break;
				default:
					usart2SendData((uint8_t *)"Opcion no valida \r\n\n", 22);
					ESTs = Menu_UART;
					break;
				}
			}
		}
		break;

	case Lectura_Tecla:												// Estado para leer una tecla presionada
		if (Gen->Key != '\0' && Gen->Mensaje[0] != '\r') {
			sprintf(Gen->RX_Buffer, "Tecla presionada: %c\r\n", Gen->Key);
			usart2SendData((uint8_t *)Gen->RX_Buffer, strlen(Gen->RX_Buffer));
			ESTs = Verificacion_soltar;
		}
		if (PendingMessage) {
			readMessage(Gen);
			if (Gen->Mensaje[0] == '\r') {
				ESTs = Menu_UART;
			}
		}
		break;

	case Variacion_DAC:												// Estado para ingresar un valor DAC
		if(PendingMessage){
			readMessage(Gen);
			if(bandera){
				uint16_t dacValue = atoi(Gen->RX_Buffer);  			// Convertir a número
				if (dacValue >= 0 && dacValue <= 3300) {
					char printDAC[100];
					uint16_t dacRegisterValue = (dacValue * 4095) / 3300;
					DAC_SetChannel1Data(DAC_Align_12b_R, dacRegisterValue);
					sprintf(printDAC, "\nDAC cambiado a: %d \r\n\n", dacValue);
					usart2SendData((uint8_t *)printDAC, strlen(printDAC));
					ESTs = Variacion_DAC;
				}else if(dacValue > 3300 ) {
					usart2SendData((uint8_t *)"Parametro no valido.\r\n\n", 24);
					ESTs = Variacion_DAC;
				}
			}else if(Gen->Mensaje[0] == '\r'){
				ESTs = Menu_UART;
			}

			if(Gen->Mensaje[0] == '\r'){
				ESTs = Menu_UART;
			}
		}
		break;

	case Variacion_Baudrate:									// Estado para modificar el baudrate
		if (PendingMessage) {
			uint32_t newBaudrate = atoi(Gen->RX_Buffer);
			if (newBaudrate >= 4800 && newBaudrate <= 115200) {
				Gen->Baudrate = newBaudrate;
				//UART4_Config(Gen);  													// Reconfigurar la UART con el nuevo baudrate
				sprintf(Gen->RX_Buffer, "Baudrate cambiado a: %d \r\n\n", Gen->Baudrate);
				usart2SendData((uint8_t *)Gen->RX_Buffer, strlen(Gen->RX_Buffer));
				UART4_Config(Gen);
				ESTs = Menu_UART;
			} else {
				usart2SendData((uint8_t *)"Parametro no valido.\r\n\n", 24);
				ESTs = Menu_UART;
			}

			if(Gen->Mensaje[0] == '\r'){
				ESTs = Menu_UART;
			}
		}
		break;

	case Verificacion_soltar:
		if(Gen->Key == '\0'){
			ESTs = Lectura_Tecla;
		}
	}
}
