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

#include "gui.h"

TENCTask *ENCTask;
extern uint8_t key_val;
extern uint8_t blink_en;
volatile uint8_t key_push_fl = 0;
volatile uint8_t stp_push_fl = 0;
volatile uint8_t stp_dub_fl = 0;
volatile uint8_t ret_push_fl = 0;
volatile uint8_t fwd_push_fl = 0;
volatile uint8_t ret_dub_fl = 0;
volatile uint8_t fwd_dub_fl = 0;
volatile uint8_t esc_push_fl = 0;
volatile uint8_t esc_dub_fl = 0;
volatile uint8_t enc_push_fl = 0;
volatile uint8_t enc_dub_fl = 0;
volatile uint8_t tim3_end_fl;
volatile uint8_t led_blink_fl;
volatile uint8_t lock_fl;
volatile uint8_t lock_fl_old;
uint32_t led_blink_count1 = 0;
uint32_t led_blink_count2 = 0;
uint32_t led_blink_count3 = 0;
uint8_t cs_resum_fl = 0;

void tim_start(uint16_t del)
{
	timer_clear_flag(TIM3, TIM_SR_UIF);
	timer_set_counter(TIM3, del);
	TIM3_CR1 |= TIM_CR1_CEN;
}

void TENCTask::Code()
{
	TIM3_CR1 |= TIM_CR1_CEN;
	while (1)
	{
		if ((key_val != 255) && !key_push_fl && !stp_push_fl && !ret_push_fl
				&& !fwd_push_fl && !esc_push_fl && !enc_push_fl)
		{
			tim_start(0xf7f0);
			switch (key_val)
			{
			case 254:
				stp_push_fl = 1;
				if (!tim3_end_fl)
				{
					tim_start(55000);
					timer_enable_irq(TIM3, TIM_SR_UIF);
				}
				break;   // stop
			case 247:
				if (!lock_fl || (stop_fl1))
				{
					key_ind = key_start;
					key_push_fl = 1;
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
					key_push_fl = 1;
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
					key_push_fl = 1;
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
					key_push_fl = 1;
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
					key_push_fl = 1;
					CSTask->Give();
				}
				break; // right down
			case 251:
				ret_push_fl = 1;                      // return
				if (!tim3_end_fl)
				{
					tim_start(55000);
					timer_enable_irq(TIM3, TIM_SR_UIF);
				}
				break;
			case 253:
				fwd_push_fl = 1;                      // forward
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
					key_push_fl = 1;
					CSTask->Give();
				}
				break;
			case 2:
				enc_push_fl = 1;                      // encoder key
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
					key_push_fl = 1;
					CSTask->Give();
				}
				break;
			case 4:
				if ((!lock_fl || (stop_fl1)) && (GPIOA_IDR & GPIO10))
				{
					key_ind = key_start;
					key_push_fl = 1;
					CSTask->Give();
				}
				break;
			}
		}
		if (key_val == 255)
		{
			if (ret_push_fl)
			{
				timer_disable_irq(TIM3, TIM_SR_UIF);
				if (!tim3_end_fl)
				{
					if (!lock_fl)
					{
						key_ind = key_return;      // return
						CSTask->Give();
					}
				}
				tim_start(0xf700);
				ret_push_fl = 0;
			}
			if (fwd_push_fl)
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
				fwd_push_fl = 0;
			}
			if (enc_push_fl)
			{
				timer_disable_irq(TIM3, TIM_SR_UIF);
				if (!tim3_end_fl)
				{
					key_ind = key_encoder;       // enc key
					CSTask->Give();
				}
				tim_start(0xf700);
				enc_push_fl = 0;
			}
			if (stp_push_fl)
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
				stp_push_fl = 0;
			}
		}
		if (timer_get_flag(TIM3, TIM_SR_UIF))
		{
			if (key_val == 255)
				key_push_fl = stp_push_fl = ret_push_fl = fwd_push_fl =
						esc_push_fl = enc_push_fl = tim3_end_fl = 0;
			else
				tim_start(0xff00);
		}
//----------------------------------------------LED Blink--------------------------------------------------------------
		if (led_blink_fl)
		{
			if (led_blink_count1 < 7)
			{
				if (!led_blink_count2)
					key_reg_out[1] &= ~0x8080;
				if (led_blink_count2 == 25000)
					key_reg_out[1] |= 0x8080;
				if (led_blink_count2++ > 50000)
				{
					led_blink_count2 = 0;
					led_blink_count1++;
				}
			}
			else
			{
				led_blink_count1 = led_blink_fl = 0;
			}
		}
//---------------------------------------------Play blink--------------------------------------------------------------
		if (led_blink_count3++ > 300000)
		{
			led_blink_count3 = 0;
			if (tim5_fl)
				tim5_fl = 0;
			else
				tim5_fl = 1;
			if (blink_en)
				CSTask->Give();
		}
//---------------------------------------------Lock check-----------------------------------------------------------------
		if (GPIOB_IDR &GPIO9)
		{
			lock_fl = 1;
			if (lock_fl_old != lock_fl)
			{
				lock_fl_old = lock_fl;
				key_reg_out[0] |= 4;
			}
		}
		else
		{
			lock_fl = 0;
			if (lock_fl_old != lock_fl)
			{
				lock_fl_old = lock_fl;
				key_reg_out[0] &= ~4;
			}
		}
	}
}
extern "C" void TIM3_IRQHandler()
{
	timer_clear_flag(TIM3, TIM_SR_UIF);
	timer_disable_irq(TIM3, TIM_SR_UIF);
	tim3_end_fl = 1;
	if (ret_push_fl || fwd_push_fl || enc_push_fl || stp_push_fl)
	{
		if (ret_push_fl)
		{
			if (!lock_fl)
			{
				key_ind = key_return;
				ret_dub_fl = 1;
				CSTask->Give();
			}
		}
		if (fwd_push_fl)
		{
			if (!lock_fl)
			{
				key_ind = key_forward;
				fwd_dub_fl = 1;
				CSTask->Give();
			}
		}
		if (enc_push_fl)
		{
			key_ind = key_encoder;
			enc_dub_fl = 1;
			CSTask->Give();
		}
		if (stp_push_fl)
		{
			key_ind = key_stop;
			stp_dub_fl = 1;
			CSTask->Give();
		}
	}
}

