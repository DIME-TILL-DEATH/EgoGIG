#include "menuplayer.h"

#include "init.h"
#include "display.h"
#include "fs_stream.h"
#include "midi.h"
#include "enc.h"

#include "menumetronome.h"
#include "menumain.h"

MenuPlayer::MenuPlayer()
{
	m_menuType = MENU_PLAYER;
	test_file();
}

MenuPlayer::~MenuPlayer()
{

}

void MenuPlayer::show(TShowMode showMode)
{
	runningNamePos = 0;

	emb_string path_old = "/SONGS";

	if (stop_fl1) // && !type) // type - function parameter
	{
		if (!test_file())
			if (num_prog < 99)
				DisplayTask->Sec_Print(FsStreamTask->sound_size());
	}
	else
	{
//		loadProg();
		DisplayTask->Clear();
		FsStreamTask->curr_path(path_old);
		FsStreamTask->sound_name(m_currentSongName);
		oem2winstar(m_currentSongName);
//		DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
		runningNamePos = 0;
		printRunningName(m_currentSongName);

		if (FsStreamTask->next_pl())
			DisplayTask->SymbolOut(15, 1, SYMBOL_NEXT_MARK);
		DisplayTask->Sec_Print(FsStreamTask->sound_size());
	}

	load_led(num_prog);

	key_reg_out[0] |= 0x10;
	key_reg_out[0] &= ~0x8;
	if (play_point1_fl)
		key_reg_out[1] &= ~(1 << 7);
	if (play_point2_fl)
		key_reg_out[1] &= ~(1 << 15);
}

void MenuPlayer::refresh()
{

}

void MenuPlayer::task()
{
	if(pause_fl)
	{
		if(tim5_fl)
			key_reg_out[0] |= 2;
		else
			key_reg_out[0] &= ~2;
	}

	if(tim5_fl) printRunningName(m_currentSongName);
}

void MenuPlayer::processPlayNext()
{
	if(m_requestPlayNext)
	{
		m_requestPlayNext = false;
		while(!play_fl1);

		memset(sound_buff, 0, wav_buff_size);
		memset(click_buff, 0, wav_buff_size);
		num_prog = (num_prog + 1) % 99;

		while (1)
		{
			if (load_prog())
				num_prog = (num_prog + 1) % 99;
			else
				break;
		}

		load_led(num_prog);

		stop_fl1 = 0;
		pause_fl = 0;

		taskDelay(100);
		play_fl = play_fl2 = 1;
		key_reg_out[0] |= 2;
		key_reg_out[0] &= ~0x80;

		us_buf1 = 0xfa;
		MIDITask->Give();
	}
}

void MenuPlayer::encoderPress()
{
	if(no_file) return;

	if (stop_fl1)
	{
		DisplayTask->Clear();

		emb_string tmp;
		FsStreamTask->play_list_folder(tmp);
		DisplayTask->StringOut(0, 0, (uint8_t*) "PL->");
		oem2winstar(tmp);
		DisplayTask->StringOut(5, 0, (uint8_t*) tmp.c_str());
		taskDelay(1500);
		DisplayTask->Clear();

		runningNamePos = 0;
		printRunningName(m_currentSongName);


		if (FsStreamTask->next_pl())
			DisplayTask->StringOut(15, 1, (uint8_t*) ">");
		if (sys_param[direction_counter])
			DisplayTask->Sec_Print(count_down);
		else
			DisplayTask->Sec_Print(count_up);
	}
	else
	{
		DisplayTask->Clear();

		uint8_t prog_temp = num_prog + 1;
		emb_string tmp;
		for (; prog_temp < 100; prog_temp++)
		{
			if (!FsStreamTask->open_song_name(prog_temp, tmp, 0, 0))
				break;
		}
		if (prog_temp != 100)
		{
			oem2winstar(tmp);
			runningNamePos = 0;
			printRunningName(tmp);
		}
		else
			DisplayTask->StringOut(2, 0, (uint8_t*) "Playlist End");
		taskDelay(1500);

		DisplayTask->Clear();
		runningNamePos = 0;
		printRunningName(m_currentSongName);

		if (FsStreamTask->next_pl())
			DisplayTask->StringOut(15, 1, (uint8_t*) ">");
		if (pause_fl)
		{
			if (sys_param[direction_counter])
				DisplayTask->Sec_Print(count_down);
			else
				DisplayTask->Sec_Print(count_up);
		}
	}
}

