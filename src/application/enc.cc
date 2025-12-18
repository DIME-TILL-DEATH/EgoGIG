#include "appdefs.h"
#include "enc.h"

#include "cs.h"
#include "init.h"
#include "display.h"
#include "fs_stream.h"
#include "libopencm3/stm32/timer.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/exti.h"
#include <libopencm3/cm3/nvic.h>

#include "leds.h"

TENCTask *ENCTask;

#define KEY_PUSHED 0x01
#define STP_PUSHED 0x02
#define RET_PUSHED 0x04
#define FWD_PUSHED 0x08
#define ESC_PUSHED 0x10
#define ENC_PUSHED 0x20

volatile uint8_t __CCM_BSS__ pushedButtons = 0;

volatile uint8_t __CCM_BSS__ encoder_state, encoder_rotated, encoder_key, key_ind;
volatile uint8_t __CCM_BSS__ key_val;
volatile uint8_t __CCM_BSS__ tim3_end_fl;
volatile uint8_t __CCM_BSS__ lock_fl;
volatile uint8_t __CCM_BSS__ lock_fl_old;

uint32_t __CCM_BSS__ led_blink_count = 0;

void tim_start(uint16_t del)
{
	timer_clear_flag(TIM3, TIM_SR_UIF);
	timer_set_counter(TIM3, del);
	TIM3_CR1 |= TIM_CR1_CEN;
}

inline uint8_t drebezg(uint32_t line)
{
	uint8_t sss;
	TIM9_CR1 &= ~TIM_CR1_CEN;
	if ((EXTI_FTSR & line) != 0)
	{
		if (TIM9_SR &TIM_SR_UIF)
		{
			sss = 1;
			EXTI_FTSR &= ~line;
			EXTI_RTSR |= line;
		}
		else
			sss = 0;
	}
	else
	{
		if (TIM9_SR &TIM_SR_UIF)
		{
			EXTI_RTSR &= ~line;
			EXTI_FTSR |= line;
			sss = 2;
		}
		else
			sss = 0;
	}
	TIM9_CNT = 0;
	TIM9_SR &= ~TIM_SR_UIF;
	TIM9_CR1 |= TIM_CR1_CEN;
	return sss;
}

