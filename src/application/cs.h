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
		qn_list_prev_song
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

	inline void stop_song_notify()
	{
		query_notify_t qn = {
			.notify = qn_stop_song
		};
		notify(qn);
	}

	inline void load_song_notify(uint8_t reqSongNum)
	{
		query_notify_t qn ={
			.notify = qn_load_song,
			.songNum = reqSongNum
		};
		notify(qn);
	}

	inline void play_next_song_notify()
	{
		query_notify_t qn = {
			.notify = qn_play_next_song
		};
		notify(qn);
	}

	inline void list_next_song_notify()
	{
		query_notify_t qn = {
			.notify = qn_list_next_song
		};
		notify(qn);
	}

	inline void	list_prev_song_notify()
	{
		query_notify_t qn = {
			.notify = qn_list_prev_song
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
