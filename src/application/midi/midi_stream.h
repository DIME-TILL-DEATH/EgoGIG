#ifndef __MIDI_STREAM_H__
#define __MIDI_STREAM_H__

#include <list>
#include <functional>

#include "appdefs.h"

class MidiStream
{
public:
	struct EventItem
	{
		uint64_t time_tics;
		size_t size;
		uint8_t *data;
	};

	MidiStream();
	~MidiStream();

	std::vector<EventItem>::const_iterator curr;
	std::vector<EventItem> items;

	void add(const uint64_t &time_tics, size_t size, uint8_t *data);
	void clear();
	void sortAndMerge();
	void reset();

private:

};

#endif /* __MIDI_STREAM_H__ */