void MenuPlayer::encoderLongPress()
{
	if(stop_fl1)
	{
		showChild(new MenuMetronome(this));
	}
}

void MenuPlayer::encoderClockwise()
{
	if(stop_fl1)
		stop_fl1 = 0;

	if(count_up < song_size)
		count_up = enc_speed_inc(count_up, song_size);

	count_down = song_size - count_up;

	if (sys_param[direction_counter])
		DisplayTask->Sec_Print(count_down);
	else
		DisplayTask->Sec_Print(count_up);

	FsStreamTask->pos(count_up * 4410);
	memset(sound_buff, 0, wav_buff_size);
	memset(click_buff, 0, wav_buff_size);
	sound_point = 0;

	if (play_fl2)
		play_fl = 1;
}

void MenuPlayer::encoderCounterClockwise()
{
	if (stop_fl1)
		stop_fl1 = 0;

	if (count_up)
		count_up = enc_speed_dec(count_up, 0);

	count_down = song_size - count_up;

	if (sys_param[direction_counter])
		DisplayTask->Sec_Print(count_down);
	else
		DisplayTask->Sec_Print(count_up);

	FsStreamTask->pos(count_up * 4410);
	memset(sound_buff, 0, wav_buff_size);
	memset(click_buff, 0, wav_buff_size);
	sound_point = 0;

	if (play_fl2)
		play_fl = 1;

}

void MenuPlayer::keyStop()
{
	if(no_file) return;

	if(!stp_dub_fl)
	{
		play_fl = 0;
		stop_fl1 = 1;
		play_fl2 = 0;
		pause_fl = 0;
		key_reg_out[0] &= ~2;
		key_reg_out[0] |= 0x80;
		us_buf1 = 0xfc;
		MIDITask->Give();
		init_prog();
		song_size = count_down = FsStreamTask->sound_size();
		click_size = FsStreamTask->click_size();
		DisplayTask->Sec_Print(count_down);
		count_up = 0;
	}
	else
	{
		if(!play_point_ind)
		{
			play_point_ind = 1;
			key_reg_out[1] |= 1 << 7;
			key_reg_out[1] |= 1 << 15;
		}
		else
		{
			play_point_ind = 0;
			if (play_point1_fl)
				key_reg_out[1] &= ~(1 << 7);
			if (play_point2_fl)
				key_reg_out[1] &= ~(1 << 15);
		}
	}
}

void MenuPlayer::keyStart()
{
	if(no_file) return;

	stop_fl = stop_fl1 = 0;
	if (!play_fl)
	{
		play_fl2 = 1;
		key_reg_out[0] |= 2;
		key_reg_out[0] &= ~0x80;
		if(play_point1_fl && !play_point_ind && !pause_fl && sys_param[loop_points])
			jump_rand_pos(play_point1);

		pause_fl = 0;
		us_buf1 = 0xfa;
		MIDITask->Give();
		usart_wait_send_ready(USART1);
		play_fl = 1;

	}
	else
	{
		pause_fl = 1;
		play_fl = play_fl2 = 0;
		us_buf1 = 0xfb;
		MIDITask->Give();
	}
}

void MenuPlayer::keyLeftUp()
{
	if(stop_fl1)
	{
		dela(0xfffff);
		num_prog = (num_prog + 10) % 99;

		while (1)
		{
			if (load_prog())
				num_prog = (num_prog + 1) % 99;
			else
				break;
		}
		load_led(num_prog);

		keyStop();
	}
}

void MenuPlayer::keyLeftDown()
{
	if(stop_fl1)
	{
		dela(0xfffff);
		if (num_prog > 10)
			num_prog -= 10;

		while (1)
		{
			if (load_prog())
			{
				if (num_prog)
					num_prog--;
				else
					num_prog = 100;
			}
			else
				break;
		}
		load_led(num_prog);

		keyStop();
	}
}

