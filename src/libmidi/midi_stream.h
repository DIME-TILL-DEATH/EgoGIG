/*
 * midi_stream.h
 *
 *  Created on: 13 февр. 2020 г.
 *      Author: klen
 */

#ifndef __MIDI_STREAM_H__
#define __MIDI_STREAM_H__

#include <list>
#include<functional>

struct midi_stream_t
{

	struct event_item_t
	{
		uint64_t time_tics;
		size_t size;
		uint8_t *data;
	};

	std::list<event_item_t> items;
	std::list<event_item_t>::iterator curr;

	inline midi_stream_t()
	{
		reset();
	}
	inline ~midi_stream_t()
	{
	}

	void add(const size_t time_tics, size_t size, uint8_t *data)
	{
		event_item_t event_item;

		event_item.time_tics = time_tics;
		event_item.size = size;
		event_item.data = new uint8_t[size];

		if (!event_item.data)
			return;
//            __throw_bad_alloc() ;

		memcpy(event_item.data, data, size);

		items.push_back(event_item);
	}

	inline void clear()
	{
		for (auto &v : items)
		{
			delete[] v.data;
		}
		items.clear();
	}

	void sort_and_merge()
	{
		// сортировка
		items.sort([](event_item_t const &a, event_item_t const &b) -> bool 
		{	return a.time_tics < b.time_tics;});

		// слияние
		if (items.size() < 2)
			return;

		std::list<event_item_t>::iterator prev = items.begin();
		std::list<event_item_t>::iterator curr = std::next(prev);
		;

		while (curr != items.end())
		{
			if (prev->time_tics == curr->time_tics)
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

	inline void reset()
	{
		curr = items.begin();
	}

};

#endif /* __MIDI_STREAM_H__ */
