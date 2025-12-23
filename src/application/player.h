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

	static constexpr uint8_t maxTrackCount = 2;
	static constexpr size_t wav_buff_size = sizeof(wav_sample_t) * 512;

	wav_sample_t soundBuff[maxTrackCount][wav_buff_size];
	uint32_t countUp{0};

	const wav_sample_t& sample(uint8_t trackNum);
	void incrementSoundPos();
	void decrementSoundPos();
	void processLoop();

	State state() { return m_state; }

	void startPlay();
	void stopPlay();
	void pause();

	void setLoopPoint1();
	void setLoopPoint2();

	void jumpToPosition(uint32_t pos);
	void jumpToLp1();
	void jumpToLp2();

	void starMetronome();
	void stopMetronome();

	void initSong();
	void songInitiated();

	uint32_t counterValue();

private:
	State m_state{PLAYER_IDLE};

	size_t m_buffPoint{0};
	uint32_t m_songPoint{0};

	uint32_t m_loopPoint1{0};
	uint32_t m_loopPoint2{0};

	const target_t first_target =
	{
		(char*) soundBuff[0],
		(char*) soundBuff[1],
		wav_buff_size / 2 * sizeof(wav_sample_t)
	};

	const target_t second_target =
	{
		(char*) (soundBuff[0] + Player::wav_buff_size / 2),
		(char*) (soundBuff[1] + Player::wav_buff_size / 2),
		wav_buff_size / 2 * sizeof(wav_sample_t)
	};

	wav_sample_t emptySample{0, 0};
};



#endif /* SRC_APPLICATION_PLAYER_H_ */
