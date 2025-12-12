#ifndef SRC_APPLICATION_SONG_H_
#define SRC_APPLICATION_SONG_H_

#include "appdefs.h"

#include "ff.h"

class Song
{
public:

	enum error_t
	{
		eOk = 0,
		eFsError,
		eSongFileNotFound,
		eNotRiffWave, // формат wave_a не соответсвует RIFF/WAV
		eWaveNotPresent,  // file_sound ("1_*.wav") отсутсвует
		eNotMidi,
	};

	uint8_t load(const emb_string& songPath);
	void close();

	emb_string songName();

	int8_t songNum{0};	// -1 in library

	bool playNext;

	static constexpr uint8_t maxTrackCount = 4;
	emb_string trackPath[maxTrackCount];
	emb_string trackName[maxTrackCount];
	uint8_t trackCount{0};

	bool trackValid[maxTrackCount];
	uint32_t trackSize[maxTrackCount];
	uint16_t soundDataOffset[maxTrackCount];

	FIL wavFile[maxTrackCount];
	FIL midiFile;

	uint32_t read_chunk_count;

private:
	static char buf[FF_MAX_LFN + 4];

	bool is_valid_wave(FIL *file, uint8_t num);

	char m_songName[16];
	char m_songComment[32];
};


#endif /* SRC_APPLICATION_SONG_H_ */
