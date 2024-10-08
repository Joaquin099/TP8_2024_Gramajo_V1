/*
 * pwm.h
 *
 *  Created on: 13 ago. 2024
 *      Author: joaqu
 */

#ifndef PWM_PWM_H_
#define PWM_PWM_H_

#include "main.h"
#include "Definiciones.h"

void EXTILine10_config(void);
void TIM2_config(void);
void EXTI15_10_IRQHandler(void);
void PWM_config(void);
void Menu_PWM(General *Gen);

#endif /* PWM_PWM_H_ */
