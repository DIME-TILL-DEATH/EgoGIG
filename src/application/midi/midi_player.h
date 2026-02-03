#ifndef __MIDI_PLAYER_H__
#define __MIDI_PLAYER_H__

#include "player.h"
#include "ff.h"

#include "midi_stream.h"
#include "midi_parser.h"

struct MidiTrack
{
	uint8_t num;
	uint32_t size;
	uint32_t startPosition;
	uint32_t currentPosition;
	uint64_t lastEventTime;
};

class MidiPlayer
{
public:
	MidiPlayer();

	void jumpToPos(size_t val);
	void process(const uint64_t& songPos);
	void processEvents();

	void startPlay();

	void loadSong(const emb_string& songPath);

	void readEvents(const uint64_t& start, const uint64_t& stop);

	bool midiFileValid() { return m_midiFileValid; }
	MidiStream midi_stream;

private:
	static FIL m_midiFile;

	bool m_midiFileValid;
	uint64_t m_songPos;
	std::vector<MidiTrack> m_midiTracks;
	midi_header m_currentHeader;

	float m_systemTimeCoef;

	uint8_t m_parserBuffer[1024];

	static constexpr uint16_t bufferTimeInterval = Player::wav_buff_size * 4;

	void openMidiFile(const char* fileName);
	void parseFile();
};

#endif
