#include "LCD.h"
#include "Display_LCD.h"
#include "Definiciones.h"

void Init_Display(void) {

	LCD_init();
	LCD_clrscr();

	LCD_WriteString(0, 0, "Tec. Digitales 2");
	LCD_WriteString(0, 1, "Laboratorio N° 7");

	delay_ms(2000);

	LCD_clrscr();

	LCD_WriteString(0, 0, ">Tecla Press");
	LCD_WriteString(0, 1, "Backlight");
}
