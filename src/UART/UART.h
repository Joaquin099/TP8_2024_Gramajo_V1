#include "Definiciones.h"

#ifndef UART_UART_H_
#define UART_UART_H_

void UART4_Config(General *Gen);
void usart2SendData(uint16_t *Buff, uint8_t Length);
void readMessage(General *Gen);
void Init_MenuUART(General *Gen);
void UART4_IRQHandler(General *Gen);

#endif /* UART_UART_H_ */
