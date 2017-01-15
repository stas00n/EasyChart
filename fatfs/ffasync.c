#include "ffasync.h"
#include "timing.h"

ASYNCIO_T* as;
extern DSTATUS Stat;
extern BYTE CardType;
extern volatile uint32_t Timer1;


DRESULT adisk_read (
	BYTE drv,       // Physical drive number (0)
	BYTE *buff,     // Pointer to the data buffer to store read data
	DWORD sector,   // Start sector number (LBA
	BYTE count      // Sector count (1..255) 
)
{
  if (drv || !count)
    return RES_PARERR;
  
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;
  
  if (!(CardType & CT_BLOCK))
    sector *= 512;              // Convert to byte address if needed

  as->blocksRead = 0;
                    
if (count == 1)                 // Single block read
{
  if (send_cmd(CMD17, sector) != 0)
    return RES_ERROR;
  
  if(!Wait_DataMarker())
    return RES_ERROR;
  
  spi_dma_read(buff, 512);
  return RES_OK;
}
else                            // Multiple block read
{
  if (send_cmd(CMD18, sector) != 0)
    return RES_ERROR;
  
  if(!Wait_DataMarker())
    return RES_ERROR;
  
  spi_dma_read(buff, (count << 9));
  return RES_OK;
  //    send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
}
	//release_spi();

	//return /*count ? RES_ERROR :*/ RES_OK;
}

//-----------------------------------------------------------------------------
bool Wait_DataMarker() // Wait for data packet in timeout of 100ms
{
  BYTE token = 0xFF;
  Timer1 = 10;
  while((token == 0xFF)/* && Timer1*/) 
  {			                
    token = rcvr_spi();
  }
  if(token != 0xFE) 
    return false;       // If not valid data token, return with error
  else
    return true;
}

//-----------------------------------------------------------------------------
inline uint8_t rcvr_spi()
{
#define SPI1_DR8 *((__IO uint8_t*)&(SPI1->DR))
  SPI1_DR8 = 0xFF;
  while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_RXNE) == RESET) { ; }
  return SPI1_DR8;
}

//-----------------------------------------------------------------------------
FRESULT f_aread(FIL* fp, void* buff, UINT btr, UINT* br, ASYNCIO_T* asyncio)
{
  FRESULT res;
  FATFS *fs;
  DWORD clst, sect;
  FSIZE_t remain;
  UINT rcnt, cc, csect;
  BYTE *rbuff = (BYTE*)buff;
  
  as = asyncio;
  as->readComplete = 0;
  as->err = 0;
  as->buf = (uint8_t*)buff;
  
  *br = 0;	// Clear read byte counter
  res = validate(&fp->obj, &fs);				// Check validity of the file object
  if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK)
    LEAVE_FF(fs, res);	// Check validity 
  if (!(fp->flag & FA_READ))
    LEAVE_FF(fs, FR_DENIED); // Check access mode
  remain = fp->obj.objsize - fp->fptr;
  if (btr > remain)
    btr = (UINT)remain;		// Truncate btr by remaining bytes

