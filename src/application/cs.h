#ifndef __CS_H__

#include "appdefs.h"
#include "libopencm3/stm32/timer.h"

enum
{
	player,
	edit_playlist,
	select_folder,
	play_next_fil,
	name_ind_play,
	delete_file,
	menu,
	sys_menu,
	metronome,
	midi_ctrl_menu,
	midi_pc_menu
};
enum
{
	key_stop = 1,
	key_start,
	key_left_up,
	key_left_down,
	key_right_up,
	key_right_down,
	key_return,
	key_forward,
	key_esc,
	key_encoder
};
enum
{
	auto_next_track = 0,
	direction_counter,
	direction_scrol_playlist,
	loop_points
};
enum
{
	ctrl1_t, ctrl1, ctrl2_t, ctrl2, ctrl3_t, ctrl3, jjj, chann
};

class TCSTask: public TTask
{
public:
	inline TCSTask(const char *name, const int stack_size, const int priority) :
			TTask(name, stack_size, priority, false)
	{
		sem = new TSemaphore(TSemaphore::fstCounting, 4, 0);
	}

	inline void Give()
	{
		if (cortex_isr_num())
		{
			BaseType_t HigherPriorityTaskWoken;
			sem->GiveFromISR(&HigherPriorityTaskWoken);
			if (HigherPriorityTaskWoken)
				TScheduler::Yeld();
		}
		else
			sem->Give();
	}

private:
	void Code();
	TSemaphore *sem;

};

extern uint8_t tim5_fl;

extern TCSTask *CSTask;

inline uint32_t enc_speed_inc(uint32_t data, uint32_t max)
{
	TIM6_CR1 &= ~TIM_CR1_CEN;
	if (timer_get_flag(TIM6, TIM_SR_UIF))
		data += 1;
	else
	{
		if (timer_get_counter(TIM6) > 0x3fff)
			data += 1;
		else
		{
			if (timer_get_counter(TIM6) > 0x1fff)
			{
				if (data < (max - 4))
					data += 5;
				else
					data += 1;
			}
			else
			{
				if (timer_get_counter(TIM6) > 0xfff)
				{
					if (data < (max - 9))
						data += 10;
					else
						data += 1;
				}
				else
				{
					if (timer_get_counter(TIM6) > 0x7ff)
					{
						if (data < (max - 19))
							data += 20;
						else
							data += 1;
					}
					else
					{
						if (data < (max - 39))
							data += 40;
						else
							data += 1;
					}
				}
			}
		}
	}
	timer_set_counter(TIM6, 0);
	timer_clear_flag(TIM6, TIM_SR_UIF);
	TIM6_CR1 |= TIM_CR1_CEN;
	return data;
}
inline uint32_t enc_speed_dec(uint32_t data, uint32_t min)
{
	TIM6_CR1 &= ~TIM_CR1_CEN;
	if (timer_get_flag(TIM6, TIM_SR_UIF))
		data -= 1;
	else
	{
		if (timer_get_counter(TIM6) > 0x3fff)
			data -= 1;
		else
		{
			if (timer_get_counter(TIM6) > 0x1fff)
			{
				if (data > (min + 4))
					data -= 5;
				else
					data -= 1;
			}
			else
			{
				if (timer_get_counter(TIM6) > 0xfff)
				{
					if (data > (min + 9))
						data -= 10;
					else
						data -= 1;
				}
				else
				{
					if (timer_get_counter(TIM6) > 0x7ff)
					{
						if (data > (min + 19))
							data -= 20;
						else
							data -= 1;
					}
					else
					{
						if (data > (min + 39))
							data -= 40;
						else
							data -= 1;
					}
				}
			}
		}
	}
	timer_set_counter(TIM6, 0);
	timer_clear_flag(TIM6, TIM_SR_UIF);
	TIM6_CR1 |= TIM_CR1_CEN;
	return data;
}

extern uint8_t condish;
extern uint8_t sys_param[];
extern uint32_t song_size;
extern uint8_t blink_en;

#endif /*__CS_H__*/
