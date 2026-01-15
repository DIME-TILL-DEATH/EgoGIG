//https://github.com/abique/midi-parser

#include <assert.h>
#include <math.h>

#include "midi_parser.h"

MidiParser::MidiParser()
	: m_state(MIDI_PARSER_INIT)
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
	const uint8_t channel_event_size[8] =
	{ 		3,  //8 - старший полубайт статуса
			3,  //9
			3,  //a
			3,  //b
			2,  //c
			2,  //d
			3 //e
			};

	return channel_event_size[(id >> 4) - 8];
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

midi_parser_state MidiParser::parseHeader()
{
	if(size < 14) return MIDI_PARSER_EOB;

	if(memcmp(in, "MThd", 4)) return MIDI_PARSER_ERROR;

	result.header.size = parse_be32(in + 4);
	result.header.format = parse_be16(in + 8);
	result.header.tracks_count = parse_be16(in + 10);
	result.header.time_division = parse_be16(in + 12);

	in += 14;
	size -= 14;
	m_state = MIDI_PARSER_TRACK;

	return MIDI_PARSER_HEADER;
}

midi_parser_state MidiParser::parseTrack()
{
	if(size < 8) return MIDI_PARSER_EOB;

	result.track.size = parse_be32(in + 4);

	in += 8;
	size -= 8;
	m_state = MIDI_PARSER_TRACK_VTIME;

	return MIDI_PARSER_TRACK;
}

midi_parser_state MidiParser::parseVtime()
{
	uint8_t nbytes = 0;
	uint8_t cont = 1; // continue flag

	result.vtime = 0;
	while(cont)
	{
		++nbytes;

		if (size < nbytes)
			return MIDI_PARSER_EOB;

		uint8_t b = in[nbytes - 1];
		result.vtime = (result.vtime << 7) | (b & 0x7f);

		// The largest value allowed within a MIDI file is 0x0FFFFFFF. A lot of
		// leading bytes with the highest bit set might overflow the nbytes counter
		// and create an endless loop.
		// If one would use 0x80 bytes for padding the check on parser->vtime would
		// not terminate the endless loop. Since the maximum value can be encoded
		// in 5 bytes or less, we can assume bad input if more bytes were used.
		if (result.vtime > 0x0fffffff || nbytes > 5)
			return MIDI_PARSER_ERROR;

		cont = b & 0x80;
	}

	in += nbytes;
	size -= nbytes;
	m_state = MIDI_PARSER_TRACK_EVENT;

	return MIDI_PARSER_TRACK_VTIME;
}

midi_parser_state MidiParser::parseChannelEvent()
{
	if(size < 3) return MIDI_PARSER_EOB;

	memcpy(buff, in, 3);
	buff_size = 3;

	m_state = MIDI_PARSER_TRACK_VTIME;

	size_t param_index = 0;
	result.midi.length = 0;
	static size_t event_size = 0;
	static uint8_t id = 0;

	// в случае если полубайт не соответствует таблице channal_event_size
	// то это значит что после метки времени пеердается тотже статус что и в предыдущей команде а сам
	// статус опущен и стразу идут данные, поэтому здесь на такой случай запоминается текущий статус

	if(isIdValid(in[0]))
	{
		id = result.midi.data[param_index++] = in[0];
		result.midi.length += 1;

		event_size = channelEventSize(in[0]);

		// статус передается
		in += 1;
		size -= 1;
	}
	else
	{
		if (in[0] & 0x80)
		{
			result.midi.data[param_index++] = id;
			result.midi.length += 1;
			event_size = channelEventSize(id);
		}

	}

	if (event_size == 3)
	{

		result.midi.data[param_index++] = in[0];
		result.midi.data[param_index++] = in[1];
		result.midi.length += 2;

		in += 2;
		size -= 2;

		return MIDI_PARSER_TRACK_EVENT;
	}

	if (event_size == 2)
	{
		result.midi.data[param_index++] = in[0];
		result.midi.length += 1;

		in += 1;
		size -= 1;
	}

	return MIDI_PARSER_TRACK_EVENT;
}

midi_parser_state MidiParser::parseMetaEvent()
{
	if(size < 2) return MIDI_PARSER_EOB;

	result.meta.type = in[1];
	int32_t offset = 2;
	result.meta.length = parseVariableLength(&offset);

	// length should never be negative or more than the remaining size
	if(result.meta.length < 0) return MIDI_PARSER_ERROR;

	if(result.meta.length > size) return MIDI_PARSER_EOB;

	// check buffer size
	if(size < offset || size - offset < result.meta.length) return MIDI_PARSER_EOB;

	uint8_t resultMetaLength;
	if(result.meta.length > bufferSize) resultMetaLength = bufferSize - 2;
	else resultMetaLength = result.meta.length;

	memcpy(buff, in, offset + resultMetaLength);
	buff_size = offset + resultMetaLength;
	result.meta.bytes = buff;

	offset += result.meta.length;
	in += offset;
	size -= offset;

	m_state = MIDI_PARSER_TRACK_VTIME;

	return MIDI_PARSER_TRACK_META;
}

midi_parser_state MidiParser::parseEvent()
{
	if (in[0] < 0xf0) return parseChannelEvent();

	if (in[0] == 0xff) return parseMetaEvent();

	return MIDI_PARSER_ERROR;
}

midi_parser_state MidiParser::parseData()
{
	if(!in || size < 1) return MIDI_PARSER_EOB;

	switch(m_state)
	{
	case MIDI_PARSER_INIT:
		return parseHeader();

	case MIDI_PARSER_TRACK:
		return parseTrack();

	case MIDI_PARSER_TRACK_VTIME:
		return parseVtime();

	case MIDI_PARSER_TRACK_EVENT:
		return parseEvent();

	default:
		return MIDI_PARSER_ERROR;
	}
}
