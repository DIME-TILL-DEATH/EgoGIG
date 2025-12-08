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


uint8_t system_list[][11] =
{ "Auto next ", "Count dir ", "Scroll dir", "LB points " };
uint8_t off_on[][5] =
{ "Off ", "On  " };
uint8_t up_down[][5] =
{ "Up  ", "Down" };
uint8_t contrl_list[][12] =
{ "Play ", "Pause", "Stop ", "Channel", "MIDI PC Set" };
uint8_t note_list[][3] =
{ "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };
uint8_t pc_list[][5] =
{ "PC  ", "CC  ", "Note" };

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



uint8_t sys_param[64];
uint8_t ctrl_param[32];
uint8_t pc_param[256];

uint8_t edit_fl;
uint8_t midi_pc = 0;

volatile uint32_t audio_pos;
volatile uint32_t play_next_file = 0;


uint8_t line_buf[5] =
{ 0, 0, 0, 0 };


inline void check_busy(uint8_t type, uint8_t val, uint8_t num, uint8_t id_comm)
{
	DisplayTask->StringOut(11, 1, (uint8_t*) "    ");
	if (!id_comm)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			if (val == ctrl_param[i * 2 + 1] && num != i)
			{
				for (uint8_t j = 0; j < 3; j++)
				{
					if (type == ctrl_param[j * 2])
						DisplayTask->StringOut(11, 1, (uint8_t*) "Busy");
				}
			}
		}
		for (uint8_t j = 0; j < 99; j++)
		{
			if (val == pc_param[j * 2 + 1] && num != j)
			{
				if (type == pc_param[j * 2])
					DisplayTask->StringOut(11, 1, (uint8_t*) "Busy");
			}
		}
	}
}
inline void string_lin(uint8_t col, uint8_t page, uint8_t val)
{
	DisplayTask->StringOut(5, 1, (uint8_t*) "    ");
	line_buf[0] = val / 100 + 48;
	if (line_buf[0] == 48)
		line_buf[0] = 32;
	line_buf[1] = val % 100 / 10 + 48;
	if (line_buf[1] == 48 && line_buf[0] == 32)
		line_buf[1] = 32;
	line_buf[2] = val % 100 % 10 + 48;
	line_buf[3] = 0;
	DisplayTask->StringOut(col, page, (uint8_t*) line_buf);
}
inline void midi_note_print(uint8_t val)
{
	DisplayTask->StringOut(5, 1, (uint8_t*) "    ");
	DisplayTask->StringOut(5, 1, (uint8_t*) note_list + (val % 12) * 3);
	if (val < 12)
		DisplayTask->StringOut(7, 1, (uint8_t*) "-1");
	else
	{
		line_buf[0] = val / 12 - 1 + 48;
		line_buf[1] = 32;
		line_buf[2] = 0;
		DisplayTask->StringOut(7, 1, (uint8_t*) line_buf);
	}
}
inline void midi_type_print(uint8_t type)
{
	switch (type)
	{
	case 0:
		DisplayTask->StringOut(0, 1, (uint8_t*) "CC  ");
		break;
	case 1:
		DisplayTask->StringOut(0, 1, (uint8_t*) "Note");
		break;

	}
}
inline void ev_midi_print(uint8_t type, uint8_t val)
{
	switch (type)
	{
	case 0:
		midi_type_print(type);
		string_lin(5, 1, val + 1);
		DisplayTask->StringOut(5, 1, (uint8_t*) line_buf);
		break;
	case 1:
		midi_type_print(type);
		midi_note_print(val);
		break;
	}
}

