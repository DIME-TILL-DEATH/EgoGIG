#ifndef SRC_APPLICATION_SONG_H_
#define SRC_APPLICATION_SONG_H_

#include "appdefs.h"

#include "ff.h"
#include "player.h"

class Song
{
public:

	enum FsError
	{
		eOk = 0,
		eFsError,
		eSongFileNotFound,
		eNotRiffWave, // формат wave_a не соответсвует RIFF/WAV
		eWaveNotPresent,  // file_sound ("1_*.wav") отсутсвует
		eNotMidi,
	};

	FsError load(const emb_string& songPath);
	FsError save(const emb_string& songPath);
	void close();

	emb_string songName();

	int8_t songNum{0};	// -1 in library

	bool playNext;


	emb_string trackPath[Player::maxTrackCount];
	emb_string trackName[Player::maxTrackCount];
	uint8_t trackCount{0};

	bool trackValid[Player::maxTrackCount];
	uint32_t trackSize[Player::maxTrackCount];
	uint16_t soundDataOffset[Player::maxTrackCount];

	FIL wavFile[Player::maxTrackCount];
	FIL midiFile;

	uint64_t read_chunk_count{0};

	bool isValidWave(FIL *file, uint8_t num);
	bool isValidWave(emb_string filePath);

	uint32_t songSize() { return m_songSize;}

private:
	static char buf[FF_MAX_LFN + 4];

	emb_string m_songName;
//	char m_songName[16];
//	char m_songComment[32];

	uint32_t m_songSize{0};
};


#endif /* SRC_APPLICATION_SONG_H_ */
