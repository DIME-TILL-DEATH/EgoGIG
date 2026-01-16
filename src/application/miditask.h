#ifndef __MIDI_H__
#define MIDI_H

#include "appdefs.h"

#include "libopencm3/stm32/dma.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/usart.h"

class TMIDITask: public TTask
{
public:
	inline TMIDITask(const char *name, const int stack_size, const int priority) :
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

extern TMIDITask *MIDITask;

#endif /*__MIDI_H__*/
