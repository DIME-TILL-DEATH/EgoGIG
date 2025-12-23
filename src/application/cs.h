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

private:
	void Code();
	TSemaphore *sem;

};

extern TCSTask *CSTask;

#endif /*__CS_H__*/
