/*
 * portSPI.c
 *
 *  Created on: 26/02/2019
 *      Author: HP
 */

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "SPI.h"

void initSPI(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	/*!< Enable the SPI clock */
	uSD_SPI_CLK_INIT(uSD_SPI_CLK, ENABLE);

	/*!< Enable GPIO clocks */
	RCC_AHB1PeriphClockCmd(uSD_SPI_SCK_GPIO_CLK | uSD_SPI_MISO_GPIO_CLK |
			uSD_SPI_MOSI_GPIO_CLK | uSD_CS_GPIO_CLK, ENABLE);

	/*!< SPI pins configuration *************************************************/

	/*!< Connect SPI pins to AF6 */
	GPIO_PinAFConfig(uSD_SPI_SCK_GPIO_PORT, uSD_SPI_SCK_SOURCE, uSD_SPI_SCK_AF);
	GPIO_PinAFConfig(uSD_SPI_MISO_GPIO_PORT, uSD_SPI_MISO_SOURCE, uSD_SPI_MISO_AF);
	GPIO_PinAFConfig(uSD_SPI_MOSI_GPIO_PORT, uSD_SPI_MOSI_SOURCE, uSD_SPI_MOSI_AF);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	/*!< SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = uSD_SPI_SCK_PIN;
	GPIO_Init(uSD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< SPI MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin =  uSD_SPI_MOSI_PIN;
	GPIO_Init(uSD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/*!< SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin =  uSD_SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(uSD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);


	/*!< Configure sFLASH Card CS pin in output pushpull mode ********************/
	GPIO_InitStructure.GPIO_Pin = uSD_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(uSD_CS_GPIO_PORT, &GPIO_InitStructure);

	/*!< Deselect the FLASH: Chip Select high */
	uSD_CS_HIGH();

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(uSD_SPI, &SPI_InitStructure);

	/*!< Enable the uSD_SPI  */
	SPI_Cmd(uSD_SPI, ENABLE);
}



void spiSpeed(uint16_t Preescaler, uint16_t CPOL, uint16_t CPHA){

	SPI_InitTypeDef  SPI_InitStructure;

	SPI_Cmd(uSD_SPI, DISABLE);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = CPOL;
	SPI_InitStructure.SPI_CPHA = CPHA;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = Preescaler;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(uSD_SPI, &SPI_InitStructure);

	SPI_Cmd(uSD_SPI, ENABLE);
}


uint8_t SPI_ReadWrite_Byte(unsigned char byte){

	while (SPI_I2S_GetFlagStatus(uSD_SPI, SPI_I2S_FLAG_TXE) == RESET); //wait buffer empty

	SPI_I2S_SendData(uSD_SPI, byte);

	while (SPI_I2S_GetFlagStatus(uSD_SPI, SPI_I2S_FLAG_BSY) == SET); //wait finish sending

	while (SPI_I2S_GetFlagStatus(uSD_SPI, SPI_I2S_FLAG_RXNE) == RESET); //wait finish sending

	return SPI_I2S_ReceiveData(uSD_SPI);
}
