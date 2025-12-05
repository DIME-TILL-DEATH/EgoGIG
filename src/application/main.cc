#include "appdefs.h"
#include "mmgr.h"

#include "cs.h"
#include "enc.h"
#include "fs_stream.h"
#include "usb.h"
#include "console.h"
#include "init.h"
#include "display.h"
#include "midi.h"

#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"

int main(void)
{
	rcc_periph_clock_enable (RCC_GPIOA);
	if (GPIOA_IDR &GPIO9)
	{
		gpio_set(GPIOA, GPIO6);
		gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6); // power display
		gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO6);
		UsbTask = new TUsbTask("USB", 20 * configMINIMAL_STACK_SIZE, 0);
		dela(0x3fffff);
		gpio_clear(GPIOA, GPIO6);
		dela(0x3fffff);
		disp_init();
		lcd44780_ClearLCD();
		lcd44780_ShowStr(0, 0, (uint8_t*) "  Connected to     a computer");
	}
	else
	{
		init();
		disp_init();
		lcd44780_ClearLCD();
		CSTask = new TCSTask("CS", 20 * configMINIMAL_STACK_SIZE, 0);
		ENCTask = new TENCTask("ENC", 2 * configMINIMAL_STACK_SIZE, 0);
		DisplayTask = new TDisplayTask("DISPLAY", 20 * configMINIMAL_STACK_SIZE, 0);
		MIDITask = new TMIDITask("MIDI", 20 * configMINIMAL_STACK_SIZE, 0);
		FsStreamTask = new TFsStreamTask("FSS", 64 * configMINIMAL_STACK_SIZE, 1);

	}

	TScheduler::StartScheduler();
}

application_storage_t application_storage;
