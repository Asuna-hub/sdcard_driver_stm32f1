#ifndef SPI_H
#define SPI_H

#include "main.h"

//defines
#define CMD0 0
#define CMD8 8
#define CMD55 55
#define ACMD41 41
#define CMD58 58

void SPI_Pinouts(void);
void SPI_Init(void);
void SPI_LowSpeed(void);
void SPI_HighSpeed(void);
uint8_t SPI_transfer_data(uint8_t dt);
void SD_CS_Low(void);
void SD_CS_High(void);

//sd
void SD_PowerUpSequence(void);
uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg);
void SD_endCmd(void);

#endif