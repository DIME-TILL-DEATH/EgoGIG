
#ifndef __SD_H__
#define __SD_H__

#include "stm32f4xx_hal.h"

   
/** 
  * @brief  SD status structure definition  
  */     
#define MSD_OK         0x00
#define MSD_ERROR      0x01


/** @defgroup STM324xG_EVAL_SD_Exported_Constants
  * @{
  */ 
//#define SD_DETECT_PIN                    GPIO_PIN_13
//#define SD_DETECT_GPIO_PORT              GPIOH
//#define __SD_DETECT_GPIO_CLK_ENABLE()    __GPIOH_CLK_ENABLE()
//#define SD_DETECT_IRQn                   EXTI15_10_IRQn

#define SD_DATATIMEOUT           ((uint32_t)100000000)


//#define SD_NOT_PRESENT           ((uint8_t)0x00)
   
/* DMA definitions for SD DMA transfer */
//#define __DMAx_TxRx_CLK_ENABLE            __DMA2_CLK_ENABLE
//#define SD_DMAx_Tx_CHANNEL                DMA_CHANNEL_4
//#define SD_DMAx_Rx_CHANNEL                DMA_CHANNEL_4
//#define SD_DMAx_Tx_STREAM                 DMA2_Stream6
//#define SD_DMAx_Rx_STREAM                 DMA2_Stream3
//#define SD_DMAx_Tx_IRQn                   DMA2_Stream6_IRQn
//#define SD_DMAx_Rx_IRQn                   DMA2_Stream3_IRQn
//#define SD_DMAx_Tx_IRQHandler             DMA2_Stream6_IRQHandler
//#define SD_DMAx_Rx_IRQHandler             DMA2_Stream3_IRQHandler
//#define SD_DetectIRQHandler()             HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13)


uint8_t sd_init(void);
void    sd_deinit(void);
uint8_t sd_read_blocks(uint32_t *pData, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
uint8_t sd_write_blocks(uint32_t *pData, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
uint8_t sd_read_blocks_dma(uint32_t *pData, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
uint8_t sd_write_blocks_dma(uint32_t *pData, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumOfBlocks);
uint8_t sd_erase(uint64_t StartAddr, uint64_t EndAddr);

HAL_SD_TransferStateTypedef 
        sd_get_status(void);
void    sd_get_card_info(HAL_SD_CardInfoTypedef *CardInfo);
uint8_t sd_is_detected(void);  


#endif /* __SD_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
