#include "sdcard_spi.h"

uint8_t is_sdhc = 0;
uint8_t sd_initialized = 0;

void SPI_Pinouts(void){
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	//P5 - SCK, P7 - MOSI,  AF output push-pull, 50 MHz
	GPIOA->CRL |= GPIO_CRL_MODE5 | GPIO_CRL_MODE7;
	GPIOA->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_CNF7);
	GPIOA->CRL |= GPIO_CRL_CNF5_1 | GPIO_CRL_CNF7_1;
	
	//P6 - MISO, AF output push-pull, 50 MHz	
	GPIOA->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6);
	GPIOA->CRL |= GPIO_CRL_CNF6_1;
	GPIOA->BSRR = GPIO_BSRR_BS6;
	
	//PA4 - CS, GP output push-pull, 50 MHz
	GPIOA->BSRR = GPIO_BSRR_BS4;
	GPIOA->CRL |= GPIO_CRL_MODE4;
	GPIOA->CRL &= ~GPIO_CRL_CNF4;
}

void SPI_Init(void){
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 &= ~(SPI_CR1_CPHA | SPI_CR1_CPOL | SPI_CR1_DFF | SPI_CR1_LSBFIRST | SPI_CR1_BIDIMODE);
	SPI1->CR1 |= SPI_CR1_SSI | SPI_CR1_SSM;
	SPI1->CR2 &= ~SPI_CR2_SSOE;
	
	SPI1->CR1 |= SPI_CR1_MSTR;
}
void SPI_LowSpeed(void){
	SPI1->CR1 &= ~SPI_CR1_SPE;
	
	//Baud Rate[2,0]: 72/256 ~ 281 kHz
	SPI1->CR1 |= SPI_CR1_BR;
	
	SPI1->CR1 |= SPI_CR1_SPE;
}

void SPI_HighSpeed(void){
	SPI1->CR1 &= ~SPI_CR1_SPE;
	
	//Baud Rate[2,0]: 72/8 ~ 9 MHz
	SPI1->CR1 &= ~SPI_CR1_BR;
	
	SPI1->CR1 |= SPI_CR1_BR_1;
	SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t SPI_transfer_data(uint8_t dt) {
	while (!(SPI1->SR & SPI_SR_TXE));
	*(__IO uint8_t*)&SPI1->DR = dt;
	while (!(SPI1->SR & SPI_SR_RXNE));
	return (*(__IO uint8_t*)&SPI1->DR);
}

void SD_CS_Low(void){
	GPIOA->BSRR = GPIO_BSRR_BR4;
}

void SD_CS_High(void){
	GPIOA->BSRR = GPIO_BSRR_BS4;
}

void SD_PowerUpSequence(void){
	SD_CS_High();
	SPI_LowSpeed();
	for (int i = 0; i < 10; i++){
		SPI_transfer_data(0xFF);
	}
}

uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg){
	SD_CS_Low();
	uint8_t r;
	uint32_t timeout = 50000;
    do {
		r = SPI_transfer_data(0xFF);
	} while (r != 0xFF && --timeout);
	
	SPI_transfer_data(cmd | 0x40);
	for (int i = 3; i >= 0; i--){
		SPI_transfer_data((arg >> (i * 8) & 0xFF));
	}
	
	uint8_t crc = 0xFF;
	if (cmd == CMD0) crc = 0x95;
	else if (cmd == CMD8) crc = 0x87;
	SPI_transfer_data(crc); 
	uint8_t count = 0;
	uint8_t rr;
	do {
		rr = SPI_transfer_data(0xFF);
		count++;
	} while ((rr & 0x80) && count < 8);
	
	return rr;
}

void SD_endCmd(void){
	SD_CS_High();
	SPI_transfer_data(0xFF);
}

uint8_t SD_Init(void){
	SPI_LowSpeed();
	SD_PowerUpSequence();
	
	uint8_t r1 = SD_SendCmd(CMD0, 0);
	SD_endCmd();
	if (r1 != 0x01) return 1;
	
	r1 = SD_SendCmd(CMD8, 0x1AA);
	uint8_t resp[4];
	for (int i = 0; i < 4; i++){
		resp[i] = SPI_transfer_data(0xFF);
	}
	SD_endCmd();
	if (resp[2] != 0x01 || resp[3] != 0xAA) return 1; // not SDv2
	
	uint32_t timeout = 50000;
	do {
		SD_SendCmd(CMD55, 0);
		SD_endCmd();
		
		r1 = SD_SendCmd(ACMD41, 0x40000000);
		SD_endCmd();
		
		timeout--;
	} while (r1 != 0x00 && timeout > 0);
	if (timeout == 0) return 1;
	
	r1 = SD_SendCmd(CMD58, 0);
	if (r1 != 0x00){
		SD_endCmd();
		return 1;
	}
	uint8_t ocr[4];
	for (int i = 0; i < 4; i++){
		ocr[i] = SPI_transfer_data(0xFF);
	}
	SD_endCmd();
	is_sdhc = (ocr[0] & 0x40) ? 1 : 0;
	
	SPI_HighSpeed();
	sd_initialized = 1;
	return 0;
}

uint8_t SD_ReadBlock(uint8_t *buf, uint32_t lba){
	if (!is_sdhc) lba <<= 9;
	
	uint8_t r17 = SD_SendCmd(CMD17, lba);
	if (r17 != 0){
		SD_endCmd();
		return 1;
	}
	
	uint8_t token;
	uint32_t timeout = 100000;
	do {
		token = SPI_transfer_data(0xFF);
		timeout--;
	} while (token != 0xFE && timeout > 0);
	if (token != 0xFE){
		SD_endCmd();
		return 1;
	}
	
	for (int i = 0; i < 512; i++){
		buf[i] = SPI_transfer_data(0xFF);
	}
	SPI_transfer_data(0xFF);
	SPI_transfer_data(0xFF);
	SD_endCmd();
	return 0;
}

uint8_t SD_WriteBlock(const uint8_t *buf, uint32_t lba){
	if (!is_sdhc) lba <<= 9;
	uint8_t r24 = SD_SendCmd(CMD24, lba);
	if (r24 != 0x00){
		SD_endCmd();
		return 1;
	}
	
	SPI_transfer_data(0xFF);
	SPI_transfer_data(0xFE);
	for (int i = 0; i < 512; i++){
		SPI_transfer_data(buf[i]);
	}
	
	SPI_transfer_data(0xFF);
	SPI_transfer_data(0xFF);
	
	uint8_t response = SPI_transfer_data(0xFF);
	if ((response & 0x1F) != 0x05){
		SD_endCmd();
		return 1;
	}
	
	uint32_t timeout = 500000;
	uint8_t busy;
	do {
		busy = SPI_transfer_data(0xFF);
		timeout--;
	} while (busy != 0xFF && timeout > 0);
	SD_endCmd();
	if (busy != 0xFF) return 1;
	return 0;
}



