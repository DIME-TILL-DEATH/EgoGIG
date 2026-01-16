
//#include "appdefs.h"
#include "sd_storage.h"
#include "diskio.h"
#include "sd.h"

static HAL_SD_CardInfoTypedef ci ;
static DSTATUS status = STA_NOINIT ;

DSTATUS sd_storage_initialize()
	{

          if ( status &= STA_NOINIT )
            {
              if (sd_init() != MSD_OK)
                          {
                             status |= STA_NOINIT ;
                             return STA_NODISK ;
                          }
            }

          status &= ~STA_NOINIT ;

          sd_get_card_info((HAL_SD_CardInfoTypedef*)&ci);

          return status ;
	}

        DSTATUS sd_storage_deinitialize()
	{

           sd_deinit();
           status |= STA_NOINIT ;
           return status ;
	}

        DSTATUS sd_storage_get_status()
        {
          return status ;
        }


	DRESULT sd_storage_read  (      uint8_t *buf, const uint64_t blk_index, const uint16_t blk_count) 	{ sd_read_blocks_dma((uint32_t*)buf, blk_index*ci.CardBlockSize , ci.CardBlockSize, blk_count ); return RES_OK ; }
	DRESULT sd_storage_write (const uint8_t *buf, const uint64_t blk_index, const uint16_t blk_count)  	{ sd_write_blocks_dma((uint32_t*)buf, blk_index*ci.CardBlockSize , ci.CardBlockSize, blk_count ); return RES_OK ; }


	DRESULT sd_storage_ioctl (const uint8_t command , void* params)
	{
		switch (command)
		{
			case CTRL_SYNC:
				return RES_OK ;
				break ;
			case GET_SECTOR_SIZE:
				*((unsigned int*)params) = 0 ;
				return RES_PARERR ;
				break ;
			case GET_SECTOR_COUNT:
				*((DWORD*)params) =  ci.CardCapacity / ci.CardBlockSize ; ;
				break ;
			case GET_BLOCK_SIZE:
				*((DWORD*)params) =  ci.CardBlockSize ; ;
				break ;
			/*case CTRL_ERASE_SECTOR:
				return RES_ERROR ;*/
			default :
				return RES_PARERR ;
		}
		return RES_OK ;

	}


	void disk_set_io_fn(BYTE drv , storage_fn_t* storage_fn)
	{
	         storage_fn->storage_initialize      = sd_storage_initialize ;
	         storage_fn->storage_deinitialize    = sd_storage_deinitialize ;
	         storage_fn->storage_get_status      = sd_storage_get_status;
	         storage_fn->storage_read            = sd_storage_read;
	         storage_fn->storage_write           = sd_storage_write;
	         storage_fn->storage_ioctl           = sd_storage_ioctl;
	}
