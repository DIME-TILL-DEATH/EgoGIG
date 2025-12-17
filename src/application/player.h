#ifndef SRC_APPLICATION_PLAYER_H_
#define SRC_APPLICATION_PLAYER_H_

#include "appdefs.h"

class Player
{
public:
	enum State
	{
		PLAYER_IDLE,
		PLAYER_LOADING_SONG, 	// play_fl1
		PLAYER_PLAYING,	// play_fl, play_fl2
		PLAYER_PAUSE
	};

	State state() { return m_state; }

	void startPlay();
	void stopPlay();
	void pause();

	void initSong();
	void songInitiated();

private:
	State m_state{PLAYER_IDLE};

};



#endif /* SRC_APPLICATION_PLAYER_H_ */
