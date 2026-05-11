#include "sdcard_spi.h"

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
	
	SPI_transfer_data((arg >> 24) & 0xFF);
	SPI_transfer_data((arg >> 16) & 0xFF);
	SPI_transfer_data((arg >> 8) & 0xFF);
	SPI_transfer_data(arg & 0xFF);
	
	uint8_t crc = 0xFF;
	if (cmd == CMD0) crc = 0x95;
	else if (cmd == CMD8) crc = 0x87;
	SPI_transfer_data(crc); 
	uint8_t count = 0;
	uint8_t r1;
	do {
		r1 = SPI_transfer_data(0xFF);
		count++;
	} while ((r1 & 0x80) && count < 8);
	
	return r1;
}

void SD_endCmd(void){
	SD_CS_High();
	SPI_transfer_data(0xFF);
}
