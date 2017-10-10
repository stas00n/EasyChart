#include "ffasync.h"

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
   
  if (count == 1)               // Single block read
  {
    if (send_cmd(/*CMD17*/CMD18, sector) != 0)
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
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI1_DR8 = 0xFF;
  while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI1_DR8;
}

//-----------------------------------------------------------------------------
FRESULT f_aread(FIL* fp, void* buff, UINT btr, /*UINT* br,*/ ASYNCIO_T* asyncio)
{
  FRESULT res;
  FATFS* fs;
  DWORD clst, sect;
  UINT  cc, csect;

  
  // Initialize async struct
  as = asyncio;
  as->bytesToread = btr;
  as->result = FR_BUSY;
  as->buf = (uint8_t*)buff;
  as->fp = fp;
  
  as->bytesRead = 0;                    // Clear read byte counter

  res = validate(&fp->obj, &fs);        // Check validity of the file object
  if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK)
    LEAVE_FF(fs, res);
  
  if (!(fp->flag & FA_READ))            // Check access mode
    LEAVE_FF(fs, FR_DENIED);
  
  if(fp->fptr == 0)                     // Start of file, 
    as->bytesCached = 0;                // there're no cached data
  
  uint32_t remain = fp->obj.objsize - fp->fptr;  // Remaining data count
  as->remBytes = remain;
  if (as->bytesToread > remain)
    as->bytesToread = remain;           // Truncate btr by remaining bytes

  if(as->bytesCached)                   // Write cached data if any
  {
    uint32_t ncpy = as->bytesToread;    // Count...
    if(ncpy > as->bytesCached)
      ncpy = as->bytesCached;
    
    uint8_t* pCache = fp->buf + SS(fs) - as->bytesCached;       // Find...
    memcpy((void*)as->buf, pCache, ncpy);                       // Write...
    as->bytesCached -= ncpy;
    as->bytesRead += ncpy;
    fp->fptr += ncpy;
    as->buf += ncpy;
    
    if(as->bytesToread == as->bytesRead)            // That's all?
    {
      as->result = FR_OK;
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
          as->clust = clmt_clust(fp, fp->fptr);
        }
        else
#endif
        {
          clst = get_fat(&fp->obj, fp->clust);	// Follow cluster chain on the FAT
        }

      }
      
      if (clst < 2)
      {
        as->result = FR_INT_ERR;
        ABORT(fs, FR_INT_ERR);
      }
      if (clst == 0xFFFFFFFF)
      {
        as->result = FR_DISK_ERR;
        ABORT(fs, FR_DISK_ERR);
      }
      fp->clust = clst;	                // Update current cluster
      as->clust = clst;
      as->clustPrev = clst;
    }
    sect = clust2sect(fs, fp->clust);	// Get current sector
    as->sect = sect;
    if (!sect)
    {
      as->result = FR_INT_ERR;
      ABORT(fs, FR_INT_ERR);
    }
    sect += csect;
    
    cc = as->remBytes / SS(fs);	 // When remaining bytes >= sector size,
    if (cc && (as->bytesToread - as->bytesRead) >= 512)   // Read maximum contiguous sectors directly
    {							
      if (adisk_read(fs->drv, (BYTE*)as->buf, sect, cc) != RES_OK)
      {
        as->result = FR_DISK_ERR;
        ABORT(fs, FR_DISK_ERR);
      }
    }
    else                        // Remaining bytes < sector size,
    {
      as->bytesCached = 512;    // Read data in cache
      if (adisk_read(fs->drv, fp->buf, sect, 1) != RES_OK)
      {
        as->result = FR_DISK_ERR;
        ABORT(fs, FR_DISK_ERR);
      }
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
  // Stop DMA
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
  
  as->bytesRead += 512;
  as->fp->fptr += 512;
  if(as->bytesRead >= as->bytesToread)          // Enough..
  {
    Dma_Stop_Rd();
    return;
  }

  bool cont = 1;                                // Contiguos clusters
  bool partial = (as->bytesRead + 512 > as->bytesToread); // partial next sector read?

  if (as->fp->fptr % SS(fs) == 0)               // On the sector boundary?
  {
    // Sector offset in the cluster
    as->sect = (UINT)(as->fp->fptr / SS(fs) & (as->fp->obj.fs->csize - 1));
    if (as->sect == 0)                          // On the cluster boundary?
    {					        // Middle or end of the file
#if _USE_FASTSEEK
      if (!as->fp->cltbl)         // Error - not valid cluster table
      {
        as->result = FR_INT_ERR;
        as->fp->err = FR_INT_ERR;
        return;
        // ABORT(fs, FR_INT_ERR);
      }
      as->clust = clmt_clust(as->fp, as->fp->fptr);
#else
#error Async read working only with _USE_FASTSEEK option enabled - edit "ffconf.h"
#endif

      if (as->clust < 2)
      {
        as->result = FR_INT_ERR;
        as->fp->err = FR_INT_ERR;
        return;
        // ABORT(fs, FR_INT_ERR);
      }
      if (as->clust == 0xFFFFFFFF)
      {
        as->result = FR_DISK_ERR;
        as->fp->err = FR_DISK_ERR;
        return;
        // ABORT(fs, FR_DISK_ERR);
      }
      as->fp->clust = as->clust;	        // Update current cluster
      cont = (as->clust == as->clustPrev + 1);  // Contiguos clusters?
      as->clustPrev = as->clust;                // Remember cluster
    }
    as->sect = clust2sect(as->fp->obj.fs, as->fp->clust);       // Get current sector
    
    if (!as->sect)
    {
      //ABORT(as->fp->obj.fs, FR_INT_ERR);
      as->result = FR_INT_ERR;
      as->fp->err = FR_INT_ERR;
      return;
    }

    if(!cont)                     // Not contigous next cluster (fragmented file)
    {
      send_cmd(CMD12, 0);         // STOP_TRANSMISSION
      release_spi();
      send_cmd(CMD18, as->sect);  // Start read from next fragment
    }
  }
  
  if((as->bytesRead < as->bytesToread) && !partial)
    Dma_Cont_Rd();
  else if(partial)
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
     as->buf += 512;
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
    as->bytesRead -= 512;
    
    uint32_t ncpy = as->bytesToread - as->bytesRead;
    memcpy((void*)as->buf, as->fp->buf, ncpy);
    as->fp->fptr += ncpy;
    as->bytesCached -= ncpy;
    as->bytesRead += ncpy;
    as->buf += ncpy;
  }
  as->result = FR_OK;
}

//------------------------------------------------------------------------------
void* FastSeek(FIL* fp)
{
  // File max cluster count
  uint32_t nClust = fp->obj.objsize / (fp->obj.fs->csize * 512) + 1;
  
  // required 2 dwords per cluster + 2
  uint32_t size32 = (nClust << 1) + 2;
  
  // Try alloc buffer
  uint32_t* clmt = (uint32_t*)malloc(size32 << 2);
  
  if(clmt)              // Successful buffer allocation
  {
    fp->cltbl = (DWORD*)clmt;
    clmt[0] = size32;                                 // Set table size
    FRESULT res = f_lseek(fp, CREATE_LINKMAP);        // Create CLMT
    if(res != FR_OK)    // Failed..
    {
      fp->cltbl = NULL;
      free(clmt);
      return NULL;
    }
  }
  else                  // Not enough memory..         
    fp->cltbl = NULL;
  
  return clmt;          // OK. Dont forget to free() when closing file 
}








