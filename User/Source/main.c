#include "main.h"

int main(){
	rcc_init();
	SPI_Pinouts();
	SPI_Init();
	sd_initialized = SD_Init();
	while(1);
}
	