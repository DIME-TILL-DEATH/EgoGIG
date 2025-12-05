/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H


#define rt_counter_free_tim_rcc    RCC_TIM2
#define rt_counter_free_tim        TIM2


#define rt_counter_irq_tim_rcc     RCC_TIM7
#define rt_counter_irq_tim         TIM7
#define rt_counter_irq_tim_handler TIM7_IRQHandler
#define rt_counter_irq_tim_irq     NVIC_TIM7_IRQ

#define RT_COUNTER_TIM_DEBUG_FREEZE() \
		    { __HAL_FREEZE_TIM2_DBGMCU(); \
		      __HAL_FREEZE_TIM7_DBGMCU(); \
		    }

#define LED_COUNT                       1
#define LED_DESC                       { RCC_GPIOC , GPIOC, GPIO_OTYPE_PP, GPIO1 , 500 , 500 , true}

#define      DEBUG_OUT_PORT  GPIOC
#define      DEBUG_OUT_RCC   RCC_GPIOC
#define      DEBUG_OUT_PIN   GPIO14

#define      SD_DETECT_PORT  GPIOA
#define      SD_DETECT_RCC   RCC_GPIOA
#define      SD_DETECT_PIN   GPIO8

#define      SD_CMD_PORT     GPIOD
#define      SD_CMD_RCC      RCC_GPIOD
#define      SD_CMD_PIN      GPIO2


#define      SD_DATA_PORT    GPIOC
#define      SD_DATA_RCC     RCC_GPIOC
#define      SD_D0_PIN       GPIO8
#define      SD_D1_PIN       GPIO9
#define      SD_D2_PIN       GPIO10
#define      SD_D3_PIN       GPIO11
#define      SD_CLK_PIN      GPIO12
#define      SD_DATA_PINS    SD_D0_PIN | SD_D1_PIN | SD_D2_PIN | SD_D3_PIN


/*
SD_DETECT    PA8

SD_CMD       PD2


SD_D0        PC8
SD_D1        PC9
SD_D2        PC10
SD_D3        PC11
SD_CLK       PC12
*/

#endif /* __PLATFORM_CONFIG_H */