void clean_fl(void)
{
	encoder_state1 = encoder_key = key_ind = stp_dub_fl = ret_dub_fl =
			fwd_dub_fl = esc_dub_fl = enc_dub_fl = 0;
}




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

		clean_fl();
	}
	return;


	switch (condish)
	{
//------------------------------------Menu------------------------------------------------
	case menu:
//		if (tim5_fl)
//			DisplayTask->Clear_str(0, num_menu & 1, 15);
//		else
//			DisplayTask->StringOut(0, num_menu & 1,
//					(uint8_t*) menu_list + num_menu * 16);
//		if (encoder_state1)
//		{
//			if (encoder_state == 1)
//			{
//				if (num_menu)
//				{
//					if (num_menu == 2)
//					{
//						DisplayTask->Clear();
//						DisplayTask->StringOut(0, 0, (uint8_t*) menu_list);
//						num_menu--;
//						DisplayTask->SymbolOut(15, 1, 62);
//						tim7_start(1);
//					}
//					else
//					{
//						DisplayTask->StringOut(0, num_menu & 1,
//								(uint8_t*) menu_list + num_menu-- * 16);
//						tim7_start(0);
//					}
//					DisplayTask->StringOut(0, num_menu & 1,
//							(uint8_t*) menu_list + num_menu * 16);
//					clean_fl();
//				}
//			}
//			else
//			{
//				if ((num_menu < 3) && stop_fl1)
//				{
//					if (num_menu == 1)
//					{
//						DisplayTask->Clear();
//						DisplayTask->SymbolOut(15, 0, 60);
//						num_menu++;
//						DisplayTask->StringOut(0, 1,
//								(uint8_t*) menu_list + 3 * 16);
//						tim7_start(1);
//					}
//					else
//					{
//						DisplayTask->StringOut(0, num_menu & 1,
//								(uint8_t*) menu_list + num_menu++ * 16);
//						tim7_start(0);
//					}
//					DisplayTask->StringOut(0, num_menu & 1,
//							(uint8_t*) menu_list + num_menu * 16);
//					clean_fl();
//				}
//			}
//		}
		if (key_ind == key_encoder)
		{
			switch (num_menu)
			{
			case 1:
			{
				emb_string path_old = "/SONGS";
				FsStreamTask->enter_dir(path_old.c_str(), "", true);
				num_prog_edit = 0;
				load_led(num_prog_edit);
				num_tr_fl = 0;
				condish = edit_playlist;
				DisplayTask->Clear();
				DisplayTask->StringOut(4, 0, (uint8_t*) "Browser");
				DisplayTask->StringOut(1, 1, (uint8_t*) "Edit Playlist");
				processingTask->Delay(1000);
				DisplayTask->Clear();
				DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
				emb_string tmp;
				if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
				{
					oem2winstar(tmp);
					DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
				}
				else
					DisplayTask->StringOut(2, 0,
							(uint8_t*) "  No wav file");
			}
				break;
			case 0:
			{
				if (stop_fl1)
				{
					FsStreamTask->enter_dir("/PLAYLIST", "", true);
					FsStreamTask->next_notify();
					condish = select_folder;
					DisplayTask->Clear();
					DisplayTask->StringOut(4, 0, (uint8_t*) "Select");
					DisplayTask->StringOut(0, 1,
							(uint8_t*) "Playlist folder");
					processingTask->Delay(1000);
					DisplayTask->Clear();
					emb_string tmp;
					FsStreamTask->browser_name(tmp);
					DisplayTask->Clear();
					oem2winstar(tmp);
					DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
					num_prog = 0;
				}
			}
				break;
			case 2:
			{
				if (stop_fl1)
				{
					condish = sys_menu;
					num_menu = 0;
					DisplayTask->Clear();
					DisplayTask->StringOut(0, 0, (uint8_t*) system_list);
					DisplayTask->StringOut(0, 1,
							(uint8_t*) system_list + 11);
					DisplayTask->StringOut(11, 0,
							(uint8_t*) off_on
									+ sys_param[auto_next_track] * 5);
					DisplayTask->StringOut(11, 1,
							(uint8_t*) up_down
									+ sys_param[direction_counter] * 5);
					DisplayTask->SymbolOut(15, 1, 62);
//					tim7_start(1);
				}
			}
				break;
			case 3:
				if (stop_fl1)
				{
					condish = midi_ctrl_menu;
					DisplayTask->Clear();
					num_menu = 0;
					edit_fl = 0;
					DisplayTask->StringOut(0, 0, (uint8_t*) contrl_list);
					ev_midi_print(ctrl_param[ctrl1_t], ctrl_param[ctrl1]);
				}
				break;
			}
			clean_fl();
		}
		if (key_ind == key_esc)
		{
//			init_play_menu(0);
//			parentMenu->returnFromChildMenu();
			clean_fl();
		}
		break;
//-----------------------------------Set Control MIDI-------------------------------------
	case midi_ctrl_menu:
		if (tim5_fl)
		{
			if (!edit_fl)
				DisplayTask->Clear_str(0, 0, 15);
			else
			{
				DisplayTask->StringOut(0, 0,
						(uint8_t*) contrl_list + num_menu * 12);
				if (edit_fl == 1)
					DisplayTask->StringOut(0, 1, (uint8_t*) "    ");
				else
				{
					if (edit_fl == 2)
						DisplayTask->StringOut(5, 1, (uint8_t*) "    ");
				}
			}
			if (num_menu == 3)
				DisplayTask->StringOut(0, 1, (uint8_t*) "Num ");
		}
		else
		{
			if (!edit_fl)
				DisplayTask->StringOut(0, 0,
						(uint8_t*) contrl_list + num_menu * 12);
			else
			{
				if (edit_fl == 1)
					midi_type_print(ctrl_param[num_menu * 2]);
				else
				{
					if (edit_fl == 2)
					{
						if (ctrl_param[num_menu * 2])
							midi_note_print(ctrl_param[num_menu * 2 + 1]);
						else
							string_lin(5, 1,
									ctrl_param[num_menu * 2 + 1] + 1);
					}
				}
			}
			if (num_menu == 3)
			{
				DisplayTask->StringOut(0, 1, (uint8_t*) "Num ");
				string_lin(5, 1, ctrl_param[num_menu * 2 + 1] + 1);
			}
		}
		if (encoder_state1)
		{
			if (encoder_state == 1)
			{
				if (!edit_fl)
				{
					if (num_menu)
					{
						DisplayTask->StringOut(0, 0,
								(uint8_t*) contrl_list + --num_menu * 12);
						if (num_menu < 3)
							(ctrl_param[num_menu * 2], ctrl_param[num_menu
									* 2 + 1]);
						else
						{
							DisplayTask->StringOut(0, 1, (uint8_t*) "Num ");
							string_lin(5, 1,
									ctrl_param[num_menu * 2 + 1] + 1);
						}
						DisplayTask->StringOut(11, 1, (uint8_t*) "    ");
					}
				}
				if (edit_fl == 1)
				{
					uint8_t temp = ctrl_param[num_menu * 2];
					if (temp)
					{
						temp--;
						ev_midi_print(temp, ctrl_param[num_menu * 2 + 1]);
						check_busy(temp, ctrl_param[num_menu * 2 + 1],
								num_menu + 1, 0);
						ctrl_param[num_menu * 2] = temp;
					}
				}
				if (edit_fl == 2)
				{
					uint8_t temp = ctrl_param[num_menu * 2 + 1];
					if (temp)
					{
						if (num_menu != 3)
						{
							temp = enc_speed_dec(temp, 0);
							ev_midi_print(ctrl_param[num_menu * 2], temp);
							ctrl_param[num_menu * 2 + 1] = temp;
							check_busy(ctrl_param[num_menu * 2], temp,
									num_menu + 1, 0);
						}
						else
						{
							ctrl_param[num_menu * 2 + 1]--;
							DisplayTask->StringOut(0, 1, (uint8_t*) "Num ");
							string_lin(5, 1,
									ctrl_param[num_menu * 2 + 1] + 1);
						}
					}
				}
			}
			else
			{
				if (!edit_fl)
				{
					if (num_menu < 4)
					{
						DisplayTask->StringOut(0, 0,
								(uint8_t*) contrl_list + ++num_menu * 12);
						if (num_menu < 3)
							ev_midi_print(ctrl_param[num_menu * 2],
									ctrl_param[num_menu * 2 + 1]);
						else
						{
							if (num_menu < 4)
							{
								DisplayTask->StringOut(0, 1,
										(uint8_t*) "Num ");
								string_lin(5, 1,
										ctrl_param[num_menu * 2 + 1] + 1);
							}
							else
								DisplayTask->StringOut(8, 1,
										(uint8_t*) "       ");
						}
						DisplayTask->StringOut(11, 1, (uint8_t*) "    ");
					}
				}
				if (edit_fl == 1)
				{
					uint8_t temp = ctrl_param[num_menu * 2];
					if (!temp)
					{
						temp++;
						ev_midi_print(temp, ctrl_param[num_menu * 2 + 1]);
						check_busy(temp, ctrl_param[num_menu * 2 + 1],
								num_menu + 1, 0);
						ctrl_param[num_menu * 2] = temp;
					}
				}
				if (edit_fl == 2)
				{
					uint8_t temp = ctrl_param[num_menu * 2 + 1];
					if (temp < 127)
					{
						if (num_menu < 3)
						{
							temp = enc_speed_inc(temp, 127);
							ev_midi_print(ctrl_param[num_menu * 2], temp);
							ctrl_param[num_menu * 2 + 1] = temp;
							check_busy(ctrl_param[num_menu * 2], temp,
									num_menu + 1, 0);
						}
						else
						{
							if (ctrl_param[num_menu * 2 + 1] < 15)
								ctrl_param[num_menu * 2 + 1]++;
							DisplayTask->StringOut(0, 1, (uint8_t*) "Num ");
							string_lin(5, 1,
									ctrl_param[num_menu * 2 + 1] + 1);
						}
					}
				}
			}
//			tim7_start(1);
		}
		if (key_ind == key_encoder)
		{
			edit_fl++;
			edit_fl %= 3;
			if (edit_fl == 1)
				if (num_menu == 3)
					edit_fl++;
			ev_midi_print(ctrl_param[num_menu * 2],
					ctrl_param[num_menu * 2 + 1]);
			if (num_menu == 3)
			{
				DisplayTask->StringOut(0, 1, (uint8_t*) "Num ");
				string_lin(5, 1, ctrl_param[num_menu * 2 + 1] + 1);
			}
			if (num_menu == 4)
			{
				DisplayTask->Clear();
				DisplayTask->StringOut(8, 1, (uint8_t*) "       ");
				edit_fl = num_menu = midi_pc = 0;
				string_lin(7, 0, midi_pc + 1);
				DisplayTask->StringOut(0, 0, (uint8_t*) "Song #");
				DisplayTask->StringOut(0, 1,
						(uint8_t*) pc_list + pc_param[midi_pc * 2] * 5);
				string_lin(5, 1, pc_param[midi_pc * 2 + 1] + 1);
				condish = midi_pc_menu;
			}
			if (!edit_fl)
				DisplayTask->StringOut(11, 1, (uint8_t*) "    ");
//			tim7_start(0);
		}
		if (key_ind == key_esc)
		{
			write_ctrl();
			condish = menu;
			DisplayTask->Clear();
			DisplayTask->SymbolOut(15, 0, 60);
			num_menu = 3;
			edit_fl = 0;
//			DisplayTask->StringOut(0, 0, (uint8_t*) menu_list + 2 * 16);
//			tim7_start(1);
		}
		clean_fl();
		break;
//------------------------------------midi prog ch set------------------------------------
	case midi_pc_menu:
		if (tim5_fl)
		{
			if (!edit_fl)
				DisplayTask->StringOut(7, 0, (uint8_t*) "   ");
			else
			{
				if (edit_fl == 1)
					DisplayTask->StringOut(0, 1, (uint8_t*) "    ");
				else
				{
					if (edit_fl == 2)
						DisplayTask->StringOut(5, 1, (uint8_t*) "      ");
					else
					{
						if (pc_param[midi_pc * 2] != 2)
							string_lin(5, 1, pc_param[midi_pc * 2 + 1] + 1);
						else
							midi_note_print(pc_param[midi_pc * 2 + 1]);
					}
				}
			}
		}
		else
		{
			if (!edit_fl)
				string_lin(7, 0, midi_pc + 1);
			else
			{
				if (edit_fl == 1)
					DisplayTask->StringOut(0, 1,
							(uint8_t*) pc_list + pc_param[midi_pc * 2] * 5);
			}
			if (pc_param[midi_pc * 2] != 2)
				string_lin(5, 1, pc_param[midi_pc * 2 + 1] + 1);
			else
				midi_note_print(pc_param[midi_pc * 2 + 1]);
		}
		if (encoder_state1)
		{
			if (encoder_state == 1)
			{
				if (!edit_fl)
				{
					if (midi_pc)
						midi_pc = enc_speed_dec(midi_pc, 0);
					DisplayTask->StringOut(0, 1,
							(uint8_t*) pc_list + pc_param[midi_pc * 2] * 5);
					if (pc_param[midi_pc * 2] != 2)
						string_lin(5, 1, pc_param[midi_pc * 2 + 1] + 1);
					else
						midi_note_print(pc_param[midi_pc * 2 + 1]);
				}
				if (edit_fl == 1)
				{
					if (pc_param[midi_pc * 2])
						pc_param[midi_pc * 2]--;
				}
				if (edit_fl == 2)
				{
					uint8_t temp = pc_param[midi_pc * 2 + 1];
					if (temp)
					{
						temp = enc_speed_dec(temp, 0);
						check_busy(pc_param[midi_pc * 2], temp, midi_pc, 1);
						pc_param[midi_pc * 2 + 1] = temp;
					}
				}
			}
			else
			{
				if (!edit_fl)
				{
					if (midi_pc < 98)
						midi_pc = enc_speed_inc(midi_pc, 98);
					DisplayTask->StringOut(0, 1,
							(uint8_t*) pc_list + pc_param[midi_pc * 2] * 5);
					if (pc_param[midi_pc * 2] != 2)
						string_lin(5, 1, pc_param[midi_pc * 2 + 1] + 1);
					else
						midi_note_print(pc_param[midi_pc * 2 + 1]);
				}
				if (edit_fl == 1)
				{
					if (pc_param[midi_pc * 2] < 2)
						pc_param[midi_pc * 2]++;
				}
				if (edit_fl == 2)
				{
					uint8_t temp = pc_param[midi_pc * 2 + 1];
					if (temp < 127)
					{
						temp = enc_speed_inc(temp, 127);
						check_busy(pc_param[midi_pc * 2], temp, midi_pc, 1);
						pc_param[midi_pc * 2 + 1] = temp;
					}
				}
			}
//			tim7_start(1);
		}
		if (key_ind == key_encoder)
		{
			edit_fl++;
			edit_fl %= 3;
			if (edit_fl == 1)
				string_lin(7, 0, midi_pc + 1);
			if (edit_fl == 2)
				DisplayTask->StringOut(0, 1,
						(uint8_t*) pc_list + pc_param[midi_pc * 2] * 5);
		}
		if (key_ind == key_esc)
		{
			DisplayTask->Clear();
			write_map();
			num_menu = 4;
			edit_fl = 0;
			condish = midi_ctrl_menu;
			DisplayTask->StringOut(0, 0, (uint8_t*) contrl_list + 4 * 16);
//			tim7_start(0);
		}
		clean_fl();
		break;
//------------------------------------System----------------------------------------------
	case sys_menu:
		if (tim5_fl)
		{
			if (!enc_key_fl)
				DisplayTask->Clear_str(0, num_menu & 1, 10);
			else
				DisplayTask->Clear_str(11, num_menu & 1, 4);
		}
		else
		{
			if (!enc_key_fl)
				DisplayTask->StringOut(0, num_menu & 1,
						(uint8_t*) system_list + num_menu * 11);
			else
			{
				if (!num_menu || num_menu == 3)
					DisplayTask->StringOut(11, num_menu & 1,
							(uint8_t*) off_on
									+ sys_param[num_menu + auto_next_track]
											* 5);
				else
					DisplayTask->StringOut(11, num_menu & 1,
							(uint8_t*) up_down
									+ sys_param[num_menu + auto_next_track]
											* 5);
			}
		}
		if (encoder_state1)
		{
			if (encoder_state == 1)
			{
				if (!enc_key_fl)
				{
					if (num_menu)
					{
						DisplayTask->StringOut(0, num_menu & 1,
								(uint8_t*) system_list + num_menu-- * 11);
						if (num_menu == 1)
						{
							DisplayTask->Clear();
							DisplayTask->StringOut(0, 0,
									(uint8_t*) system_list);
							DisplayTask->StringOut(0, 1,
									(uint8_t*) system_list + 11);
							DisplayTask->StringOut(11, 0,
									(uint8_t*) off_on
											+ sys_param[auto_next_track]
													* 5);
							DisplayTask->StringOut(11, 1,
									(uint8_t*) up_down
											+ sys_param[direction_counter]
													* 5);
							DisplayTask->SymbolOut(15, 1, 62);
//							tim7_start(1);
						}
						DisplayTask->StringOut(0, num_menu & 1,
								(uint8_t*) system_list + num_menu * 11);
//						tim7_start(0);
					}
				}
				else
				{
					if (sys_param[num_menu + auto_next_track])
					{
						switch (num_menu)
						{
						case 0:
							DisplayTask->StringOut(11, 0,
									(uint8_t*) off_on
											+ --sys_param[auto_next_track]
													* 5);
							break;
						case 1:
							DisplayTask->StringOut(11, 1,
									(uint8_t*) up_down
											+ --sys_param[direction_counter]
													* 5);
							break;
						case 2:
							DisplayTask->StringOut(11, 0,
									(uint8_t*) up_down
											+ --sys_param[direction_scrol_playlist]
													* 5);
							break;
						case 3:
							DisplayTask->StringOut(11, 1,
									(uint8_t*) off_on
											+ --sys_param[loop_points] * 5);
							break;
						}
//						tim7_start(1);
					}
				}
			}
			else
			{       //-----------------------encoder up---------------
				if (!enc_key_fl)
				{
					if (num_menu < 3)
					{
						DisplayTask->StringOut(0, 0,
								(uint8_t*) system_list + num_menu++ * 11);
						if (num_menu == 2)
						{
							DisplayTask->Clear();
							DisplayTask->SymbolOut(15, 0, 60);
							DisplayTask->StringOut(0, 1,
									(uint8_t*) system_list
											+ (num_menu + 1) * 11);
							DisplayTask->StringOut(11, 0,
									(uint8_t*) up_down
											+ sys_param[direction_scrol_playlist]
													* 5);
							DisplayTask->StringOut(11, 1,
									(uint8_t*) off_on
											+ sys_param[loop_points] * 5);
						}
						DisplayTask->StringOut(0, num_menu & 1,
								(uint8_t*) system_list + num_menu * 11);
//						tim7_start(0);
					}
				}
				else
				{
					if (!sys_param[num_menu + auto_next_track])
					{
						switch (num_menu)
						{
						case 0:
							DisplayTask->StringOut(11, 0,
									(uint8_t*) off_on
											+ ++sys_param[auto_next_track]
													* 5);
							break;
						case 1:
							DisplayTask->StringOut(11, 1,
									(uint8_t*) up_down
											+ ++sys_param[direction_counter]
													* 5);
							break;
						case 2:
							DisplayTask->StringOut(11, 0,
									(uint8_t*) up_down
											+ ++sys_param[direction_scrol_playlist]
													* 5);
							break;
						case 3:
							DisplayTask->StringOut(11, 1,
									(uint8_t*) off_on
											+ ++sys_param[loop_points] * 5);
							break;
						}
//						tim7_start(1);
					}
				}
			}
			clean_fl();
		}
		if (key_ind == key_encoder)
		{
			if (!enc_key_fl)
			{
				enc_key_fl = 1;
				DisplayTask->StringOut(0, num_menu & 1,
						(uint8_t*) system_list + num_menu * 11);
			}
			else
			{
				enc_key_fl = 0;
				if (!num_menu || num_menu == 3)
					DisplayTask->StringOut(11, num_menu & 1,
							(uint8_t*) off_on + sys_param[num_menu] * 5);
				else
					DisplayTask->StringOut(11, num_menu & 1,
							(uint8_t*) up_down + sys_param[num_menu] * 5);
			}
			clean_fl();
//			tim7_start(0);
		}
		if (key_ind == key_esc)
		{
			enc_key_fl = 0;
			write_sys();
//			init_play_menu(1);
//			parentMenu->returnFromChildMenu();
			clean_fl();
		}

		break;
//------------------------------------Edit Play List--------------------------------------
	case edit_playlist:
		if (key_ind == key_return)
		{
			num_tr_fl = 0;
			DisplayTask->Clear();
			DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
			emb_string tmp;
			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
			{
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
			}
			else
				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
			clean_fl();
		}
		if (key_ind == key_forward)
		{
			num_tr_fl = 1;
			DisplayTask->Clear();
			DisplayTask->StringOut(0, 0, (uint8_t*) "2:");
			emb_string tmp;
			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 1))
			{
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
			}
			else
				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
			clean_fl();
		}
		if (key_ind == key_left_down)
		{
			if (num_prog_edit > 10)
				num_prog_edit -= 10;
			load_led(num_prog_edit);
			num_tr_fl = 0;
			act_fl = 0;
			clean_fl();
			emb_string tmp;
			DisplayTask->Clear_str(2, 0, 30);
			DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
			{
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
			}
			else
				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
		}
		if (key_ind == key_left_up)
		{
			num_prog_edit = (num_prog_edit + 10) % 99;
			load_led(num_prog_edit);
			num_tr_fl = 0;
			act_fl = 0;
			clean_fl();
			emb_string tmp;
			DisplayTask->Clear_str(2, 0, 30);
			DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
			{
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
			}
			else
				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
		}
		if (key_ind == key_right_up)
		{
			if (num_prog_edit)
				num_prog_edit--;
			load_led(num_prog_edit);
			num_tr_fl = 0;
			act_fl = 0;
			clean_fl();
			emb_string tmp;
			DisplayTask->Clear_str(2, 0, 30);
			DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
			{
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
			}
			else
				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
		}
		if (key_ind == key_right_down)
		{
			num_prog_edit = (num_prog_edit + 1) % 99;
			load_led(num_prog_edit);
			num_tr_fl = 0;
			act_fl = 0;
			clean_fl();
			emb_string tmp;
			DisplayTask->Clear_str(2, 0, 30);
			DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
			{
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
			}
			else
				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
		}
		if (key_ind == key_esc)
		{
//			init_play_menu(1);
//			parentMenu->returnFromChildMenu();
			clean_fl();
		}
		if (encoder_state1)
		{
			if (encoder_state == 2)
			{
				FsStreamTask->next_notify();
				emb_string tmp;
				FsStreamTask->browser_name(tmp);
				DisplayTask->Clear();
				if (!num_tr_fl)
					DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
				else
					DisplayTask->StringOut(0, 0, (uint8_t*) "2:");
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
			}
			else
			{
				FsStreamTask->prev_notify();
				emb_string tmp;
				emb_string tmp1;
				FsStreamTask->browser_name(tmp1);
				FsStreamTask->curr_path(tmp);
				if (!tmp.compare("/SONGS") && !tmp1.compare(".."))
					FsStreamTask->next_notify();
				else
				{
					DisplayTask->Clear();
					if (!num_tr_fl)
						DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
					else
						DisplayTask->StringOut(0, 0, (uint8_t*) "2:");
					oem2winstar(tmp1);
					DisplayTask->StringOut(2, 0, (uint8_t*) tmp1.c_str());
				}
			}
			clean_fl();
		}
		if (key_ind == key_encoder)
		{
			emb_string tmp;
			emb_string tmp1;
			FsStreamTask->browser_name(tmp1);
			FsStreamTask->curr_path(tmp);
			if (!(!tmp.compare("/SONGS") && !tmp1.compare("..")))
			{
				if (!num_tr_fl)
				{
					if (enc_dub_fl)
						play_next_file = 1;
					else
						play_next_file = 0;
				}
				FsStreamTask->action_notify(
						num_tr_fl ?
								TFsStreamTask::action_param_t::ap_2_wav :
								TFsStreamTask::action_param_t::ap_1_wav,
						num_prog_edit, 0);
				if (act_fl)
//					no_file = 0; // in MenuPlayer

				if (!act_fl && num_tr_fl)
				{
					DisplayTask->Clear();
					DisplayTask->StringOut(0, 0,
							(uint8_t*) "  First assign       track1");
					processingTask->Delay(2000);
					DisplayTask->Clear();
					DisplayTask->StringOut(0, 0, (uint8_t*) "2:");
				}
				//emb_string tmp ;
				FsStreamTask->browser_name(tmp);
				DisplayTask->Clear_str(2, 0, 14);
				DisplayTask->Clear_str(0, 1, 16);
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
				if (FsStreamTask->file_flag() == true)
				{
					if (play_next_file)
						DisplayTask->StringOut(15, 1, (uint8_t*) ">");
					else
						DisplayTask->Clear_str(15, 1, 1);
				}
				else
					DisplayTask->Clear_str(15, 1, 1);
			}
			clean_fl();
		}
		if (key_ind == key_stop)
		{
			DisplayTask->Clear();
			DisplayTask->StringOut(2, 0, (uint8_t*) "Remove song");
			DisplayTask->StringOut(1, 1, (uint8_t*) "from playlist?");
			clean_fl();
			condish = delete_file;
		}

		break;
