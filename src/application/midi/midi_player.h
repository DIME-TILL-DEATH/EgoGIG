#include "midi_stream.h"
#include "ff.h"

#include "midi_parser.h"

class MidiPlayer
{
public:
	MidiPlayer();

	void pos(size_t val);
	void process(const uint64_t& sample_pos);
	void reset();

	void openMidiFile(const char* fileName);

private:
	MidiStream midi_stream;
	static FIL midiFile;

	size_t track = 0;
	uint32_t time;

	float systemTimeCoef;

	MidiParser* parser{nullptr};

	void parseFile();
};

