#ifndef DEFINICIONES_DEFINICIONES_H_
#define DEFINICIONES_DEFINICIONES_H_

#include "stm32f4xx.h"

#define BufferDimension 200


typedef	struct {
	char Key;
	uint16_t Temp;
	uint16_t u16_DAC;
	uint8_t Key_Press;
	uint32_t u32_Systick;
	uint16_t u16_Sys_Time;

	uint8_t Index_Buffer;
	char RX_Buffer[BufferDimension];
	uint8_t str_length;
	char Mensaje[BufferDimension];
	int Baudrate;

	char FileTxt[20];

} General;

#endif /* DEFINICIONES_DEFINICIONES_H_ */
