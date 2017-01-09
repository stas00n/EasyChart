#include "ffasync.h"

DRESULT adisk_read (
	BYTE drv,			// Physical drive number (0)
	BYTE *buff,			// Pointer to the data buffer to store read data
	DWORD sector,		// Start sector number (LBA
	BYTE count			// Sector count (1..255) 
)
{
//	if (drv || !count) return RES_PARERR;
//	if (Stat & STA_NOINIT) return RES_NOTRDY;
//
//	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */
//
if (count == 1) 	/* Single block read */
{
  if (send_cmd(CMD17, sector) == 0)	 /* READ_SINGLE_BLOCK */
  {
    spi_dma_read(buff, 512);

      count = 0;

  }
}
else				/* Multiple block read */
{
  if (send_cmd(CMD18, sector) == 0) 	/* READ_MULTIPLE_BLOCK */
  {
    spi_dma_read(buff, (count << 9));
    count = 0;
        send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
  }
}
	//release_spi();

	return count ? RES_ERROR : RES_OK;
}

