#ifndef SRC_APPLICATION_PLAYER_H_
#define SRC_APPLICATION_PLAYER_H_

#include "appdefs.h"

class Player
{
public:
	enum State
	{
		UNKNOWN,
		PLAYER_IDLE,
		PLAYER_LOADING_SONG, 	// play_fl1
		PLAYER_PLAYING,	// play_fl, play_fl2
		PLAYER_PAUSE,
		METRONOME_PLAYING
	};

	static constexpr uint8_t maxTrackCount = 4;
	static constexpr size_t wav_buff_size = sizeof(wav_sample_t) * 512;

	wav_sample_t soundBuff[maxTrackCount][wav_buff_size];

	State state() { return m_state; }

	void startPlay();
	void stopPlay();
	void pause();
	void jumpToPosition(uint32_t pos);

	void initSong();
	void songInitiated();

private:
	State m_state{PLAYER_IDLE};
};



#endif /* SRC_APPLICATION_PLAYER_H_ */
