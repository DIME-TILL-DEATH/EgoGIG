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
	static FIL midiFile;


	MidiParser* parser{nullptr};

	size_t track = 0;
	uint32_t time;

	uint64_t m_songPos;

	float systemTimeCoef;

	void parseFile();
};

#endif
