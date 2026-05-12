#include "main.h"

int main(){
	rcc_init();
	SPI_Pinouts();
	SPI_Init();
	SPI_LowSpeed();
	SD_PowerUpSequence();
	
	uint8_t r1 = SD_SendCmd(CMD0, 0);
	SD_endCmd();
	
	uint8_t r7 = SD_SendCmd(CMD8, 0x1AA);
	uint8_t resp[4];
	for (int i = 0; i < 4; i++){
		resp[0] = SPI_transfer_data(0xFF);
	}
	SD_endCmd();
	
	uint32_t timeout = 50000;
	do {
		SD_SendCmd(CMD55, 0);
		SD_endCmd();
		
		uint8_t ASMD44 = SD_SendCmd(ACMD41, 0x40000000);
		SD_endCmd();
		
		timeout--;
	} while (ASMD44 != 0x00 && timeout > 0);
	
	uint8_t r58 = SD_SendCmd(CMD58, 0);
	uint8_t ocr[4];
	for (int i = 0; i < 4; i++){
		ocr[i] = SPI_transfer_data(0xFF);
	}
	SD_endCmd();
	is_sdhc = (ocr[0] & 40) ? 0 : 1;
	
	
	while(1);
}
	