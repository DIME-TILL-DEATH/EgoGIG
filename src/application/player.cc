#include "player.h"

#include "init.h"
#include "fs_stream.h"

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
}

void Player::stopPlay()
{
	m_state = PLAYER_IDLE;
}

void Player::pause()
{
	switch(m_state)
	{
	case PLAYER_PLAYING: m_state = PLAYER_PAUSE; break;
	case PLAYER_PAUSE: m_state = PLAYER_PLAYING; break;
	}
}
