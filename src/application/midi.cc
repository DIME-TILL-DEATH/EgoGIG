#include "midi.h"
#include "appdefs.h"
#include "cs.h"
#include "init.h"
#include "libopencm3/stm32/usart.h"

#include "gui.h"

#include "menumidicontrol.h"

TMIDITask *MIDITask;

uint8_t us_buf;
uint8_t us_buf1;
uint8_t irq_fl = 0;
void TMIDITask::Code()
{
	while (1)
	{
		sem->Take(portMAX_DELAY);
		usart_send_blocking(USART1, us_buf1);
	}
}

//extern volatile uint8_t play_fl2;
//extern volatile uint8_t stop_fl;
//extern volatile uint8_t stop_fl1;
//extern volatile uint8_t pause_fl;
//extern volatile uint8_t play_fl1;

uint8_t midi_buf = 0;
uint8_t midi_ev = 0;

extern "C" void USART1_IRQHandler()
{
	USART1_SR &= ~USART_SR_RXNE;
	us_buf = USART1_DR;
	switch (midi_buf)
	{
	case 0:
		if (us_buf == 0xfa)
		{
//			play_fl = 1;
//			pause_fl = 0;
//			play_fl2 = 1;
			player.startPlay();
			key_reg_out[0] |= 2;
			key_reg_out[0] &= ~0x80;
			midi_buf = 0;
		}
		if (us_buf == 0xfb)
		{
			player.pause();
//			pause_fl = 1;
//			play_fl = 0;
			midi_buf = 0;
		}
		if (us_buf == 0xfc)
		{
			key_ind = key_stop;
			CSTask->Give();
			midi_buf = 0;
		}
		if (us_buf == (0xb0 | ctrl_param[chann]))
			midi_buf = 0xb0;
		if (us_buf == (0x90 | ctrl_param[chann]))
			midi_buf = 0x90;
		if (us_buf == (0xc0 | ctrl_param[chann]))
			midi_buf = 0xc0;
		break;
	case 0xb0: //CC
		if (ctrl_param[ctrl1_t] == MenuMidiControl::MIDI_IN_CC  && ctrl_param[ctrl1] == us_buf)
		{
//			play_fl = 1;
//			pause_fl = 0;
//			play_fl2 = 1;
			player.startPlay();

			key_reg_out[0] |= 2;
			key_reg_out[0] &= ~0x80;
		}
		if (ctrl_param[ctrl2_t] == MenuMidiControl::MIDI_IN_CC && ctrl_param[ctrl2] == us_buf)
		{
//			pause_fl = 1;
//			play_fl = 0;
			player.pause();
		}
		if (ctrl_param[ctrl3_t] == MenuMidiControl::MIDI_IN_CC && ctrl_param[ctrl3] == us_buf)
		{
			key_ind = key_stop;
			CSTask->Give();
		}

		if(player.state() == Player::PLAYER_IDLE)
		{
			for (uint8_t i = 0; i < 99; i++)
			{
				if (pc_param[i * 2] == 1 && pc_param[i * 2 + 1] == us_buf)
				{
					num_prog = (i - 1) % 99;
					key_ind = key_right_down;
					CSTask->Give();
					break;
				}
			}
		}
		midi_buf = 4;
		break;
	case 0x90: // Note
		if (ctrl_param[ctrl1_t] == MenuMidiControl::MIDI_IN_NOTE && ctrl_param[ctrl1] == us_buf)
		{
//			play_fl = 1;
//			pause_fl = 0;
//			play_fl2 = 1;
			player.startPlay();

			key_reg_out[0] |= 2;
			key_reg_out[0] &= ~0x80;
		}
		if (ctrl_param[ctrl2_t] == MenuMidiControl::MIDI_IN_NOTE && ctrl_param[ctrl2] == us_buf)
		{
//			pause_fl = 1;
//			play_fl = 0;
			player.pause();
		}
		if (ctrl_param[ctrl3_t] == MenuMidiControl::MIDI_IN_NOTE && ctrl_param[ctrl3] == us_buf)
		{
			key_ind = key_stop;
			CSTask->Give();
		}

		if(player.state() == Player::PLAYER_IDLE)
		{
			for (uint8_t i = 0; i < 99; i++)
			{
				if (pc_param[i * 2] == 2 && pc_param[i * 2 + 1] == us_buf)
				{
					key_ind = key_right_down;
					num_prog = (i - 1) % 99;
					CSTask->Give();
					break;
				}
			}
		}
		midi_buf = 4;
		break;
	case 0xc0: // PC
		if (ctrl_param[ctrl1_t] == MenuMidiControl::MIDI_IN_PC  && ctrl_param[ctrl1] == us_buf)
		{
//			play_fl = 1;
//			pause_fl = 0;
//			play_fl2 = 1;
			player.startPlay();

			key_reg_out[0] |= 2;
			key_reg_out[0] &= ~0x80;
		}
		if (ctrl_param[ctrl2_t] == MenuMidiControl::MIDI_IN_PC && ctrl_param[ctrl2] == us_buf)
		{
//			pause_fl = 1;
//			play_fl = 0;
			player.pause();
		}
		if (ctrl_param[ctrl3_t] == MenuMidiControl::MIDI_IN_PC && ctrl_param[ctrl3] == us_buf)
		{
			key_ind = key_stop;
			CSTask->Give();
		}

		if(player.state() == Player::PLAYER_IDLE)
		{
			for (uint8_t i = 0; i < 99; i++)
			{
				if (!pc_param[i * 2] && pc_param[i * 2 + 1] == us_buf)
				{
					key_ind = key_right_down;
					num_prog = (i - 1) % 99;
					CSTask->Give();
					break;
				}
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
