#include "player.h"

#include "init.h"
#include "fs_stream.h"

#include "display.h"
#include "leds.h"

void Player::initSong()
{
	m_state = PLAYER_LOADING_SONG;

	sound_point = 0;
	samp_point = 0;

	FsStreamTask->pos(0);

	msec_tik = 0;

	for (uint32_t i = 0; i < (wav_buff_size); i++)
	{
		sound_buff[i].left = 0;
		sound_buff[i].right = 0;
		click_buff[i].left = 0;
		click_buff[i].right = 0;
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
	count_up = pos / 4410.0f;
	count_down = song_size - count_up;

	if (sys_param[direction_counter])
		DisplayTask->Sec_Print(count_down);
	else
		DisplayTask->Sec_Print(count_up);

	memset(sound_buff, 0, Player::wav_buff_size);
	memset(click_buff, 0, Player::wav_buff_size);
}

