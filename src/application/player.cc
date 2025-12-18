#include "player.h"

#include "init.h"
#include "fs_stream.h"

#include "display.h"
#include "leds.h"

const wav_sample_t& Player::sample(uint8_t trackNum)
{
	if(m_songPoint < FsStreamTask->selectedSong.trackSize[trackNum])
		return soundBuff[trackNum][m_buffPoint];
	else
		return emptySample;
}

void Player::incrementSoundPos()
{
	if (m_buffPoint == 0)
		FsStreamTask->data_notify(&second_target);

	if (m_buffPoint == wav_buff_size / 2)
		FsStreamTask->data_notify(&first_target);


	m_songPoint++;
	m_buffPoint++;
	m_buffPoint &= wav_buff_size - 1;
}

void Player::decrementSoundPos()
{
	if(m_buffPoint > 0)
	{
		m_buffPoint--;
		m_songPoint--;
	}
}

void Player::initSong()
{
	m_state = PLAYER_LOADING_SONG;

	FsStreamTask->pos(0);
	countUp = 0;
	m_songPoint = 0;

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
	m_buffPoint = 0;
	m_state = PLAYER_IDLE;
}

void Player::startPlay()
{
	m_state = PLAYER_PLAYING;
	m_songPoint = 0;

	Leds::greenOn();
	Leds::redOff();
}

void Player::stopPlay()
{
	m_state = PLAYER_IDLE;
	m_songPoint = 0;

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

	m_buffPoint = 0;
	m_songPoint = pos;

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

