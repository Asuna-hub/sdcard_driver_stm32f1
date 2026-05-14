#include "main.h"
FATFS fs;
FIL fil;
UINT bw;

int main(){
	rcc_init();
	SPI_Pinouts();
	SPI_Init();
	SD_Init();
	
	f_mount(&fs,"", 1);
	f_open(&fil, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
	f_write(&fil, "Hello SD!", 11, &bw);
	f_close(&fil);
	while(1);
}
	