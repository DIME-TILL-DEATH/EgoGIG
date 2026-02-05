#include "appdefs.h"
#include "cs.h"
#include "init.h"
#include "libopencm3/stm32/usart.h"

#include "enc.h"
#include "init.h"
#include "leds.h"

#include "menumidicontrol.h"
#include "miditask.h"
#include "display.h"

TMIDITask *MIDITask;

uint8_t us_buf1;
void TMIDITask::Code()
{
	while (1)
	{
		sem->Take(portMAX_DELAY);
		usart_send_blocking(USART1, us_buf1);
	}
}

uint8_t midi_buf = 0;
extern "C" void USART1_IRQHandler()
{
	USART1_SR &= ~USART_SR_RXNE;
	uint8_t us_buf = USART1_DR;
	switch (midi_buf)
	{
	case 0:
		if (us_buf == 0xfa)
		{
			player.startPlay();
			midi_buf = 0;
		}

		if (us_buf == 0xfb)
		{
			player.pause();
			midi_buf = 0;
		}

		if (us_buf == 0xfc)
		{
			CSTask->stop_song_notify();
			midi_buf = 0;
		}

		if(us_buf == 0xf3)
			midi_buf = 0xf3;

		if (us_buf == (0xb0 | ctrl_param[MCHANNEL]))
			midi_buf = 0xb0;
		if (us_buf == (0x90 | ctrl_param[MCHANNEL]))
			midi_buf = 0x90;
		if (us_buf == (0xc0 | ctrl_param[MCHANNEL]))
			midi_buf = 0xc0;
		break;

	case 0xb0: //CC
		if (ctrl_param[MCTRL_START_TYPE] == MenuMidiControl::MIDI_IN_CC  && ctrl_param[MCTRL_START_VALUE] == us_buf)
			player.startPlay();
		if (ctrl_param[MCTRL_PAUSE_TYPE] == MenuMidiControl::MIDI_IN_CC && ctrl_param[MCTRL_PAUSE_VALUE] == us_buf)
			player.pause();
		if (ctrl_param[MCTRL_STOP_TYPE] == MenuMidiControl::MIDI_IN_CC && ctrl_param[MCTRL_STOP_VALUE] == us_buf)
			CSTask->stop_song_notify();
		if (ctrl_param[MCTRL_NEXT_SONG_TYPE] == MenuMidiControl::MIDI_IN_CC && ctrl_param[MCTRL_NEXT_SONG_VALUE] == us_buf)
			CSTask->list_next_song_notify();
		if (ctrl_param[MCTRL_PREV_SONG_TYPE] == MenuMidiControl::MIDI_IN_CC && ctrl_param[MCTRL_PREV_SONG_VALUE] == us_buf)
			CSTask->list_prev_song_notify();

		if(player.state() == Player::PLAYER_IDLE)
		{
			for (uint8_t i = 0; i < 99; i++)
			{
				if (pc_param[i * 2] == 1 && pc_param[i * 2 + 1] == us_buf)
				{
					CSTask->load_song_notify(us_buf);
					break;
				}
			}
		}
		midi_buf = 4;
		break;
	case 0x90: // Note
		if (ctrl_param[MCTRL_START_TYPE] == MenuMidiControl::MIDI_IN_NOTE && ctrl_param[MCTRL_START_VALUE] == us_buf)
			player.startPlay();
		if (ctrl_param[MCTRL_PAUSE_TYPE] == MenuMidiControl::MIDI_IN_NOTE && ctrl_param[MCTRL_PAUSE_VALUE] == us_buf)
			player.pause();
		if (ctrl_param[MCTRL_STOP_TYPE] == MenuMidiControl::MIDI_IN_NOTE && ctrl_param[MCTRL_STOP_VALUE] == us_buf)
			CSTask->stop_song_notify();
		if (ctrl_param[MCTRL_NEXT_SONG_TYPE] == MenuMidiControl::MIDI_IN_NOTE && ctrl_param[MCTRL_NEXT_SONG_VALUE] == us_buf)
			CSTask->list_next_song_notify();
		if (ctrl_param[MCTRL_PREV_SONG_TYPE] == MenuMidiControl::MIDI_IN_NOTE && ctrl_param[MCTRL_PREV_SONG_VALUE] == us_buf)
			CSTask->list_prev_song_notify();

		if(player.state() == Player::PLAYER_IDLE)
		{
			for (uint8_t i = 0; i < 99; i++)
			{
				if (pc_param[i * 2] == 2 && pc_param[i * 2 + 1] == us_buf)
				{
					CSTask->load_song_notify(us_buf);
					break;
				}
			}
		}
		midi_buf = 4;
		break;
	case 0xc0: // PC
		if (ctrl_param[MCTRL_START_TYPE] == MenuMidiControl::MIDI_IN_PC  && ctrl_param[MCTRL_START_VALUE] == us_buf)
			player.startPlay();
		if (ctrl_param[MCTRL_PAUSE_TYPE] == MenuMidiControl::MIDI_IN_PC && ctrl_param[MCTRL_PAUSE_VALUE] == us_buf)
			player.pause();
		if (ctrl_param[MCTRL_STOP_TYPE] == MenuMidiControl::MIDI_IN_PC && ctrl_param[MCTRL_STOP_VALUE] == us_buf)
			CSTask->stop_song_notify();
		if (ctrl_param[MCTRL_NEXT_SONG_TYPE] == MenuMidiControl::MIDI_IN_PC && ctrl_param[MCTRL_NEXT_SONG_VALUE] == us_buf)
			CSTask->list_next_song_notify();
		if (ctrl_param[MCTRL_PREV_SONG_TYPE] == MenuMidiControl::MIDI_IN_PC && ctrl_param[MCTRL_PREV_SONG_VALUE] == us_buf)
			CSTask->list_prev_song_notify();


		if(player.state() == Player::PLAYER_IDLE)
		{
			for (uint8_t i = 0; i < 99; i++)
			{
				if (!pc_param[i * 2] && pc_param[i * 2 + 1] == us_buf)
				{
					CSTask->load_song_notify(us_buf);
					break;
				}
			}
		}
		midi_buf = 0;
		break;
	case 0xf3: // song select
		if(currentMenu->menuType() == MENU_PLAYER && player.state() == Player::PLAYER_IDLE)
		{
			if(us_buf < 99)
			{
				CSTask->load_song_notify(us_buf);
			}
		}
		midi_buf = 0;
		break;
	case 4:
		midi_buf = 0;
		break;
	}
	usart_send_blocking(USART1, us_buf);
}
