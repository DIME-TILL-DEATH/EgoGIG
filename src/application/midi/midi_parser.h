/*
 * Simple MIDI parser implementation.
 * I used the following reference:
 * http://www.sonicspot.com/guide/midifiles.html
 */

#ifndef __MIDI_PARSER_H__
#define __MIDI_PARSER_H__

#include "arch.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum midi_parser_status
{
	MIDI_PARSER_EOB = -2,
	MIDI_PARSER_ERROR = -1,
	MIDI_PARSER_INIT = 0,
	MIDI_PARSER_HEADER = 1,
	MIDI_PARSER_TRACK = 2,
	MIDI_PARSER_TRACK_VTIME = 3,
	MIDI_PARSER_TRACK_EVENT = 4,
	MIDI_PARSER_TRACK_META = 5,
	MIDI_PARSER_TRACK_SYSEX = 6,
};

struct midi_header
{
	int32_t size;
	uint16_t format;
	int16_t tracks_count;
	int16_t time_division;
};

struct midi_track
{
	int32_t size;
	uint32_t posInFile;
};

enum midi_status
{
	MIDI_STATUS_NOTE_OFF = 0x8,
	MIDI_STATUS_NOTE_ON = 0x9,
	MIDI_STATUS_NOTE_AT = 0xA, // after touch
	MIDI_STATUS_CC = 0xB, // control change
	MIDI_STATUS_PGM_CHANGE = 0xC,
	MIDI_STATUS_CHANNEL_AT = 0xD, // after touch
	MIDI_STATUS_PITCH_BEND = 0xE,
};

enum midi_meta
{
	MIDI_META_SEQ_NUM = 0x00,
	MIDI_META_TEXT = 0x01,
	MIDI_META_COPYRIGHT = 0x02,
	MIDI_META_TRACK_NAME = 0x03,
	MIDI_META_INSTRUMENT_NAME = 0x04,
	MIDI_META_LYRICS = 0x05,
	MIDI_META_MAKER = 0x06,
	MIDI_META_CUE_POINT = 0x07,
	MIDI_META_CHANNEL_PREFIX = 0x20,
	MIDI_META_END_OF_TRACK = 0x2F,
	MIDI_META_SET_TEMPO = 0x51,
	MIDI_META_SMPTE_OFFSET = 0x54,
	MIDI_META_TIME_SIGNATURE = 0x58,
	MIDI_META_KEY_SIGNATURE = 0x59,
	MIDI_META_SEQ_SPECIFIC = 0x7F,
};

union midi_midi_event
{
	struct
	{
		uint8_t data[128];
		uint8_t size;
	};
	uint8_t bytes[130];
};

struct midi_meta_event
{
	uint8_t type;
	int32_t length;
	const uint8_t *bytes;  // reference to the input buffer
};

struct midi_sysex_event
{
	uint8_t sysex;
	uint8_t type;
	int32_t length;
	const uint8_t *bytes;  // reference to the input buffer
};

class MidiParser
{
public:
	MidiParser();

	midi_parser_status parseData();

	/* input buffer */
	const uint8_t *in;
	int32_t size;
	uint8_t buffer[1024];

	/* result */
	int64_t vtime;
	struct midi_header header;
	struct midi_track track;

	union midi_midi_event midi;
	struct midi_meta_event meta;
	struct midi_sysex_event sysex;

	uint8_t buff[64];
	size_t buff_size;

	uint32_t music_temp;

private:
	midi_parser_status m_state;

	uint16_t parse_be16(const uint8_t *in);
	uint32_t parse_be32(const uint8_t *in);
	uint64_t parseVariableLength(int32_t *offset);

	midi_parser_status parseHeader();
	midi_parser_status parseTrack();
	midi_parser_status parseVtime();
	midi_parser_status parseChannelEvent();
	midi_parser_status parseMetaEvent();
	midi_parser_status parseEvent();

	// проверка статуса на вадтдность
	// (необходимо для выяснения ситуации когда данные идут сразу за меткой времени)
	bool isIdValid(uint8_t id);
	// вычисление размера события по id статуса(команды)
	uint8_t channelEventSize(uint8_t id);
};

#endif /* __MIDI_PARSER_H__ */
