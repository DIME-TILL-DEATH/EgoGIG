#ifndef __CS_H__
#define __CS_H__

#include "appdefs.h"
#include "libopencm3/stm32/timer.h"

//#include "gui.h"



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

	enum notify_t
	{
		qn_stop_song = 0,
		qn_load_song,
		qn_play_next_song,
		qn_list_next_song,
		qn_list_prev_song,
		qn_jump_to_point1,
		qn_jump_to_point2
	};

	struct query_notify_t
	{
		notify_t notify :8;
		union
		{
			uint8_t songNum;
			uint8_t data[3];
		} __attribute__((packed));
	};

	void notify(notify_t notifyType, uint8_t value = 0)
	{
		query_notify_t qn ={
			.notify = notifyType,
			.songNum = value
		};
		notify(qn);
	}


private:
	void Code();
	TSemaphore *sem;

	void notify(const query_notify_t &val)
	{
		if (cortex_isr_num())
		{
			// send comand from ISR
			BaseType_t xHigherPriorityTaskWoken;
			NotifyFromISR(*((uint32_t*) &val), eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
		else
		{
			// thread mode
			Notify(*((uint32_t*) &val), eSetValueWithOverwrite);
		}
	}
};

extern TCSTask *CSTask;

#endif /*__CS_H__*/
