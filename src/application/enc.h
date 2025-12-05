#ifndef __ENC_H__

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

#endif /*__ENC_H__*/
