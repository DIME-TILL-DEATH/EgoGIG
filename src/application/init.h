#ifndef __INIT_H__
#define __INIT_H__

#include "libopencm3/stm32/exti.h"
#include "libopencm3/stm32/timer.h"

#include "menuplayer.h"
#include "player.h"

enum
{
	auto_next_track = 0,
	direction_counter,
	direction_scrol_playlist,
	loop_points
};

enum
{
	ctrl1_t = 0, ctrl1,
	ctrl2_t, ctrl2,
	ctrl3_t, ctrl3,
	jjj, chann
};

extern MenuPlayer* menuPlayer;
extern Player player;

extern uint16_t key_reg_out[];
extern uint8_t num_prog;
extern uint8_t m_num_prog_edit;
extern const uint8_t led_sym[];
extern volatile uint8_t tim3_end_fl;
extern uint8_t sys_param[];
extern uint8_t ctrl_param[];
extern uint8_t pc_param[];
extern uint8_t cs_resum_fl;
extern volatile uint32_t file_end_fl;
extern volatile uint8_t lock_fl;
extern volatile uint8_t key_ind;
extern volatile uint32_t metronom_int;
extern uint16_t metronom_counter;
extern uint8_t metronom_start;
extern uint16_t temp_counter;
extern uint32_t tap_temp;
extern uint32_t tap_temp1;
extern uint32_t tap_temp2;

extern volatile uint32_t play_point1;
extern volatile uint32_t play_point2;

extern volatile uint32_t count_down;
extern volatile uint32_t count_up;

extern wav_sample_t sound_buff[];
extern wav_sample_t click_buff[];

extern uint16_t key_reg_in[2];
extern uint16_t key_reg_out[2];
extern uint8_t key_val;

extern volatile uint8_t encoder_state, encoder_state1, encoder_key, key_ind;

extern uint16_t msec_tik;
extern size_t sound_point;
extern volatile uint32_t samp_point;

extern uint8_t us_buf1;
extern volatile uint32_t click_size;
extern uint8_t sys_param[];
extern uint32_t song_size;
extern uint8_t blink_en;
extern volatile uint8_t led_blink_fl;

extern uint8_t tim5_fl;

void init(void);
void i2s_dma_interrupt_enable();
void i2s_dma_interrupt_disable();
void tim_start(uint16_t del);

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

inline void load_led(uint8_t num)
{
	uint8_t temp = num + 1;
	key_reg_out[1] = (led_sym[temp / 10]) | ((led_sym[temp % 10]) << 8);
}

inline void __attribute__ ((always_inline)) dela(uint32_t p)
{
	for (uint32_t i = 0; i < p; i++)
		NOP();
}
#endif /*__INIT_H__*/
