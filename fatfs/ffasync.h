#ifndef _FFASYNC_H_
#define _FFASYNC_H_

#include "ff.h"
#include "ffasync.h"
#include "integer.h"
#include <stdint.h>
#include <string.h>
#include "diskio.h"
#include "stm32f0xx_conf.h"

// TODO: unvolatile if not need
typedef struct
{
  uint32_t bytesToread;
  uint32_t bytesRead;
  uint8_t* buf;
  volatile FRESULT result;
  uint32_t      remBytes;
  FIL* fp;
  uint32_t sect;
  uint32_t clust;
  uint32_t clustPrev;
  uint32_t bytesCached;
}ASYNCIO_T;

void Dma_Cont_Rd(uint8_t* cache = NULL);
void Dma_Stop_Rd();

// Definitions for MMC/SDC command
#define CMD0	(0x40+0)	// GO_IDLE_STATE
#define CMD1	(0x40+1)	// SEND_OP_COND (MMC)
#define ACMD41	(0xC0+41)	// SEND_OP_COND (SDC)
#define CMD8	(0x40+8)	// SEND_IF_COND
#define CMD9	(0x40+9)	// SEND_CSD
#define CMD10	(0x40+10)	// SEND_CID
#define CMD12	(0x40+12)	// STOP_TRANSMISSION
#define ACMD13	(0xC0+13)	// SD_STATUS (SDC)
#define CMD16	(0x40+16)	// SET_BLOCKLEN
#define CMD17	(0x40+17)	// READ_SINGLE_BLOCK
#define CMD18	(0x40+18)	// READ_MULTIPLE_BLOCK
#define CMD23	(0x40+23)	// SET_BLOCK_COUNT (MMC)
#define ACMD23	(0xC0+23)	// SET_WR_BLK_ERASE_COUNT (SDC)
#define CMD24	(0x40+24)	// WRITE_BLOCK
#define CMD25	(0x40+25)	// WRITE_MULTIPLE_BLOCK
#define CMD55	(0x40+55)	// APP_CMD 
#define CMD58	(0x40+58)	// READ_OCR

#if defined(STM32F030)
 #define CARD_SUPPLY_SWITCHABLE   0
 //#define GPIO_PWR                 GPIOD
 //#define RCC_APB2Periph_GPIO_PWR  RCC_APB2Periph_GPIOD
 //#define GPIO_Pin_PWR             GPIO_Pin_10
 //#define GPIO_Mode_PWR            GPIO_Mode_Out_OD /* pull-up resistor at power FET */
 #define SOCKET_WP_CONNECTED      0
 #define SOCKET_CP_CONNECTED      0
 #define SPI_SD                   SPI1
 #define GPIO_CS                  GPIOA
 #define RCC_AHBPeriph_GPIO_CS   RCC_AHBPeriph_GPIOA
 #define GPIO_Pin_CS              GPIO_Pin_4
 #define DMA_Channel_SPI_SD_RX    DMA1_Channel2
 #define DMA_Channel_SPI_SD_TX    DMA1_Channel3
 #define DMA_FLAG_SPI_SD_TC_RX    DMA1_FLAG_TC2
 #define DMA_FLAG_SPI_SD_TC_TX    DMA1_FLAG_TC3
 #define GPIO_SPI_SD              GPIOA
 #define GPIO_Pin_SPI_SD_SCK      GPIO_Pin_5
 #define GPIO_Pin_SPI_SD_MISO     GPIO_Pin_6
 #define GPIO_Pin_SPI_SD_MOSI     GPIO_Pin_7
 #define RCC_APBPeriphClockCmd_SPI_SD  RCC_APB2PeriphClockCmd
 #define RCC_APBPeriph_SPI_SD     RCC_APB2Periph_SPI1
 /* - for SPI1 and full-speed APB2: 72MHz/4 */
 #define SPI_BaudRatePrescaler_SPI_SD  SPI_BaudRatePrescaler_8

#endif



#define	ABORT(fs, res)		{ fp->err = (BYTE)(res); LEAVE_FF(fs, res); }


/* Reentrancy related */
#if _FS_REENTRANT
#if _USE_LFN == 1
#error Static LFN work area cannot be used at thread-safe configuration
#endif
#define	ENTER_FF(fs)		{ if (!lock_fs(fs)) return FR_TIMEOUT; }
#define	LEAVE_FF(fs, res)	{ unlock_fs(fs, res); return res; }
#else
#define	ENTER_FF(fs)
#define LEAVE_FF(fs, res)	return res
#endif

/* Definitions of sector size */
#if (_MAX_SS < _MIN_SS) || (_MAX_SS != 512 && _MAX_SS != 1024 && _MAX_SS != 2048 && _MAX_SS != 4096) || (_MIN_SS != 512 && _MIN_SS != 1024 && _MIN_SS != 2048 && _MIN_SS != 4096)
#error Wrong sector size configuration
#endif
#if _MAX_SS == _MIN_SS
#define	SS(fs)	((UINT)_MAX_SS)	/* Fixed sector size */
#else
#define	SS(fs)	((fs)->ssize)	/* Variable sector size */
#endif


FRESULT f_aread(FIL* fp, void* buff, UINT btr,/* UINT* br,*/ ASYNCIO_T* asyncio);

DRESULT adisk_read(BYTE drv, BYTE* buff, DWORD sector, BYTE count);
bool Wait_DataMarker();
void mem_cpy (void* dst, const void* src, UINT cnt);
DWORD clust2sect (	/* !=0:Sector number, 0:Failed (invalid cluster#) */
	FATFS* fs,		/* File system object */
	DWORD clst		/* Cluster# to be converted */
);
DWORD get_fat (	/* 0xFFFFFFFF:Disk error, 1:Internal error, 2..0x7FFFFFFF:Cluster status */
	_FDID* obj,	/* Corresponding object */
	DWORD clst	/* Cluster number to get the value */
);

DWORD clmt_clust (	/* <2:Error, >=2:Cluster number */
	FIL* fp,		/* Pointer to the file object */
	FSIZE_t ofs		/* File offset to be converted to cluster# */
);

extern "C" 
{
  BYTE send_cmd (BYTE cmd, DWORD arg);
  void spi_dma_read(BYTE* buff, UINT btr = 512);
  void release_spi (void);
}
extern "C"
void stm32_dma_transfer(
	BOOL receive,		/* FALSE for buff->SPI, TRUE for SPI->buff               */
	const BYTE *buff,	/* receive TRUE  : 512 byte data block to be transmitted
						   receive FALSE : Data buffer to store received data    */
	UINT btr 			/* receive TRUE  : Byte count (must be multiple of 2)
						   receive FALSE : Byte count (must be 512)              */
);

uint8_t rcvr_spi();



extern "C"
FRESULT validate (	/* Returns FR_OK or FR_INVALID_OBJECT */
	_FDID* obj,		/* Pointer to the _OBJ, the 1st member in the FIL/DIR object, to check validity */
	FATFS** fs		/* Pointer to pointer to the owner file system object to return */
);
#endif /* _FFASYNC_H_ */