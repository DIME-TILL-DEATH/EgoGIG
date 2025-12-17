#include "menuplayer.h"

#include "init.h"
#include "display.h"
#include "fs_stream.h"
#include "midi.h"
#include "enc.h"

#include "menumetronome.h"
#include "menumain.h"

#include "player.h"

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

	if (player.state() == Player::PLAYER_IDLE) // && !type) // type - function parameter
	{
		if (!test_file())
			if (num_prog < 99)
				DisplayTask->Sec_Print(FsStreamTask->sound_size());
	}
	else
	{
		DisplayTask->Clear();
		FsStreamTask->curr_path(path_old);
		m_currentSongName = FsStreamTask->selectedSong.songName();
		oem2winstar(m_currentSongName);
		runningNamePos = 0;
		printRunningName(m_currentSongName);

		if(FsStreamTask->selectedSong.playNext)
			DisplayTask->SymbolOut(15, 1, SYMBOL_NEXT_MARK);

		DisplayTask->Sec_Print(FsStreamTask->sound_size());
	}

	load_led(num_prog);

	key_reg_out[0] |= 0x10;
	key_reg_out[0] &= ~0x8;

	if (playPoint1Selected)
		key_reg_out[1] &= ~(1 << 7);
	if (playPoint2Selected)
		key_reg_out[1] &= ~(1 << 15);
}

void MenuPlayer::refresh()
{

}

void MenuPlayer::task()
{
	if(player.state() == Player::PLAYER_PAUSE)
	{
		// Led blink
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

		while(player.state() == Player::PLAYER_LOADING_SONG)

		memset(sound_buff, 0, Player::wav_buff_size);
		memset(click_buff, 0, Player::wav_buff_size);
		num_prog = (num_prog + 1) % 99;

		while (1)
		{
			if (loadSong())
				num_prog = (num_prog + 1) % 99;
			else
				break;
		}

		load_led(num_prog);

		taskDelay(100);

		player.startPlay();

		key_reg_out[0] |= 2;
		key_reg_out[0] &= ~0x80;

		us_buf1 = 0xfa;
		MIDITask->Give();
	}
}

void MenuPlayer::encoderPress()
{
	if(no_file) return;

	if(player.state() == Player::PLAYER_IDLE)
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


		if(FsStreamTask->selectedSong.playNext)
			DisplayTask->StringOut(15, 1, (uint8_t*) SYMBOL_NEXT_MARK);

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
//			if(!FsStreamTask->open_song_name(prog_temp, tmp, 0, 0))
//				break;
		}

		if (prog_temp != 100)
		{
			oem2winstar(tmp);
			runningNamePos = 0;
			printRunningName(tmp);
		}
		else
		{
			DisplayTask->StringOut(2, 0, (uint8_t*) "Playlist End");
		}

		taskDelay(1500);

		DisplayTask->Clear();
		runningNamePos = 0;
		printRunningName(m_currentSongName);

		if (FsStreamTask->selectedSong.playNext)
			DisplayTask->StringOut(15, 1, (uint8_t*) SYMBOL_NEXT_MARK);

		if(player.state() == Player::PLAYER_PAUSE)
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
	if(player.state() == Player::PLAYER_IDLE)
	{
		showChild(new MenuMetronome(this));
	}
}

void MenuPlayer::encoderClockwise()
{
	if(count_up < song_size)
		count_up = enc_speed_inc(count_up, song_size);

	count_down = song_size - count_up;

	if (sys_param[direction_counter])
		DisplayTask->Sec_Print(count_down);
	else
		DisplayTask->Sec_Print(count_up);

	FsStreamTask->pos(count_up * 4410);
	memset(sound_buff, 0, Player::wav_buff_size);
	memset(click_buff, 0, Player::wav_buff_size);
	sound_point = 0;
}

void MenuPlayer::encoderCounterClockwise()
{
	if (count_up)
		count_up = enc_speed_dec(count_up, 0);

	count_down = song_size - count_up;

	if (sys_param[direction_counter])
		DisplayTask->Sec_Print(count_down);
	else
		DisplayTask->Sec_Print(count_up);

	FsStreamTask->pos(count_up * 4410);
	memset(sound_buff, 0, Player::wav_buff_size);
	memset(click_buff, 0, Player::wav_buff_size);
	sound_point = 0;
}

