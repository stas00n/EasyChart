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
  as->blocksToRead = count;     // Initialize sector cnt
  
  if (drv || !count)
    return RES_PARERR;
  
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;
  
  if (!(CardType & CT_BLOCK))
    sector *= 512;              // Convert to byte address if needed
  
  as->blocksRead = 0;
  
  if (count == 1)               // Single block read
  {
    if (send_cmd(CMD17, sector) != 0)
      return RES_ERROR;
    
    if(!Wait_DataMarker())
      return RES_ERROR;
    
    spi_dma_read(buff);
    return RES_OK;
  }
  else                          // Multiple block read
  {
    if (send_cmd(CMD18, sector) != 0)
      return RES_ERROR;
    
    if(!Wait_DataMarker())
      return RES_ERROR;
    
    spi_dma_read(buff);
    return RES_OK;
    
  }
}

//-----------------------------------------------------------------------------
bool Wait_DataMarker() // Wait for data packet in timeout of 100ms
{
  BYTE token = 0xFF;
  Timer1 = 10;
  while((token == 0xFF) && Timer1) 
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
FRESULT f_aread(FIL* fp, void* buff, UINT btr, /*UINT* br,*/ ASYNCIO_T* asyncio)
{
  FRESULT res;
  FATFS *fs;
  DWORD clst, sect;
 // FSIZE_t remain;
  UINT /*rcnt,*/ cc, csect;
 // BYTE *rbuff = (BYTE*)buff;
  
  // Initialize async struct
  as = asyncio;
  as->readComplete = 0;
  as->bytesToread = btr;
  as->result = FR_BUSY;
  as->buf = (uint8_t*)buff;
  as->fp = fp;
  
  //*br = 0;	        
  as->bytesRead = 0;  // Clear read byte counter
  
  
  
  res = validate(&fp->obj, &fs);        // Check validity of the file object
  if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK)
    LEAVE_FF(fs, res);
  
  if (!(fp->flag & FA_READ))            // Check access mode
    LEAVE_FF(fs, FR_DENIED);
  
  if(fp->fptr == 0)                     // Start of file, 
    as->bytesCached = 0;                // there're no cached data
  
  uint32_t remain = fp->obj.objsize - fp->fptr;  // Remaining data count
  if (as->bytesToread > remain)
  {
    btr = remain;                       // Truncate btr by remaining bytes
    as->bytesToread = remain;
  }
  
  if(as->bytesCached)                   // Write cached data if any
  {
    uint32_t ncpy = as->bytesToread;    // Count...
    if(ncpy > as->bytesCached)
      ncpy = as->bytesCached;
    
    uint8_t* pCache = fp->buf + SS(fs) - as->bytesCached;       // Find...
    memcpy((void*)as->buf, pCache, ncpy);                       // Write...
    as->bytesCached -= ncpy;
    as->bytesToread -= ncpy;
    fp->fptr += ncpy;
    as->buf += ncpy;
    
    if(as->bytesToread == 0)            // That's all?
    {
      as->result = FR_OK;
      as->readComplete = 1;
      LEAVE_FF(fs, FR_OK);
    }
  }

  if (fp->fptr % SS(fs) == 0)           // On the sector boundary?
  {			
    csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));// Sector offset in the cluster
    if (csect == 0)                     // On the cluster boundary?
    {					
      if (fp->fptr == 0)                // On the top of the file?
      {	
        clst = fp->obj.sclust;		// Follow cluster chain from the origin
      }
      else                              // Middle or end of the file
      {						
#if _USE_FASTSEEK
        if (fp->cltbl)
        {
          clst = clmt_clust(fp, fp->fptr);	// Get cluster# from the CLMT
        }
        else
#endif
        {
          clst = get_fat(&fp->obj, fp->clust);	// Follow cluster chain on the FAT
        }
      }
      
      if (clst < 2) ABORT(fs, FR_INT_ERR);
      if (clst == 0xFFFFFFFF) ABORT(fs, FR_DISK_ERR);
      fp->clust = clst;	                // Update current cluster
    }
    sect = clust2sect(fs, fp->clust);	// Get current sector
    if (!sect)
      ABORT(fs, FR_INT_ERR);
    sect += csect;
    
    cc = as->bytesToread / SS(fs);	 // When remaining bytes >= sector size,
    if (csect + cc > fs->csize)       // Clip at cluster boundary
    {	
      cc = fs->csize - csect;
    }
    as->remFullSects = cc;
    as->remBytes = as->bytesToread - cc * SS(fs);
    if(as->remBytes >= SS(fs))
      as->remBytes = 0;
    if(as->remBytes)
      cc++;
    if (cc)   // Read maximum contiguous sectors directly
    {							
      if (adisk_read(fs->drv, (BYTE*)as->buf, sect, cc) != RES_OK)
        ABORT(fs, FR_DISK_ERR);
    }
    
    else                        // Remaining bytes < sector size,
    {
      if (adisk_read(fs->drv, fp->buf, sect, 1) != RES_OK)
        ABORT(fs, FR_DISK_ERR);
    }
  }
     
  LEAVE_FF(fs, FR_OK);
}

//------------------------------------------------------------------------------
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
  
  as->remFullSects--;
  
  

  as->fp->fptr += 512;
  as->bytesToread -= 512;
  //as->fp->sect ++;
  as->bytesRead += 512;
  uint32_t blocksRead = ++as->blocksRead;
  
  if(as->remFullSects)
    Dma_Cont_Rd();
  else if(as->remBytes)
    Dma_Cont_Rd(as->fp->buf);
  else
    Dma_Stop_Rd();
}
//------------------------------------------------------------------------------
void Dma_Cont_Rd(uint8_t* cache)
{
  if(!Wait_DataMarker())
  {
    Dma_Stop_Rd();
    as->result = FR_DISK_ERR;
    return;
  }
  if(cache != NULL)     // Read in cache
  {
    DMA_Channel_SPI_SD_RX->CMAR = (uint32_t)cache;
    as->bytesCached = 512;
  }
  else                  // Read directly in buffer
  {
    as->buf += 512;
    DMA_Channel_SPI_SD_RX->CMAR += 512;
  }
  
  // Reload data counters
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

  send_cmd(CMD12, 0);           // STOP_TRANSMISSION
  release_spi();
  
  if(as->bytesCached)           // Write remaining data
  {
    as->fp->fptr -= 512;
    as->bytesRead -=512;
    //as->bytesToread -= 512;
    
    uint32_t ncpy = as->bytesToread;
    ncpy -= as->bytesRead;
    memcpy((void*)as->buf, as->fp->buf, ncpy);
    as->fp->fptr += ncpy;
    as->bytesCached -= ncpy;
    as->bytesRead += ncpy;
    as->buf += ncpy;
  }
  as->result = FR_OK;
  as->readComplete = 1;
}