void TENCTask::Code()
{
	TIM3_CR1 |= TIM_CR1_CEN;
	while (1)
	{
		if(key_val != 255 && !pushedButtons)
		{
			tim_start(0xf7f0);
			switch (key_val)
			{
			case 254:
				pushedButtons |= STP_PUSHED;
				if (!tim3_end_fl)
				{
					tim_start(55000);
					timer_enable_irq(TIM3, TIM_SR_UIF);
				}
				break;   // stop
			case 247:
				if (!lock_fl) // || (stop_fl1))
				{
					key_ind = key_start;

					pushedButtons |= KEY_PUSHED;
					CSTask->Give();
				}
				break;   // start

			case 239:
				if (!lock_fl)
				{
					if (!sys_param[direction_scrol_playlist])
						key_ind = key_left_up;
					else
						key_ind = key_left_down;

					pushedButtons |= KEY_PUSHED;
					CSTask->Give();
				}
				break;  // left up
			case 223:
				if (!lock_fl)
				{
					if (!sys_param[direction_scrol_playlist])
						key_ind = key_left_down;
					else
						key_ind = key_left_up;

					pushedButtons |= KEY_PUSHED;
					CSTask->Give();
				}
				break;  // left down
			case 127:
				if (!lock_fl)
				{
					if (!sys_param[direction_scrol_playlist])
						key_ind = key_right_up;
					else
						key_ind = key_right_down;

					pushedButtons |= KEY_PUSHED;
					CSTask->Give();
				}
				break;  // right up
			case 191:
				if (!lock_fl)
				{
					if (!sys_param[direction_scrol_playlist])
						key_ind = key_right_down;
					else
						key_ind = key_right_up;

					pushedButtons |= KEY_PUSHED;
					CSTask->Give();
				}
				break; // right down
			case 251:
				pushedButtons |= RET_PUSHED;
				if (!tim3_end_fl)
				{
					tim_start(55000);
					timer_enable_irq(TIM3, TIM_SR_UIF);
				}
				break;
			case 253:
				pushedButtons |= FWD_PUSHED;
				if (!tim3_end_fl)
				{
					tim_start(55000);
					timer_enable_irq(TIM3, TIM_SR_UIF);
				}
				break;
			case 1:
				if (!lock_fl)
				{
					key_ind = key_esc;
					pushedButtons |= KEY_PUSHED;
					CSTask->Give();
				}
				break;
			case 2:
				pushedButtons |= ENC_PUSHED;
				if (!tim3_end_fl)
				{
					tim_start(55000);
					timer_enable_irq(TIM3, TIM_SR_UIF);
				}
				break;
			case 3:
				if (!lock_fl && (GPIOA_IDR & GPIO10))
				{
					key_ind = key_stop;
					pushedButtons |= KEY_PUSHED;
					CSTask->Give();
				}
				break;
			case 4:
				if ((!lock_fl) && (GPIOA_IDR & GPIO10)) //((!lock_fl || (stop_fl1)) && (GPIOA_IDR & GPIO10))
				{
					key_ind = key_start;
					pushedButtons |= KEY_PUSHED;
					CSTask->Give();
				}
				break;
			}
		}

		if (key_val == 255)
		{
			if(pushedButtons & RET_PUSHED)// (ret_push_fl)
			{
				timer_disable_irq(TIM3, TIM_SR_UIF);
				if (!tim3_end_fl)
				{
					if (!lock_fl)
					{
						key_ind = key_return;
						CSTask->Give();
					}
				}
				tim_start(0xf700);
				pushedButtons = 0;
			}
			if(pushedButtons & FWD_PUSHED) //(fwd_push_fl)
			{
				timer_disable_irq(TIM3, TIM_SR_UIF);
				if (!tim3_end_fl)
				{
					if (!lock_fl)
					{
						key_ind = key_forward;       // forward
						CSTask->Give();
					}
				}
				tim_start(0xf700);
				pushedButtons = 0;
			}
			if(pushedButtons & ENC_PUSHED) //(enc_push_fl)
			{
				timer_disable_irq(TIM3, TIM_SR_UIF);
				if (!tim3_end_fl)
				{
					key_ind = key_encoder;       // enc key
					CSTask->Give();
				}
				tim_start(0xf700);
				pushedButtons = 0;
			}

			if(pushedButtons & STP_PUSHED) //(stp_push_fl)
			{
				timer_disable_irq(TIM3, TIM_SR_UIF);
				if (!tim3_end_fl)
				{
					if (!lock_fl)
					{
						key_ind = key_stop;       // stop
						CSTask->Give();
					}
				}
				tim_start(0xf700);
				pushedButtons = 0;
			}
		}

		if(timer_get_flag(TIM3, TIM_SR_UIF))
		{
			if (key_val == 255)
			{
				tim3_end_fl = 0;
				pushedButtons = 0;
			}
			else
				tim_start(0xff00);
		}
//----------------------------------------------LED Blink--------------------------------------------------------------
		Leds::processBlinking();
//---------------------------------------------Play blink--------------------------------------------------------------
		if(led_blink_count++ > 300000)
		{
			led_blink_count = 0;
			if(tim5_fl)
				tim5_fl = 0;
			else
				tim5_fl = 1;

			if(menuPlayer) // Only after start CS Task
				CSTask->Give();
		}
//---------------------------------------------Lock check-----------------------------------------------------------------
		if(GPIOB_IDR &GPIO9)
		{
			lock_fl = 1;
			if (lock_fl_old != lock_fl)
			{
				lock_fl_old = lock_fl;
				Leds::lockOn();
			}
		}
		else
		{
			lock_fl = 0;
			if (lock_fl_old != lock_fl)
			{
				lock_fl_old = lock_fl;
				Leds::lockOff();
			}
		}
	}
}

extern "C" void TIM3_IRQHandler()
{
	timer_clear_flag(TIM3, TIM_SR_UIF);
	timer_disable_irq(TIM3, TIM_SR_UIF);
	tim3_end_fl = 1;

	if(pushedButtons & (RET_PUSHED | FWD_PUSHED | ENC_PUSHED | STP_PUSHED)) //(ret_push_fl || fwd_push_fl || enc_push_fl || stp_push_fl)
	{
		if(pushedButtons & RET_PUSHED)//(ret_push_fl)
		{
			if(!lock_fl)
			{
				key_ind = key_return_long;
				CSTask->Give();
			}
		}

		if(pushedButtons & FWD_PUSHED)//(fwd_push_fl)
		{
			if(!lock_fl)
			{
				key_ind = key_forward_long;
				CSTask->Give();
			}
		}

		if(pushedButtons & ENC_PUSHED)//(enc_push_fl)
		{
			key_ind = key_encoder_long;
			CSTask->Give();
		}

		if(pushedButtons & STP_PUSHED)//(stp_push_fl)
		{
			key_ind = key_stop_long;
			CSTask->Give();
		}
	}
}

extern "C" void EXTI4_IRQHandler()
{
	nvic_clear_pending_irq (NVIC_EXTI4_IRQ);
	if (!lock_fl)
	{
		if (drebezg(EXTI4) == 1)
		{
			if (GPIOC_IDR &GPIO5)
				encoder_state = 1;
			else
				encoder_state = 2;

			encoder_rotated = 1;
			CSTask->Give();
		}
	}
	exti_reset_request (EXTI4);
}
