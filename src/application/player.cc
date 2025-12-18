#include "player.h"

#include "init.h"
#include "fs_stream.h"

#include "display.h"
#include "leds.h"

void Player::initSong()
{
	m_state = PLAYER_LOADING_SONG;

	FsStreamTask->pos(0);
	countUp = 0;

	for(uint32_t a=0; a < maxTrackCount; a++)
	{
		for (uint32_t i = 0; i < (wav_buff_size); i++)
		{
			soundBuff[a][i].left = 0;
			soundBuff[a][i].right = 0;
		}
	}
}

void Player::songInitiated()
{
	m_state = PLAYER_IDLE;
}

void Player::startPlay()
{
	m_state = PLAYER_PLAYING;

	Leds::greenOn();
	Leds::redOff();
}

void Player::stopPlay()
{
	m_state = PLAYER_IDLE;

	Leds::redOn();
	Leds::greenOff();
}

void Player::pause()
{
	switch(m_state)
	{
	case PLAYER_PLAYING: m_state = PLAYER_PAUSE; break;
	case PLAYER_PAUSE:
	{
		m_state = PLAYER_PLAYING;
		Leds::greenOn();
		break;
	}
	default: break;
	}
}


void Player::jumpToPosition(uint32_t pos)
{
	FsStreamTask->pos(pos);
	player.countUp = pos / 4410.0f;

	DisplayTask->Sec_Print(player.counterValue());

	memset(soundBuff, 0, Player::wav_buff_size * maxTrackCount);
}

void Player::starMetronome()
{
	m_state = METRONOME_PLAYING;
}

void Player::stopMetronome()
{
	m_state = PLAYER_IDLE;
}

uint32_t Player::counterValue()
{
	if(m_state == PLAYER_IDLE)
	{
		return FsStreamTask->selectedSong.songSize();
	}
	else
	{
		if(sys_param[direction_counter])
			return FsStreamTask->selectedSong.songSize() - countUp;
		else
			return countUp;
	}
}