void MenuPlayer::keyRightUp()
{
	if(stop_fl1)
	{
		dela(0xfffff);
		num_prog = (num_prog - 1) % 99;
		while (1)
		{
			if (load_prog())
				num_prog = (num_prog - 1) % 99;
			else
				break;
		}
		load_led(num_prog);

		keyStop();
	}
}

void MenuPlayer::keyRightDown()
{
	if(stop_fl1)
	{
		dela(0xfffff);
		num_prog = (num_prog + 1) % 99;
		while (1)
		{
			if (load_prog())
				num_prog = (num_prog + 1) % 99;
			else
				break;
		}
		load_led(num_prog);
		keyStop();
	}

}

void MenuPlayer::keyReturn()
{
	if(no_file) return;

	if(ret_dub_fl)
	{
		play_point1 = FsStreamTask->pos();
		play_point1_fl = 1;
		if (!play_point2)
			play_point2 = song_size * 4410;
		key_reg_out[1] &= ~(1 << 7);
	}
	else
	{
		jump_rand_pos(play_point1);
	}
}

void MenuPlayer::keyForward()
{
	if(no_file) return;

	if (fwd_dub_fl)
	{
		play_point2 = FsStreamTask->pos();
		play_point2_fl = 1;
		key_reg_out[1] &= ~(1 << 15);
	}
	else
	{
		jump_rand_pos(play_point2);
	}
}

void MenuPlayer::keyEsc()
{
	if(stop_fl1)
	{
		tim7_start(1);
		key_reg_out[0] &= ~0x10;
		key_reg_out[0] |= 0x8;
		showChild(new MenuMain(this));
	}
}

bool MenuPlayer::load_prog()
{
	while (!play_fl1);

	if (FsStreamTask->open(num_prog))
	{
		return 1;
	}
	else
	{
		DisplayTask->Clear();

		emb_string path_old = "/SONGS";
		FsStreamTask->curr_path(path_old);
		FsStreamTask->sound_name(m_currentSongName);
		oem2winstar(m_currentSongName);

		runningNamePos = 0;
		printRunningName(m_currentSongName);

		if (FsStreamTask->next_pl()) DisplayTask->SymbolOut(15, 1, SYMBOL_NEXT_MARK);
		DisplayTask->Sec_Print(FsStreamTask->sound_size());

		init_prog();
		song_size = count_down = FsStreamTask->sound_size();
		click_size = FsStreamTask->click_size();
		play_point1 = play_point2 = play_point1_fl = play_point2_fl = 0;
		count_up = 0;

		return 0;
	}
}

bool MenuPlayer::test_file()
{
	uint8_t temp = 0;
	no_file = 0;

	for (; num_prog < 100; num_prog++)
	{
		if (!load_prog())
			break;
	}

	if (num_prog > 98)
	{
		num_prog = 0;
		temp = 1;
		no_file = 1;
		DisplayTask->Clear();
		DisplayTask->StringOut(2, 0, (uint8_t*) "No files in      Playlist");
		dela(0xffffff);
	}
	return temp;
}

void MenuPlayer::jump_rand_pos(uint32_t pos)
{
	FsStreamTask->pos(pos);
	count_up = pos / 4410.0f;
	count_down = song_size - count_up;

	if (sys_param[direction_counter])
		DisplayTask->Sec_Print(count_down);
	else
		DisplayTask->Sec_Print(count_up);

	memset(sound_buff, 0, wav_buff_size);
	memset(click_buff, 0, wav_buff_size);
}

void MenuPlayer::init_prog(void)
{
	play_fl = 0;
	while(!play_fl1);

	play_fl1 = 0;
	sound_point = 0;
	samp_point = 0;
	FsStreamTask->pos(0);
	msec_tik = 0;
	for (uint32_t i = 0; i < (wav_buff_size); i++)
		sound_buff[i].left = sound_buff[i].right = click_buff[i].left =
				click_buff[i].right = 0;
}
