// https://github.com/abique/midi-parser

/*
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
*/

#include "sdk.h"


#include "midi_stream.h"
#include "midi-parser.h"

#include "ff.h"


size_t track = 0 ;
uint32_t time ;
std::string msg ;

static void parse_and_dump(struct midi_parser *parser, midi_stream_t& midi_stream)
{
  midi_parser_status status;

  while (1) {
    status = midi_parse(parser);
    switch (status) {
    case MIDI_PARSER_EOB:

      return;

    case MIDI_PARSER_ERROR:
      return;

    case MIDI_PARSER_INIT:

      break;

    case MIDI_PARSER_HEADER:
      {
        parser->sync_freq = ((uint64_t)parser->header.time_division * 1000000) / parser->music_temp ;
      }
      break;

    case MIDI_PARSER_TRACK:
      track++ ;
      time = 0 ;

      break;


    case MIDI_PARSER_TRACK_MIDI:
      time += parser->vtime ;

      midi_stream.add(time, parser->midi.size,  parser->midi.bytes) ;

      break;

    case MIDI_PARSER_TRACK_META:
      time += parser->vtime ;

      // метасобытия не добвляются в список, изза не надобности в выводе
      //midi_stream.add(time, parser->buff_size,  parser->buff) ;

     //
     if ( (*((uint32_t*)&parser->buff[0]) & 0xffffff) == 0x351ff)
	{
	  union
	  {
	    uint32_t val ;
	    uint8_t  bytes[4] ;
	  } temp ;

	  temp.bytes[3] = 0 ;
	  temp.bytes[2] = parser->buff[3] ;
	  temp.bytes[1] = parser->buff[4] ;
	  temp.bytes[0] = parser->buff[5] ;

	  parser->music_temp = temp.val ;
	  parser->sync_freq = ((uint64_t)parser->header.time_division * 1000000) / parser->music_temp ;

	}


      break;

    case MIDI_PARSER_TRACK_SYSEX:
      time += parser->vtime ;

      break;

    default:

      return;
    }
  }
}


static FIL fil ;

static size_t stream_read(uint8_t* buf, size_t size)
{
  size_t readed ;
  f_read( &fil, buf, size, &readed ) ;
  return readed ;
}




float parse_file(const char *path, midi_stream_t& midi_stream)
{

  f_open(&fil,path,FA_READ) ;
  
  size_t file_size = f_size(&fil) ;

  uint8_t* mem = new uint8_t [file_size] ;

  size_t readed ;
  f_read( &fil, mem, file_size, &readed ) ;

  struct midi_parser parser;
  parser.state = MIDI_PARSER_INIT;
  parser.size  = file_size;
  parser.in    = mem;
  parser.stream_read = stream_read ;

  parse_and_dump(&parser, midi_stream);

  delete mem ;

  f_close(&fil);

  return parser.sync_freq ;
}
