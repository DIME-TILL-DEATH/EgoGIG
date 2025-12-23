#include "menuplayer.h"

#include "init.h"
#include "display.h"
#include "fs_stream.h"
#include "midi.h"
#include "enc.h"

#include "menumetronome.h"
#include "menumain.h"

#include "parambase.h"

#include "player.h"

#include "leds.h"

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
				DisplayTask->Sec_Print(FsStreamTask->selectedSong.songSize());
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

		DisplayTask->Sec_Print(FsStreamTask->selectedSong.songSize());
	}

	Leds::digit(num_prog);
	Leds::redOn();
	Leds::menuGreenOn();

	if(playPoint1Selected)
		Leds::digitPoint1On();
	if(playPoint2Selected)
		Leds::digitPoint2On();
}

void MenuPlayer::refresh()
{

}

void MenuPlayer::task()
{
	if(player.state() == Player::PLAYER_PAUSE)
	{
		if(tim5_fl)
			Leds::greenOn();
		else
			Leds::greenOff();
	}

	if(tim5_fl) printRunningName(m_currentSongName);
}

void MenuPlayer::processPlayNext()
{
	if(m_requestPlayNext)
	{
		m_requestPlayNext = false;

		while(player.state() == Player::PLAYER_LOADING_SONG)

		player.initSong();

		num_prog = (num_prog + 1) % 99;

		while (1)
		{
			if (loadSong())
				num_prog = (num_prog + 1) % 99;
			else
				break;
		}

		Leds::digit(num_prog);

		taskDelay(100);

		player.startPlay();

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

		DisplayTask->Sec_Print(player.counterValue());
	}
	else
	{
		DisplayTask->Clear();

		uint8_t prog_temp = num_prog + 1;
		for (; prog_temp < 100; prog_temp++)
		{
			emb_string songPath;
			emb_printf::sprintf(songPath, "%s/%1.ego", FsStreamTask->browserPlaylistFolder().c_str(), prog_temp);
			if(FsStreamTask->editingSong.load(songPath) == Song::eOk)
			{
				break;
			}
		}

		if (prog_temp != 100)
		{
			oem2winstar(FsStreamTask->editingSong.trackName[0]);
			runningNamePos = 0;
			printRunningName(FsStreamTask->editingSong.trackName[0]);
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
			DisplayTask->Sec_Print(player.counterValue());
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
	if(player.countUp < FsStreamTask->selectedSong.songSize())
		player.countUp = ParamBase::encSpeedInc(player.countUp, FsStreamTask->selectedSong.songSize());

	player.jumpToPosition(player.countUp * 4410);
}

void MenuPlayer::encoderCounterClockwise()
{
	if(player.countUp)
		player.countUp = ParamBase::encSpeedDec(player.countUp, 0);

	player.jumpToPosition(player.countUp * 4410);
}

void MenuPlayer::keyStop()
{
	if(no_file) return;

	player.stopPlay();

	// midi send
	us_buf1 = 0xfc;
	MIDITask->Give();

	initSong();

	DisplayTask->Sec_Print(player.counterValue());
}

void MenuPlayer::keyStopLong()
{
	if(no_file) return;

	if(m_loopModeActive)
	{
		m_loopModeActive = false;

		Leds::digitPoint1Off();
		Leds::digitPoint2Off();
	}
	else
	{
		m_loopModeActive = true;
		if (playPoint1Selected)
			Leds::digitPoint1On();
		if (playPoint2Selected)
			Leds::digitPoint2On();
	}
}

void MenuPlayer::keyStart()
{
	if(no_file) return;

	switch(player.state())
	{
		case Player::PLAYER_IDLE:
		{
			if(playPoint1Selected && m_loopModeActive && sys_param[loop_points])
				player.jumpToLp1();

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
		Leds::digit(num_prog);

		keyStop();
	}
}

void MenuPlayer::keyLeftDown()
{
	if(player.state() == Player::PLAYER_IDLE)
	{
		dela(0xfffff);

		num_prog = (num_prog - 10) % 99;

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
		Leds::digit(num_prog);

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
		Leds::digit(num_prog);

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
		Leds::digit(num_prog);
		keyStop();
	}

}

void MenuPlayer::keyReturn()
{
	if(no_file) return;

	player.jumpToLp1();
}

void MenuPlayer::keyReturnLong()
{
	if(no_file) return;

	playPoint1Selected = 1;
	player.setLoopPoint1();

	Leds::digitPoint1On();
}

void MenuPlayer::keyForward()
{
	if(no_file) return;

	player.jumpToLp1();
}

void MenuPlayer::keyForwardLong()
{
	if(no_file) return;

	playPoint2Selected = 1;
	player.setLoopPoint2();

	Leds::digitPoint2On();
}

void MenuPlayer::keyEsc()
{
	if(player.state() == Player::PLAYER_IDLE)
	{
		tim7_start(1);

		Leds::redOff();
		Leds::greenOff();
		Leds::menuRedOn();

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
		DisplayTask->Sec_Print(FsStreamTask->selectedSong.songSize());

		initSong();

		playPoint1Selected = playPoint2Selected = 0;

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
