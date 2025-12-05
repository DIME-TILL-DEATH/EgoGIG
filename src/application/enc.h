#ifndef __ENC_H__
#define __ENC_H__

#include "appdefs.h"
#include "libopencm3/stm32/timer.h"

class TENCTask: public TTask
{
public:
	inline TENCTask(const char *name, const int stack_size, const int priority) :
			TTask(name, stack_size, priority, false)
	{

	}

private:
	void Code();
};

extern TENCTask *ENCTask;

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


#endif /*__ENC_H__*/
