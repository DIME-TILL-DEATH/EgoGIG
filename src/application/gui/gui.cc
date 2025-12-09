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


uint8_t num_menu = 0;
uint8_t num_prog = 0;
uint8_t num_prog_edit = 0;
const uint8_t led_sym[10] =
{ 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90 };

uint8_t condish = 0;
uint8_t tim5_fl = 0;
uint8_t blink_en = 0;
uint8_t num_tr_fl = 0;
uint8_t enc_key_fl = 0;


uint8_t edit_fl;

volatile uint32_t audio_pos;

uint8_t line_buf[5] =
{ 0, 0, 0, 0 };


void processGui(TTask* processingTask)
{
	if(currentMenu)
	{
		currentMenu->task();

		switch(key_ind)
		{
		case key_stop: currentMenu->keyStop(); break;
		case key_start: currentMenu->keyStart(); break;
		case key_left_up: currentMenu->keyLeftUp(); break;
		case key_left_down: currentMenu->keyLeftDown(); break;
		case key_right_up: currentMenu->keyRightUp(); break;
		case key_right_down: currentMenu->keyRightDown(); break;
		case key_return: currentMenu->keyReturn(); break;
		case key_forward: currentMenu->keyForward(); break;
		case key_esc: currentMenu->keyEsc(); break;
		case key_encoder:
		{
			if (!enc_dub_fl)
				currentMenu->encoderPress();
			else
				currentMenu->encoderLongPress();
			break;
		}
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

		encoder_state1 = encoder_key = key_ind = stp_dub_fl = ret_dub_fl = fwd_dub_fl = esc_dub_fl = enc_dub_fl = 0;
	}
	return;


	switch (condish)
	{
//------------------------------------midi prog ch set------------------------------------
//	case midi_pc_menu:
//		if (tim5_fl)
//		{
//			if (!edit_fl)
//				DisplayTask->StringOut(7, 0, (uint8_t*) "   ");
//			else
//			{
//				if (edit_fl == 1)
//					DisplayTask->StringOut(0, 1, (uint8_t*) "    ");
//				else
//				{
//					if (edit_fl == 2)
//						DisplayTask->StringOut(5, 1, (uint8_t*) "      ");
//					else
//					{
//						if (pc_param[midi_pc * 2] != 2)
//							string_lin(5, 1, pc_param[midi_pc * 2 + 1] + 1);
//						else
//							midi_note_print(pc_param[midi_pc * 2 + 1]);
//					}
//				}
//			}
//		}
//		else
//		{
//			if (!edit_fl)
//				string_lin(7, 0, midi_pc + 1);
//			else
//			{
//				if (edit_fl == 1)
//					DisplayTask->StringOut(0, 1,
//							(uint8_t*) pc_list + pc_param[midi_pc * 2] * 5);
//			}
//			if (pc_param[midi_pc * 2] != 2)
//				string_lin(5, 1, pc_param[midi_pc * 2 + 1] + 1);
//			else
//				midi_note_print(pc_param[midi_pc * 2 + 1]);
//		}
//		if (encoder_state1)
//		{
//			if (encoder_state == 1)
//			{
//				if (!edit_fl)
//				{
//					if (midi_pc)
//						midi_pc = enc_speed_dec(midi_pc, 0);
//					DisplayTask->StringOut(0, 1,
//							(uint8_t*) pc_list + pc_param[midi_pc * 2] * 5);
//					if (pc_param[midi_pc * 2] != 2)
//						string_lin(5, 1, pc_param[midi_pc * 2 + 1] + 1);
//					else
//						midi_note_print(pc_param[midi_pc * 2 + 1]);
//				}
//				if (edit_fl == 1)
//				{
//					if (pc_param[midi_pc * 2])
//						pc_param[midi_pc * 2]--;
//				}
//				if (edit_fl == 2)
//				{
//					uint8_t temp = pc_param[midi_pc * 2 + 1];
//					if (temp)
//					{
//						temp = enc_speed_dec(temp, 0);
//						check_busy(pc_param[midi_pc * 2], temp, midi_pc, 1);
//						pc_param[midi_pc * 2 + 1] = temp;
//					}
//				}
//			}
//			else
//			{
//				if (!edit_fl)
//				{
//					if (midi_pc < 98)
//						midi_pc = enc_speed_inc(midi_pc, 98);
//					DisplayTask->StringOut(0, 1,
//							(uint8_t*) pc_list + pc_param[midi_pc * 2] * 5);
//					if (pc_param[midi_pc * 2] != 2)
//						string_lin(5, 1, pc_param[midi_pc * 2 + 1] + 1);
//					else
//						midi_note_print(pc_param[midi_pc * 2 + 1]);
//				}
//				if (edit_fl == 1)
//				{
//					if (pc_param[midi_pc * 2] < 2)
//						pc_param[midi_pc * 2]++;
//				}
//				if (edit_fl == 2)
//				{
//					uint8_t temp = pc_param[midi_pc * 2 + 1];
//					if (temp < 127)
//					{
//						temp = enc_speed_inc(temp, 127);
//						check_busy(pc_param[midi_pc * 2], temp, midi_pc, 1);
//						pc_param[midi_pc * 2 + 1] = temp;
//					}
//				}
//			}
////			tim7_start(1);
//		}
//		if (key_ind == key_encoder)
//		{
//			edit_fl++;
//			edit_fl %= 3;
//			if (edit_fl == 1)
//				string_lin(7, 0, midi_pc + 1);
//			if (edit_fl == 2)
//				DisplayTask->StringOut(0, 1,
//						(uint8_t*) pc_list + pc_param[midi_pc * 2] * 5);
//		}
//		if (key_ind == key_esc)
//		{
//			DisplayTask->Clear();
//			write_map();
//			num_menu = 4;
//			edit_fl = 0;
//			condish = midi_ctrl_menu;
//			DisplayTask->StringOut(0, 0, (uint8_t*) contrl_list + 4 * 16);
////			tim7_start(0);
//		}
//		clean_fl();
//		break;
//----------------------------------------------Delete file-----------------------------------
//	case delete_file:
//		if (key_ind == key_start)
//		{
//			clean_fl();
//			DisplayTask->Clear();
//			DisplayTask->StringOut(5, 0, (uint8_t*) "SURE?");
//			while (1)
//			{
//				if (key_ind == key_start)
//				{
//					FsStreamTask->delete_track(num_prog_edit);
//					DisplayTask->StringOut(2, 0, (uint8_t*) "Song deleted");
//					processingTask->Delay(1000);
//					break;
//				}
//				if (key_ind == key_stop)
//					break;
//			}
//			DisplayTask->Clear();
//			DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
//			emb_string tmp;
//			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
//			{
//				oem2winstar(tmp);
//				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
//			}
//			else
//				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
//			num_tr_fl = 0;
//			condish = edit_playlist;
//		}
//		if (key_ind == key_stop)
//		{
//			DisplayTask->Clear();
//			DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
//			clean_fl();
//			emb_string tmp;
//			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
//			{
//				oem2winstar(tmp);
//				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
//			}
//			else
//				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
//			num_tr_fl = 0;
//			condish = edit_playlist;
//		}
//		break;

//------------------------------------End--------------------------------------------------
	}
}

void write_map(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/midi_map.ego", FA_WRITE);
	f_write(&fsys, pc_param, 256, &br);
	f_close(&fsys);
}

void read_map(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/midi_map.ego", FA_OPEN_ALWAYS | FA_READ);
	f_read(&fsys, pc_param, 256, &br);
	f_close(&fsys);
}
