#include "gui.h"

#include "fs_stream.h"

#include "abstractmenu.h"

#include "cs.h"
#include "init.h"
#include "display.h"
#include "libopencm3/stm32/gpio.h"
#include "midi.h"
#include "appdefs.h"
#include "init.h"
#include "display.h"
#include "fs_stream.h"
#include "enc.h"

#include "libopencm3/stm32/timer.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/dma.h"
#include <libopencm3/cm3/nvic.h>
#include "libopencm3/stm32/spi.h"
#include "libopencm3/stm32/usart.h"

uint8_t num_prog = 0;

//const uint8_t led_sym[10] =
//{ 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90 };

uint8_t tim5_fl = 0;
uint8_t blink_en = 0;
uint8_t num_tr_fl = 0;

void processGui(TTask* processingTask)
{
	if(currentMenu)
	{
		currentMenu->task();

		switch(key_ind)
		{
		case key_stop: currentMenu->keyStop(); break;
		case key_stop_long: currentMenu->keyStopLong(); break;
		case key_start: currentMenu->keyStart(); break;
		case key_left_up: currentMenu->keyLeftUp(); break;
		case key_left_down: currentMenu->keyLeftDown(); break;
		case key_right_up: currentMenu->keyRightUp(); break;
		case key_right_down: currentMenu->keyRightDown(); break;
		case key_return: currentMenu->keyReturn(); break;
		case key_return_long: currentMenu->keyReturnLong(); break;
		case key_forward: currentMenu->keyForward(); break;
		case key_forward_long: currentMenu->keyForwardLong(); break;
		case key_esc: currentMenu->keyEsc(); break;
		case key_encoder: currentMenu->encoderPress(); break;
		case key_encoder_long: currentMenu->encoderLongPress(); break;
		}

		if(encoder_state1)
		{
			if(encoder_state == 2)
			{
				currentMenu->encoderClockwise();
			}
			else
			{
				currentMenu->encoderCounterClockwise();
			}
		}

		encoder_state1 = encoder_key = key_ind = 0;
	}
	return;
}
