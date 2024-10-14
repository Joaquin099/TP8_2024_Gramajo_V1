#include "main.h"
#include "Definiciones.h"
#include "SD.h"
#include "SPI.h"
#include "ff.h"	//FatFs Library
#include "string.h"

char FileName[20] = { "prueba.txt" };
char sdData[100] = { "esto es una prueba, HOLA?" };

FATFS  static FatFs;
FIL static File;
FRESULT static Res;
UINT static bitsWrite;


void InitSD(void) {
	if (FR_OK == (Res = f_mount(&FatFs, "0:", 1))){
		if (FR_OK == (Res = f_open(&File, FileName, FA_READ | FA_OPEN_APPEND | FA_WRITE))){
			if (FR_OK == (Res = f_write(&File, sdData, strlen(sdData), &bitsWrite))) {
				if (FR_OK == (Res = f_sync(&File))){
					bitsWrite = 0;
				}
			}
			Res = f_close(&File);
		}
		Res = f_unmount("0:");
	}
}



int8_t AddTxt(char *FileName, char *sdData) {

	if (FR_OK == (Res = f_mount(&FatFs, "0:", 1))){


		if (FR_OK == (Res = f_open(&File, FileName, FA_OPEN_APPEND | FA_WRITE))){

			if (FR_OK == (Res = f_write(&File, sdData, strlen(sdData), &bitsWrite))) {

				if (FR_OK == (Res = f_sync((FIL *) &File))){

					bitsWrite = 0;
				}
			}

			Res = f_close((FIL *) &File);
			Res = f_unmount("0:");
		}
	}
	return Res;
}

int8_t CloseFile(const char *FileName) {

	Res = f_close(&File);
	Res = f_unmount("0:");
}