void MenuPlayer::keyStop()
{
	if(no_file) return;

	player.stopPlay();

	key_reg_out[0] &= ~2;
	key_reg_out[0] |= 0x80;

	// midi send
	us_buf1 = 0xfc;
	MIDITask->Give();

	initSong();

	DisplayTask->Sec_Print(count_down);
	count_up = 0;

}

void MenuPlayer::keyStopLong()
{
	if(no_file) return;

	if(m_loopModeActive)
	{
		m_loopModeActive = false;
		key_reg_out[1] |= 1 << 7;
		key_reg_out[1] |= 1 << 15;
	}
	else
	{
		m_loopModeActive = true;
		if (playPoint1Selected)
			key_reg_out[1] &= ~(1 << 7);
		if (playPoint2Selected)
			key_reg_out[1] &= ~(1 << 15);
	}
}

void MenuPlayer::keyStart()
{
	if(no_file) return;

	key_reg_out[0] |= 2; //green led light

	switch(player.state())
	{
		case Player::PLAYER_IDLE:
		{
			key_reg_out[0] &= ~0x80;

			if(playPoint1Selected && m_loopModeActive && sys_param[loop_points])
				player.jumpToPosition(play_point1);

			us_buf1 = 0xfa;
			MIDITask->Give();
			usart_wait_send_ready(USART1);

			player.startPlay();
			break;
		}
		case Player::PLAYER_PLAYING:
		case Player::PLAYER_PAUSE:
		{
			player.pause();

			us_buf1 = 0xfb;
			MIDITask->Give();
			break;
		}
		default: break;
	}
}

void MenuPlayer::keyLeftUp()
{
	if(player.state() == Player::PLAYER_IDLE)
	{
		dela(0xfffff);
		num_prog = (num_prog + 10) % 99;

		while (1)
		{
			if (loadSong())
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
	if(player.state() == Player::PLAYER_IDLE)
	{
		dela(0xfffff);
		if (num_prog > 10)
			num_prog -= 10;

		while (1)
		{
			if (loadSong())
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
	if(player.state() == Player::PLAYER_IDLE)
	{
		dela(0xfffff);
		num_prog = (num_prog - 1) % 99;
		while (1)
		{
			if (loadSong())
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
	if(player.state() == Player::PLAYER_IDLE)
	{
		dela(0xfffff);
		num_prog = (num_prog + 1) % 99;
		while (1)
		{
			if (loadSong())
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

	player.jumpToPosition(play_point1);
}

void MenuPlayer::keyReturnLong()
{
	if(no_file) return;

	play_point1 = FsStreamTask->pos();
	playPoint1Selected = 1;
	if (!play_point2)
		play_point2 = song_size * 4410;
	key_reg_out[1] &= ~(1 << 7);
}

void MenuPlayer::keyForward()
{
	if(no_file) return;

	player.jumpToPosition(play_point2);
}

void MenuPlayer::keyForwardLong()
{
	if(no_file) return;

	play_point2 = FsStreamTask->pos();
	playPoint2Selected = 1;
	key_reg_out[1] &= ~(1 << 15);
}

void MenuPlayer::keyEsc()
{
	if(player.state() == Player::PLAYER_IDLE)
	{
		tim7_start(1);
		key_reg_out[0] &= ~0x10;
		key_reg_out[0] |= 0x8;

		showChild(new MenuMain(this));
	}
}

bool MenuPlayer::loadSong()
{
	while(player.state() == Player::PLAYER_LOADING_SONG);

	emb_string songPath;
	emb_printf::sprintf(songPath, "%s/%1.ego", FsStreamTask->browserPlaylistFolder().c_str(), num_prog);
	if(FsStreamTask->selectedSong.load(songPath))
	{
		return 1;
	}
	else
	{
		DisplayTask->Clear();

		emb_string path_old = "/SONGS";
		FsStreamTask->curr_path(path_old);

		m_currentSongName = FsStreamTask->selectedSong.songName();
		oem2winstar(m_currentSongName);

		runningNamePos = 0;
		printRunningName(m_currentSongName);

		if(FsStreamTask->selectedSong.playNext) DisplayTask->SymbolOut(15, 1, SYMBOL_NEXT_MARK);
		DisplayTask->Sec_Print(FsStreamTask->sound_size());

		initSong();
		song_size = count_down = FsStreamTask->sound_size();
		click_size = FsStreamTask->click_size();
		play_point1 = play_point2 = playPoint1Selected = playPoint2Selected = 0;
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
		if (!loadSong())
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

void MenuPlayer::initSong(void)
{
	while(player.state() == Player::PLAYER_LOADING_SONG);

	player.initSong();
}
