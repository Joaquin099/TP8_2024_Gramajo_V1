/*
 * portSPI.h
 *
 *  Created on: 26/02/2019
 *      Author: HP
 */

#ifndef FATFS_PORTSPI_H_
#define FATFS_PORTSPI_H_

#include "stm32f4xx_spi.h"

#define uSD_SPI                    		SPI3
#define uSD_SPI_CLK                     RCC_APB1Periph_SPI3
#define uSD_SPI_CLK_INIT                RCC_APB1PeriphClockCmd

#define uSD_SPI_SCK_PIN                 GPIO_Pin_10
#define uSD_SPI_SCK_GPIO_PORT           GPIOC
#define uSD_SPI_SCK_GPIO_CLK            RCC_AHB1Periph_GPIOC
#define uSD_SPI_SCK_SOURCE              GPIO_PinSource10
#define uSD_SPI_SCK_AF                  GPIO_AF_SPI3

#define uSD_SPI_MISO_PIN                GPIO_Pin_11
#define uSD_SPI_MISO_GPIO_PORT          GPIOC
#define uSD_SPI_MISO_GPIO_CLK           RCC_AHB1Periph_GPIOC
#define uSD_SPI_MISO_SOURCE             GPIO_PinSource11
#define uSD_SPI_MISO_AF                 GPIO_AF_SPI3

#define uSD_SPI_MOSI_PIN                GPIO_Pin_12
#define uSD_SPI_MOSI_GPIO_PORT          GPIOC
#define uSD_SPI_MOSI_GPIO_CLK           RCC_AHB1Periph_GPIOC
#define uSD_SPI_MOSI_SOURCE             GPIO_PinSource12
#define uSD_SPI_MOSI_AF                 GPIO_AF_SPI3

#define uSD_CS_PIN                    	GPIO_Pin_13
#define uSD_CS_GPIO_PORT              	GPIOC
#define uSD_CS_GPIO_CLK               	RCC_AHB1Periph_GPIOC


#define uSD_CS_LOW()      		 		GPIO_ResetBits(uSD_CS_GPIO_PORT, uSD_CS_PIN)
#define uSD_CS_HIGH()      				GPIO_SetBits(uSD_CS_GPIO_PORT, uSD_CS_PIN)


void initSPI(void);
void spiSpeed(uint16_t Preescaler, uint16_t CPOL, uint16_t CPHA);
uint8_t SPI_ReadWrite_Byte(unsigned char byte);

#endif /* FATFS_PORTSPI_H_ */
