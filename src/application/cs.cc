#include "cs.h"
#include "appdefs.h"
#include "init.h"
#include "display.h"
#include "fs_stream.h"

#include "enc.h"
#include "init.h"

#include "menuplayer.h"
#include "menusystem.h"
#include "menumidicontrol.h"
#include "menumidipc.h"

#include "libopencm3/stm32/gpio.h"
#include "miditask.h"

TCSTask *CSTask;
TSofwareTimer *DisplayBlinkTask;

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

	MenuSystem::read_sys();
	MenuMidiControl::read_ctrl();
	MenuMidiPc::read_map();

	menuPlayer = new MenuPlayer();
	currentMenu = menuPlayer;
	currentMenu->show();

	while (1)
	{
		sem->Take(portMAX_DELAY);

		if(currentMenu)
		{
			currentMenu->task();

			switch(key_ind)
			{
			case key_stop: currentMenu->keyStop(); break;
			case key_stop_long: currentMenu->keyStopLong(); break;
			case key_start: currentMenu->keyStart(); break;
			case key_left_up: currentMenu->keyLeftUp(); break;
			case key_left_down: currentMenu->keyLeftDown(); break;
			case key_right_up: currentMenu->keyRightUp(); break;
			case key_right_down: currentMenu->keyRightDown(); break;
			case key_return: currentMenu->keyReturn(); break;
			case key_return_long: currentMenu->keyReturnLong(); break;
			case key_forward: currentMenu->keyForward(); break;
			case key_forward_long: currentMenu->keyForwardLong(); break;
			case key_esc: currentMenu->keyEsc(); break;
			case key_encoder: currentMenu->encoderPress(); break;
			case key_encoder_long: currentMenu->encoderLongPress(); break;
			}

			if(encoder_rotated)
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

			encoder_rotated = encoder_key = key_ind = 0;
		}


		query_notify_t qn;
		if(NotifyWait(0, 0, (uint32_t*) &qn, 0))
		{
			switch(qn.notify)
			{
			case qn_stop_song:
			{
				player.stopPlay();
				player.initSong();
				DisplayTask->Sec_Print(player.counterValue());
				break;
			}

			case qn_load_song:
			{
				uint8_t requestedSongNum = qn.songNum;
				bool loadErr = menuPlayer->loadSong(requestedSongNum);
				if(!loadErr)
				{
					menuPlayer->setSongNum(requestedSongNum);
				}
				break;
			}

			case qn_play_next_song:
			{
				menuPlayer->keyStop();

				bool startNextSong = FsStreamTask->selectedSong.playNext;

				while(player.state() == Player::PLAYER_LOADING_SONG)

				player.initSong();
				player.resetLoopPoints();

				uint8_t currentSongNum = (menuPlayer->songNum() + 1) % 99;

				uint8_t result = 1;
				while(result)
				{
					result = menuPlayer->loadSong(currentSongNum);
					currentSongNum = (currentSongNum + 1) % 99;
				}

				menuPlayer->setSongNum(currentSongNum);

				Delay(100);

				if(startNextSong && sys_param[auto_next_track])
				{
					player.startPlay();
					us_buf1 = 0xfa;
					MIDITask->Give();
				}
				else
				{
					player.startPlay();
					player.pause();
				}
				break;
			}

			case qn_list_next_song: menuPlayer->keyRightDown(); break;
			case qn_list_prev_song: menuPlayer->keyRightUp(); break;
			}
		}
	}
}

