#ifndef SD_MENU_SD_H_
#define SD_MENU_SD_H_

void InitSD(void);
int8_t NewFile (const char *FileName);
int8_t OpenCreateFile (const char *FileName);
int8_t AddTxt(char *FileName, char *sdData);
int8_t CloseFile (const char *FileName);

#endif /* SD_MENU_SD_H_ */
