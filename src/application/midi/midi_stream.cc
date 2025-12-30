#include "midi_stream.h"

MidiStream::MidiStream()
{
	reset();
}

MidiStream::~MidiStream()
{
}

void MidiStream::add(const uint64_t &timeTics, size_t size, uint8_t *data)
{
	EventItem eventItem;

	eventItem.time_tics = timeTics;
	eventItem.size = size;
	eventItem.data = new uint8_t[size];

	if (!eventItem.data)
		return;
//            __throw_bad_alloc() ;

	memcpy(eventItem.data, data, size);

	items.push_back(eventItem);
}

void MidiStream::clear()
{
	for(auto &v : items)
	{
		delete[] v.data;
	}
	items.clear();
}

void MidiStream::sortAndMerge()
{

	std::sort(items.begin(), items.end(),
			[](EventItem const &a, EventItem const &b) -> bool
			{
				return a.time_tics < b.time_tics;
			});

	// слияние
	if(items.size() < 2)
		return;

	std::vector<EventItem>::iterator prev = items.begin();
	std::vector<EventItem>::iterator curr = std::next(prev);

	while(curr != items.end())
	{
		if(prev->time_tics == curr->time_tics)
		{

			uint8_t *buf = new uint8_t[prev->size + curr->size];

			memcpy(buf, prev->data, prev->size);
			memcpy(buf + prev->size, curr->data, curr->size);

			delete[] prev->data;
			delete[] curr->data;

			prev->data = buf;
			prev->size = prev->size + curr->size;
			items.erase(curr);

			curr = std::next(prev);
		}
		else
		{
			prev++;
			curr = std::next(prev);
		}
	}
}

void MidiStream::reset()
{
	curr = items.begin();
}
