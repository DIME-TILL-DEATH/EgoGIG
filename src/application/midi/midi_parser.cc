//https://github.com/abique/midi-parser

#include <assert.h>
#include <math.h>

#include "midi_parser.h"

MidiParser::MidiParser()
	:music_temp(1),
	 m_state(MIDI_PARSER_INIT)
{
}

bool MidiParser::isIdValid(uint8_t id)
{
	/* валидные старше полубайты
	 8
	 9
	 a
	 b
	 c
	 d
	 e
	 */
	uint8_t tmp = id >> 4;
	if ((tmp >= 0x8) && (tmp <= 0xe))
		return true;
	return false;
}

// вычисление размера события по id статуса(команды)
uint8_t MidiParser::channelEventSize(uint8_t id)
{
	const uint8_t chanal_event_size[8] =
	{ 3,  //8 - старший полубайт статуса
			3,  //9
			3,  //a
			3,  //b
			2,  //c
			2,  //d
			3 //e
			};
	return chanal_event_size[(id >> 4) - 8];
}

uint16_t MidiParser::parse_be16(const uint8_t *data)
{
	return (data[0] << 8) | data[1];
}

uint32_t MidiParser::parse_be32(const uint8_t *data)
{
	return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

uint64_t MidiParser::parseVariableLength(int32_t *offset)
{
	uint64_t value = 0;
	int32_t i = *offset;

	for (; i < size; ++i)
	{
		value = (value << 7) | (in[i] & 0x7f);
		if (!(in[i] & 0x80))
			break;
	}
	*offset = i + 1;
	return value;
}

midi_parser_status MidiParser::parseHeader()
{
	if (size < 14)
		return MIDI_PARSER_EOB;

	if (memcmp(in, "MThd", 4))
		return MIDI_PARSER_ERROR;

	header.size = parse_be32(in + 4);
	header.format = parse_be16(in + 8);
	header.tracks_count = parse_be16(in + 10);
	header.time_division = parse_be16(in + 12);

	in += 14;
	size -= 14;
	m_state = MIDI_PARSER_HEADER;

	return MIDI_PARSER_HEADER;
}

midi_parser_status MidiParser::parseTrack()
{
	if(size < 8) return MIDI_PARSER_EOB;

	track.size = parse_be32(in + 4);
	m_state = MIDI_PARSER_TRACK;
	in += 8;
	size -= 8;

	// Save pointer to track start in file, return to header
	return MIDI_PARSER_TRACK;
}

bool MidiParser::parseVtime()
{
	uint8_t nbytes = 0;
	uint8_t cont = 1; // continue flag

	vtime = 0;
	while(cont)
	{
		++nbytes;

		if (size < nbytes || track.size < nbytes)
			return false;

		uint8_t b = in[nbytes - 1];
		vtime = (vtime << 7) | (b & 0x7f);

		// The largest value allowed within a MIDI file is 0x0FFFFFFF. A lot of
		// leading bytes with the highest bit set might overflow the nbytes counter
		// and create an endless loop.
		// If one would use 0x80 bytes for padding the check on parser->vtime would
		// not terminate the endless loop. Since the maximum value can be encoded
		// in 5 bytes or less, we can assume bad input if more bytes were used.
		if (vtime > 0x0fffffff || nbytes > 5)
			return false;

		cont = b & 0x80;
	}

	in += nbytes;
	size -= nbytes;
	track.size -= nbytes;

	return true;
}

midi_parser_status MidiParser::parseChannelEvent()
{
	if(size < 3) return MIDI_PARSER_EOB;

	memcpy(buff, in, 3);
	buff_size = 3;

	size_t param_index = 0;
	midi.size = 0;
	static size_t event_size = 0;
	static uint8_t id = 0;

	// в случае если полубайт не соответствует таблице channal_event_size
	// то это значит что после метки времени пеердается тотже статус что и в предыдущей команде а сам
	// статус опущен и стразу идут данные, поэтому здесь на такой случай запоминается текущий статус

	if(isIdValid(in[0]))
	{
		id = midi.data[param_index++] = in[0];
		midi.size += 1;

		event_size = channelEventSize(in[0]);

		// статус передается
		in += 1;
		size -= 1;
		track.size -= 1;
	}
	else
	{
		if (in[0] & 0x80)
		{
			midi.data[param_index++] = id;
			midi.size += 1;
			event_size = channelEventSize(id);
		}

	}

	if (event_size == 3)
	{

		midi.data[param_index++] = in[0];
		midi.data[param_index++] = in[1];
		midi.size += 2;

		in += 2;
		size -= 2;
		track.size -= 2;

		return MIDI_PARSER_TRACK_MIDI;
	}

	if (event_size == 2)
	{
		midi.data[param_index++] = in[0];
		midi.size += 1;

		in += 1;
		size -= 1;
		track.size -= 1;
	}

	return MIDI_PARSER_TRACK_MIDI;
}

midi_parser_status MidiParser::parseMetaEvent()
{
//	assert(in[0] == 0xff);

	if (size < 2)
		return MIDI_PARSER_ERROR;

	meta.type = in[1];
	int32_t offset = 2;
	meta.length = parseVariableLength(&offset);

	// length should never be negative or more than the remaining size
	if (meta.length < 0 || meta.length > size)
		return MIDI_PARSER_ERROR;

	// check buffer size
	if (size < offset || size - offset < meta.length)
		return MIDI_PARSER_ERROR;

	memcpy(buff, in, offset + meta.length);
	buff_size = offset + meta.length;

	offset += meta.length;
	in += offset;
	size -= offset;
	track.size -= offset;

	return MIDI_PARSER_TRACK_META;
}

midi_parser_status MidiParser::parseEvent()
{
	if (!parseVtime())
		return MIDI_PARSER_EOB;

	// Make sure the parser has not consumed the entire file or track, else
	// `parser-in[0]` might access heap-memory after the allocated buffer.
	if (size <= 0 || track.size <= 0)
		return MIDI_PARSER_ERROR;

	if (in[0] < 0xf0)
		return parseChannelEvent();

	if (in[0] == 0xff)
		return parseMetaEvent();

	return MIDI_PARSER_ERROR;
}

midi_parser_status MidiParser::parseData()
{
	if(!in || size < 1) return MIDI_PARSER_EOB;

	switch(m_state)
	{
	case MIDI_PARSER_INIT:
		return parseHeader();

	case MIDI_PARSER_HEADER:
		return parseTrack();

	case MIDI_PARSER_TRACK:
		if(track.size == 0)
		{
			// we reached the end of the track
			m_state = MIDI_PARSER_HEADER;
			return parseData();
		}
		return parseEvent();

	default:
		return MIDI_PARSER_ERROR;
	}
}
