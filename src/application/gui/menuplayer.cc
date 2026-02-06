#include "menuplayer.h"

#include "../miditask.h"
#include "init.h"
#include "display.h"
#include "fs_stream.h"
#include "enc.h"

#include "menumetronome.h"
#include "menumain.h"

#include "parambase.h"

#include "player.h"

#include "leds.h"

MenuPlayer::MenuPlayer()
{
	m_menuType = MENU_PLAYER;
	m_loopModeActive = false;
	test_file();
}

MenuPlayer::~MenuPlayer()
{

}

void MenuPlayer::show(TShowMode showMode)
{
	runningNamePos = 0;

	emb_string path_old = "/SONGS";

	if(player.state() == Player::PLAYER_IDLE) // && !type) // type - function parameter
	{
		if (!test_file())
			if (m_currentSongNum < 99 && FsStreamTask->selectedSong.songSize() > 0)
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

	Leds::digit(m_currentSongNum);
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

		if(midiPlayer.midiFileValid())
			DisplayTask->SymbolOut(14, 1, (uint8_t)'M');
		if(FsStreamTask->selectedSong.playNext)
			DisplayTask->SymbolOut(15, 1, SYMBOL_NEXT_MARK);

		DisplayTask->Sec_Print(player.counterValue());
	}
	else
	{
		DisplayTask->Clear();

		uint8_t prog_temp = m_currentSongNum + 1;
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

		if(midiPlayer.midiFileValid())
			DisplayTask->SymbolOut(14, 1, (uint8_t)'M');
		if (FsStreamTask->selectedSong.playNext)
			DisplayTask->SymbolOut(15, 1, SYMBOL_NEXT_MARK);

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
	midiPlayer.jumpToPos(player.countUp * 4410);
}

void MenuPlayer::encoderCounterClockwise()
{
	if(player.countUp)
		player.countUp = ParamBase::encSpeedDec(player.countUp, 0);

	player.jumpToPosition(player.countUp * 4410);
	midiPlayer.jumpToPos(player.countUp * 4410);
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
		m_currentSongNum = (m_currentSongNum + 10) % 99;
		uint8_t retryCount = 0;
		while(retryCount<100)
		{
			if(loadSong(m_currentSongNum))
			{
				retryCount++;
				m_currentSongNum = (m_currentSongNum + 1) % 99;
			}
			else
			{
				Leds::digit(m_currentSongNum);
				break;
			}

		}
		initSong();
		m_loopModeActive = false;
	}
}

void MenuPlayer::keyLeftDown()
{
	if(player.state() == Player::PLAYER_IDLE)
	{
		dela(0xfffff);
		m_currentSongNum = (m_currentSongNum - 10) % 99;
		uint8_t retryCount = 0;
		while(retryCount<100)
		{
			if(loadSong(m_currentSongNum))
			{
				retryCount++;
				m_currentSongNum = (m_currentSongNum - 1) % 99;
			}
			else
			{
				Leds::digit(m_currentSongNum);
				break;
			}
		}
		initSong();
		m_loopModeActive = false;
	}
}

void MenuPlayer::keyRightUp()
{
	if(player.state() == Player::PLAYER_IDLE)
	{
		dela(0xfffff);
		m_currentSongNum = (m_currentSongNum - 1) % 99;
		uint8_t retryCount = 0;
		while(retryCount<100)
		{
			if(loadSong(m_currentSongNum))
			{
				retryCount++;
				m_currentSongNum = (m_currentSongNum - 1) % 99;
			}
			else
			{
				Leds::digit(m_currentSongNum);
				break;
			}
		}
		initSong();
		m_loopModeActive = false;
	}
}

void MenuPlayer::keyRightDown()
{
	if(player.state() == Player::PLAYER_IDLE)
	{
		dela(0xfffff);
		m_currentSongNum = (m_currentSongNum + 1) % 99;
		uint8_t retryCount = 0;
		while(retryCount<100)
		{
			if(loadSong(m_currentSongNum))
			{
				retryCount++;
				m_currentSongNum = (m_currentSongNum + 1) % 99;
			}
			else
			{
				Leds::digit(m_currentSongNum);
				break;
			}
		}
		initSong();
		m_loopModeActive = false;
	}
}

void MenuPlayer::keyReturn()
{
	if(no_file) return;

	if(m_loopModeActive)
		player.jumpToLp1();
}

void MenuPlayer::keyReturnLong()
{
	if(no_file) return;

	m_loopModeActive = true;

	playPoint1Selected = 1;
	player.setLoopPoint1();

	Leds::digitPoint1On();
	Leds::requestLed1Blinking();
}

void MenuPlayer::keyForward()
{
	if(no_file) return;

	if(m_loopModeActive)
		player.jumpToLp2();
}

void MenuPlayer::keyForwardLong()
{
	if(no_file) return;

	m_loopModeActive = true;

	playPoint2Selected = 1;
	player.setLoopPoint2();

	Leds::digitPoint2On();
	Leds::requestLed2Blinking();
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

bool MenuPlayer::loadSong(uint8_t songNum)
{
	while(player.state() == Player::PLAYER_LOADING_SONG);

	emb_string songPath;
	emb_printf::sprintf(songPath, "%s/%1.ego", FsStreamTask->browserPlaylistFolder().c_str(), songNum);
	if(FsStreamTask->selectedSong.load(songPath))
	{
		return 1;
	}
	else
	{
		midiPlayer.loadSong(songPath);

		DisplayTask->Clear();

		emb_string path_old = "/SONGS";
		FsStreamTask->curr_path(path_old);

		m_currentSongName = FsStreamTask->selectedSong.songName();
		oem2winstar(m_currentSongName);

		runningNamePos = 0;
		printRunningName(m_currentSongName);

		if(midiPlayer.midiFileValid()) DisplayTask->SymbolOut(14, 1, (uint8_t)'M');
		if(FsStreamTask->selectedSong.playNext) DisplayTask->SymbolOut(15, 1, SYMBOL_NEXT_MARK);
		DisplayTask->Sec_Print(FsStreamTask->selectedSong.songSize());

		initSong();
		player.resetLoopPoints();

		playPoint1Selected = playPoint2Selected = 0;

		return 0;
	}
}

void MenuPlayer::setSongNum(uint8_t songNum)
{
	if(songNum < 99)
	{
		m_currentSongNum = songNum;
		Leds::digit(m_currentSongNum);
	}
}

bool MenuPlayer::test_file()
{
	uint8_t temp = 0;
	no_file = 0;

	for (; m_currentSongNum < 100; m_currentSongNum++)
	{
		if (!loadSong(m_currentSongNum))
			break;
	}

	if (m_currentSongNum > 98)
	{
		m_currentSongNum = 0;
		m_currentSongName.clear();
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
