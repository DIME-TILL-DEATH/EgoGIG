#include "cs.h"
#include "midi.h"
#include "appdefs.h"
#include "init.h"
#include "display.h"
#include "fs_stream.h"

#include "gui.h"
#include "init.h"

#include "menuplayer.h"
#include "menusystem.h"
#include "menumidicontrol.h"

#include "libopencm3/stm32/timer.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/dma.h"
#include <libopencm3/cm3/nvic.h>
#include "libopencm3/stm32/spi.h"
#include "libopencm3/stm32/usart.h"

TCSTask *CSTask;
TSofwareTimer *DisplayBlinkTask;


/////////////////////////////////////////////Gui//////////////////////////////////////////////////////////////
void TCSTask::Code()
{
	Delay(500);
	DisplayTask->Clear();
	DisplayTask->StringOut(2, 0, (uint8_t*) "AMT EGO_GIG");
	DisplayTask->StringOut(1, 1, (uint8_t*) "SD Multiplayer");

	Delay(1500);
	DisplayTask->Clear();
	DisplayTask->StringOut(3, 0, (uint8_t*) FIRMWARE_VER);

	Delay(500);
	if (GPIOB_IDR &GPIO8)
	{
		DisplayTask->Clear();
		DisplayTask->StringOut(2, 0, (uint8_t*) "SD not ready");
		DisplayTask->StringOut(2, 1, (uint8_t*) "insert card");

		Suspend();
	}

	extern volatile uint8_t FsStream_enable_fl;
	FsStream_enable_fl = 1;
	FsStreamTask->Resume();
	while (FsStream_enable_fl);

	DisplayTask->Clear();
	key_reg_out[0] |= 0x90;

	load_led(num_prog);

	MenuSystem::read_sys();
	MenuMidiControl::read_ctrl();
	read_map();

	blink_en = 1;

	menuPlayer = new MenuPlayer();
	currentMenu = menuPlayer;
	currentMenu->show();

	while (1)
	{
		sem->Take(portMAX_DELAY);
		processGui(this);
		menuPlayer->processPlayNext();
	}
}

