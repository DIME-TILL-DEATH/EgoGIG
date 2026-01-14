#ifndef __MIDI_PLAYER_H__
#define __MIDI_PLAYER_H__


#include "ff.h"

#include "midi_stream.h"
#include "midi_parser.h"

class MidiPlayer
{
public:
	MidiPlayer();

	void pos(size_t val);
	void process(const uint64_t& songPos);
	void processEvents();

	void openMidiFile(const char* fileName);

	MidiStream midi_stream;

private:
	static FIL m_midiFile;

	uint64_t m_songPos;
//	std::vector<MidiTrack> m_midiTracks;
	midi_header m_currentHeader;

	float m_systemTimeCoef;

	uint8_t m_parserBuffer[1024];

	void parseFile();
};

#endif
