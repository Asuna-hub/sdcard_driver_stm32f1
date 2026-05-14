/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "main.h"
#include "diskio.h"		/* Declarations FatFs MAI */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if (pdrv != 0){
		return STA_NOINIT;
	}
	else if (sd_initialized == 1)
		return 0;
	else
		return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	if (pdrv != 0)
		return STA_NOINIT;
	uint8_t r = SD_Init();
	if (r == 0)
		return 0;
	else
		return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if (pdrv != 0)
		return RES_PARERR;
	for (int i = 0; i < count; i++){
		if (SD_ReadBlock(buff, sector) != 0)
			return RES_ERROR;
		buff += 512;
		sector++;
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if (pdrv != 0) return RES_PARERR;
	for (int i = 0; i < count; i++){
		if (SD_WriteBlock(buff, sector) != 0)
			return RES_ERROR;
		buff += 512;
		sector++;
	}
	return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if (pdrv != 0) return RES_PARERR;
	
	switch(cmd){
		case CTRL_SYNC:
			return RES_OK;
		
		case GET_SECTOR_SIZE:
			*(WORD*)buff = 512;
			return RES_OK;
		
		case GET_SECTOR_COUNT:
			*(LBA_t*)buff = 62500000;
			return RES_OK;
		
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 1;
			return RES_OK;
		
		default:
			return RES_PARERR;
	}
}

DWORD get_fattime(){
	    return ((DWORD)(2025 - 1980) << 25)
         | ((DWORD)1 << 21)
         | ((DWORD)1 << 16);
}

