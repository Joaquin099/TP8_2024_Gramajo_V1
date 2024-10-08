/*-----------------------------------Includes-----------------------------------*/
#include "LCD.h"
#include "DAC.h"
#include "ADC.h"
#include "stdio.h"
#include "pwm.h"
#include "Display_LCD.h"
#include "Definiciones.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_adc.h"

/*-----------------------------------Variables----------------------------------*/
char str_texto[30];
uint16_t numero_value = 0;
static uint16_t num_DAC = 0;
static uint8_t constante = 0;


char *texto[]={
		"Tecla pres.",
		"Backlight",
		"FUN Control",
		"Generador DAC",
		"Medicion ADC",
		""
};

typedef enum{
	Menu,
	Tecla_press,
	Backlight,
	PWM,
	Generador_DAC,
	Medicion_ADC,
	Mantener
}Estados;

Estados EST = Menu;

/*------------------------------------Main-------------------------------------*/
void Init_Menu(General *Gen){

	switch(EST){
	case Menu:
		if(Gen->Key != 'A' && Gen->Key != 'B' && Gen->Key == '\0'){
				return;
			}

		if(constante == 0 && Gen->Key == 'A'){
			EST = Menu;
			break;
		}
		if(constante == 4 && Gen->Key == 'B'){
			EST = Menu;
			break;
		}

		if(Gen->Key == 'B'){
			constante++;
			LCD_clrscr();
			LCD_WriteString(0, 0, ">");
			LCD_WriteString(1, 0, texto[constante]);
			LCD_WriteString(0, 1, texto[constante+1]);
			EST = Mantener;
			break;
		}

		if(Gen->Key == 'A'){
			constante=constante-1;
			LCD_clrscr();
			LCD_WriteString(0, 0, ">");
			LCD_WriteString(1, 0, texto[constante]);
			LCD_WriteString(0, 1, texto[constante+1]);
			EST = Mantener;
			break;
		}

		if(Gen->Key == 'C'){
			LCD_clrscr();
			if(constante == 0){
				EST = Tecla_press;
				break;
			}else if(constante == 1){
				EST = Backlight;
				break;
			}else if(constante == 2){
				EST = PWM;
				break;
			}else if(constante == 3){
				EST = Generador_DAC;
				break;
			}else if(constante == 4){
				EST = Medicion_ADC;
				break;
			}
		}

		if(Gen->Key == 'D'){
			LCD_clrscr();
			LCD_WriteString(0, 0, ">");
			LCD_WriteString(1, 0, texto[constante]);
			LCD_WriteString(0, 1, texto[constante+1]);
			EST = Menu;
		}
		break;

	case Mantener:
		if(Gen->Key == 'A' || Gen->Key == 'B'){
			EST = Mantener;
		}else{
			EST = Menu;
		}
		break;

	case Tecla_press:
		if(Gen->Key == 'D'){
			EST = Menu;
			break;
		}
		sprintf(str_texto, "%c", Gen->Key);
		LCD_WriteString(0, 0, "Tecla pres.: ");
		LCD_gotoxy(13,0);
		LCD_WriteString(13, 0, str_texto);
		break;

	case Backlight:
		if(Gen->Key == 'D'){
			EST = Menu;
			break;
		}
		if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_13)){
			LCD_clrscr();
			LCD_WriteString(0, 0, "Backlight: ON");
		}else{
			LCD_clrscr();
			LCD_WriteString(0, 0, "Backlight: OFF");
		}
		break;

	case PWM:
		if(Gen->Key == 'D'){
			EST = Menu;
			break;
		}
		//if(bandera == 0){
			Menu_PWM(Gen);
			//bandera = 1;
		//}
		break;

	case Generador_DAC:

		if(Gen->Key == 'D'){
			EST = Menu;
			break;
		}
		if((Gen->Key >= '0') && (Gen->Key <= '9')){
			numero_value = (Gen->Key - '0') * 100;
			num_DAC = (uint16_t) (((numero_value/3300.0)*4095.0));
			writeDAC(num_DAC);
		}
		LCD_clrscr();
		LCD_WriteString(0, 0, "DAC: ");
		sprintf(str_texto, "%d", numero_value);
		LCD_WriteString(5, 0, str_texto);
		LCD_WriteString(9, 0, "mV");
		break;

	case Medicion_ADC:
		if(Gen->Key == 'D'){
			EST = Menu;
			break;
		}
		LCD_clrscr();
		ADC_Muestreo(Gen);

	    LCD_WriteString(0, 0, "Temp: ");
		sprintf(str_texto, "%d", Gen->Temp);
		LCD_WriteString(6, 0, str_texto);
		LCD_WriteString(13, 0, "C");

		LCD_WriteString(0, 1, "Temp: ");
		sprintf(str_texto, "%d", Gen->Temp + 273);
		LCD_WriteString(6, 1, str_texto);
		LCD_WriteString(13, 1, "K");

		break;
	}
}