//  for ( ;  btr;								/* Repeat until all data read */
//		rbuff += rcnt, fp->fptr += rcnt, *br += rcnt, btr -= rcnt)
//        {
  if (fp->fptr % SS(fs) == 0)
  {			/* On the sector boundary? */
    csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));	/* Sector offset in the cluster */
    if (csect == 0)
    {					/* On the cluster boundary? */
      if (fp->fptr == 0)
      {			/* On the top of the file? */
        clst = fp->obj.sclust;		/* Follow cluster chain from the origin */
      }
      else
      {						/* Middle or end of the file */
#if _USE_FASTSEEK
        if (fp->cltbl)
        {
          clst = clmt_clust(fp, fp->fptr);	/* Get cluster# from the CLMT */
        }
        else
#endif
        {
          clst = get_fat(&fp->obj, fp->clust);	/* Follow cluster chain on the FAT */
        }
      }
      if (clst < 2) ABORT(fs, FR_INT_ERR);
      if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
      fp->clust = clst;				/* Update current cluster */
    }
    sect = clust2sect(fs, fp->clust);	/* Get current sector */
    if (!sect) ABORT(fs, FR_INT_ERR);
    sect += csect;
    cc = btr / SS(fs);					/* When remaining bytes >= sector size, */
    if (cc)
    {							/* Read maximum contiguous sectors directly */
      if (csect + cc > fs->csize)
      {	/* Clip at cluster boundary */
        cc = fs->csize - csect;
      }
      as->blocksToRead = cc;
      if (adisk_read(fs->drv, rbuff, sect, cc) != RES_OK) ABORT(fs, FR_DISK_ERR);
//      #if !_FS_READONLY && _FS_MINIMIZE <= 2			/* Replace one of the read sectors with cached data if it contains a dirty sector */
//      #if _FS_TINY
//      if (fs->wflag && fs->winsect - sect < cc)
//      {
//        mem_cpy(rbuff + ((fs->winsect - sect) * SS(fs)), fs->win, SS(fs));
//      }
//#else
//      if ((fp->flag & FA_DIRTY) && fp->sect - sect < cc)
//      {
//        mem_cpy(rbuff + ((fp->sect - sect) * SS(fs)), fp->buf, SS(fs));
//      }
//      #endif
//      #endif
      	rcnt = SS(fs) * cc;				// Number of bytes transferred */
      //	continue;
      
    }
    //#if !_FS_TINY
    //			if (fp->sect != sect) {			/* Load data sector if not in cache */
    //#if !_FS_READONLY
    //				if (fp->flag & FA_DIRTY) {		/* Write-back dirty sector cache */
    //					if (disk_write(fs->drv, fp->buf, fp->sect, 1) != RES_OK) ABORT(fs, FR_DISK_ERR);
    //					fp->flag &= (BYTE)~FA_DIRTY;
    //				}
    //#endif
    //				if (disk_read(fs->drv, fp->buf, sect, 1) != RES_OK)	ABORT(fs, FR_DISK_ERR);	/* Fill sector cache */
    //			}
    //#endif
    fp->sect = sect;
  }
//  rcnt = SS(fs) - (UINT)fp->fptr % SS(fs);	/* Number of bytes left in the sector */
//  if (rcnt > btr) rcnt = btr;					/* Clip it by btr if needed */
//#if _FS_TINY
//		if (move_window(fs, fp->sect) != FR_OK) ABORT(fs, FR_DISK_ERR);	/* Move sector window */
//		mem_cpy(rbuff, fs->win + fp->fptr % SS(fs), rcnt);	/* Extract partial sector */
//#else
//		mem_cpy(rbuff, fp->buf + fp->fptr % SS(fs), rcnt);	/* Extract partial sector */
//#endif
//  }
//  rbuff += rcnt;
  fp->fptr += rcnt;
  *br += rcnt;
  btr -= rcnt;
                
         
  LEAVE_FF(fs, FR_OK);
  return FR_OK;
}

/* Copy memory to memory */
static
void mem_cpy (void* dst, const void* src, UINT cnt)
{
  BYTE* s = (BYTE*)src;
  BYTE* d = (BYTE*)dst;
  while (cnt--)
  {
    *d++ = *s++;
  }
}


//------------------------------------------------------------------------------
extern "C" void DMA1_Channel2_3_IRQHandler()
{
  DMA_ClearFlag(DMA1_FLAG_GL2 |
                DMA1_FLAG_TC2 |
                DMA1_FLAG_HT2 |
                DMA1_FLAG_TE2);
  
  
  DMA_Cmd(DMA_Channel_SPI_SD_RX, DISABLE);
  DMA_Cmd(DMA_Channel_SPI_SD_TX, DISABLE);
  SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);
  
  // Discard crc
  rcvr_spi();
  rcvr_spi();

  uint32_t blocksRead = ++as->blocksRead;
  if(blocksRead < as->blocksToRead)
    Dma_Cont_Rd();
  else
    Dma_Stop_Rd();
}
//------------------------------------------------------------------------------
void Dma_Cont_Rd()
{
  if(!Wait_DataMarker())
  {
    as->err = 1;
    as->readComplete = 1;
    return;
  }

  as->buf += 512;
  DMA_Channel_SPI_SD_RX->CMAR += 512;
  DMA_Channel_SPI_SD_RX->CNDTR = 512;
  DMA_Channel_SPI_SD_TX->CNDTR = 512;

  // Enable DMA RX Channel
  DMA_Cmd(DMA_Channel_SPI_SD_RX, ENABLE);
  // Enable DMA TX Channel
  DMA_Cmd(DMA_Channel_SPI_SD_TX, ENABLE);
  // Enable SPI TX/RX request
  SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);
}

//------------------------------------------------------------------------------
void Dma_Stop_Rd()
{
  DMA_DeInit(DMA_Channel_SPI_SD_RX);
  DMA_DeInit(DMA_Channel_SPI_SD_TX);
  SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);

  send_cmd(CMD12, 0);
  as->readComplete = 1;
}






