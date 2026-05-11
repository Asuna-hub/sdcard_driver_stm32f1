#include "main.h"

int main(){
	rcc_init();
	SPI_Pinouts();
	SPI_Init();
	SPI_LowSpeed();
	SD_PowerUpSequence();
	
	uint8_t Reg1 = SD_SendCmd(CMD0, 0);
	SD_endCmd();
	
	uint8_t Reg7 = SD_SendCmd(CMD8, 0x1AA);
	uint8_t resp[4];
	resp[0] = SPI_transfer_data(0xFF);
	resp[1] = SPI_transfer_data(0xFF);
	resp[2] = SPI_transfer_data(0xFF);
	resp[3] = SPI_transfer_data(0xFF);
	SD_endCmd();
	
	uint32_t timeout44 = 50000;
	uint8_t ASMD44;
	do {
		SD_SendCmd(CMD55, 0);
		SD_endCmd();
		
		ASMD44 = SD_SendCmd(ACMD41, 0x40000000);
		SD_endCmd();
		
		timeout44--;
	} while (ASMD44 != 0x00 && timeout44 > 0);
	while(1);
}
	