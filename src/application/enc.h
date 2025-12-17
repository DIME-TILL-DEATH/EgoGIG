#ifndef __ENC_H__
#define __ENC_H__

#include "appdefs.h"

enum
{
	key_stop = 1,
	key_stop_long,
	key_start,
	key_left_up,
	key_left_down,
	key_right_up,
	key_right_down,
	key_return,
	key_return_long,
	key_forward,
	key_forward_long,
	key_esc,
	key_encoder,
	key_encoder_long
};


class TENCTask: public TTask
{
public:
	inline TENCTask(const char *name, const int stack_size, const int priority)
			: TTask(name, stack_size, priority, false)
	{

	}

private:
	void Code();
};

extern TENCTask *ENCTask;

#endif /*__ENC_H__*/