//----------------------------------------------Delete file-----------------------------------
	case delete_file:
		if (key_ind == key_start)
		{
			clean_fl();
			DisplayTask->Clear();
			DisplayTask->StringOut(5, 0, (uint8_t*) "SURE?");
			while (1)
			{
				if (key_ind == key_start)
				{
					FsStreamTask->delete_track(num_prog_edit);
					DisplayTask->StringOut(2, 0, (uint8_t*) "Song deleted");
					processingTask->Delay(1000);
					break;
				}
				if (key_ind == key_stop)
					break;
			}
			DisplayTask->Clear();
			DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
			emb_string tmp;
			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
			{
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
			}
			else
				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
			num_tr_fl = 0;
			condish = edit_playlist;
		}
		if (key_ind == key_stop)
		{
			DisplayTask->Clear();
			DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
			clean_fl();
			emb_string tmp;
			if (!FsStreamTask->open_song_name(num_prog_edit, tmp, 0, 0))
			{
				oem2winstar(tmp);
				DisplayTask->StringOut(2, 0, (uint8_t*) tmp.c_str());
			}
			else
				DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
			num_tr_fl = 0;
			condish = edit_playlist;
		}
		break;
//------------------------------------Select Playlist Folder------------------------------
	case select_folder:
		if (encoder_state1)
		{
			if (encoder_state == 2)
			{
				FsStreamTask->next_notify();
				emb_string tmp;
				FsStreamTask->browser_name(tmp);
				DisplayTask->Clear();
				oem2winstar(tmp);
				DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
			}
			else
			{
				FsStreamTask->prev_notify();
				emb_string tmp;
				emb_string tmp1;
				FsStreamTask->browser_name(tmp1);
				FsStreamTask->curr_path(tmp);
				if (!tmp.compare("/PLAYLIST") && !tmp1.compare(".."))
					FsStreamTask->next_notify();
				else
				{
					DisplayTask->Clear();
					oem2winstar(tmp1);
					DisplayTask->StringOut(0, 0, (uint8_t*) tmp1.c_str());
				}
			}
			clean_fl();
		}
		if (key_ind == key_encoder)
		{
			if (enc_dub_fl)
			{
				if (!FsStreamTask->play_list_folder())
				{
					DisplayTask->Clear();
					DisplayTask->StringOut(2, 0, (uint8_t*) "Not Folder!");
				}
				else
				{
					DisplayTask->Clear();
					DisplayTask->StringOut(3, 0, (uint8_t*) "Select Ok!");
					processingTask->Delay(1000);
					emb_string tmp;
					FsStreamTask->browser_name(tmp);
					DisplayTask->Clear();
					oem2winstar(tmp);
					DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
				}
			}
			else
			{
				FsStreamTask->action_notify(
						TFsStreamTask::action_param_t::ap_1_wav, 0, 0);
				emb_string tmp;
				FsStreamTask->browser_name(tmp);
				DisplayTask->Clear();
				oem2winstar(tmp);
				DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
			}
			clean_fl();
		}
		if (key_ind == key_esc)
		{
			//			init_play_menu(0);
			//			parentMenu->returnFromChildMenu();
			clean_fl();
		}
		break;

//------------------------------------End--------------------------------------------------
	}
}

void write_sys(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/system.ego", FA_WRITE);
	f_write(&fsys, sys_param, 64, &br);
	f_close(&fsys);
}

void read_sys(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/system.ego", FA_OPEN_ALWAYS | FA_READ);
	f_read(&fsys, sys_param, 64, &br);
	f_close(&fsys);
}
void write_ctrl(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/ctrl.ego", FA_WRITE);
	f_write(&fsys, ctrl_param, 32, &br);
	f_close(&fsys);
}

void read_ctrl(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/ctrl.ego", FA_OPEN_ALWAYS | FA_READ);
	f_read(&fsys, ctrl_param, 32, &br);
	f_close(&fsys);
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
