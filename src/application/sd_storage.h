/*
 * storge.h
 *
 *  Created on: Aug 30, 2014
 *      Author: s
 */

#ifndef STORGE_H_
#define STORGE_H_

#include "stdint.h"
#include "diskio.h"

#ifdef  __cplusplus
 extern "C" {
#endif
   void sdio_init() ;
   DSTATUS sd_storage_initialize();
   DSTATUS sd_storage_deinitialize();
   DSTATUS sd_storage_get_status();
   DRESULT sd_storage_read  (      uint8_t *buf, const uint64_t blk_index, const uint16_t blk_count);
   DRESULT sd_storage_write (const uint8_t *buf, const uint64_t blk_index, const uint16_t blk_count);
   DRESULT sd_storage_ioctl (const uint8_t command , void* params);

#ifdef  __cplusplus
 	 }
#endif

#endif /* STORGE_H_ */
