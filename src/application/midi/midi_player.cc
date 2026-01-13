#include "midi_player.h"
#include "sdk.h"
#include "arch.h"
#include "libopencm3/stm32/dma.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/usart.h"


FIL MidiPlayer::midiFile;

MidiPlayer::MidiPlayer()
{
	rcc_periph_clock_enable(RCC_DMA2);
	dma_stream_reset(DMA2, DMA_STREAM7);
	dma_set_priority(DMA2, DMA_STREAM7, DMA_SxCR_PL_MEDIUM);
	dma_set_memory_size(DMA2, DMA_STREAM7, DMA_SxCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA2, DMA_STREAM7, DMA_SxCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(DMA2, DMA_STREAM7);
	dma_disable_peripheral_increment_mode(DMA2, DMA_STREAM7);
	dma_set_transfer_mode(DMA2, DMA_STREAM7,
			DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	dma_set_peripheral_address(DMA2, DMA_STREAM7, (uint32_t) &USART1_DR);
	dma_set_memory_address(DMA2, DMA_STREAM7, (uint32_t) 0);
	//dma_enable_circular_mode(DMA2, DMA_STREAM7);
	dma_set_number_of_data(DMA2, DMA_STREAM7, 0);
	dma_channel_select(DMA2, DMA_STREAM7, DMA_SxCR_CHSEL_4);
}

void MidiPlayer::openMidiFile(const char* fileName)
{
	midi_stream.clear();
	f_close(&midiFile);

	if(f_open(&midiFile, fileName, FA_READ) == FR_OK)
	{
		if(f_size(&midiFile) < 4096)
		{
			parseFile();
			f_close(&midiFile);

			midi_stream.sortAndMerge();
			reset();
		}
		else
			f_close(&midiFile);
	}
}

void MidiPlayer::parseFile()
{
	size_t file_size = f_size(&midiFile);

	uint8_t *mem = new uint8_t[file_size];

	size_t readed;
	f_read(&midiFile, mem, file_size, &readed);

	if(parser) delete parser;
	parser = new MidiParser;

	parser->size = file_size;
	parser->in = mem;

	midi_parser_status status;

	while (1)
	{
		status = parser->parseData();
		switch(status)
		{
		case MIDI_PARSER_EOB:
			delete mem;
			return;

		case MIDI_PARSER_ERROR:
			delete mem;
			return;

		case MIDI_PARSER_INIT:

			break;

		case MIDI_PARSER_HEADER:
		{
			float sync_freq = ((uint64_t) parser->header.time_division * 1000000) / parser->music_temp;
			systemTimeCoef = 44100 / sync_freq;
			break;
		}
		case MIDI_PARSER_TRACK:
		{
			track++;
			time = 0;

			break;
		}
		case MIDI_PARSER_TRACK_MIDI:
		{
			time += parser->vtime;

			midi_stream.add(time * systemTimeCoef, parser->midi.size, parser->midi.bytes);

			break;
		}
		case MIDI_PARSER_TRACK_META:
		{
			time += parser->vtime;

			// метасобытия не добвляются в список, изза не надобности в выводе

			if ((*((uint32_t*) &parser->buff[0]) & 0xffffff) == 0x351ff)
			{
				union
				{
					uint32_t val;
					uint8_t bytes[4];
				} temp;

				temp.bytes[3] = 0;
				temp.bytes[2] = parser->buff[3];
				temp.bytes[1] = parser->buff[4];
				temp.bytes[0] = parser->buff[5];

				parser->music_temp = temp.val;

				float sync_freq = ((uint64_t) parser->header.time_division * 1000000) / parser->music_temp;
				systemTimeCoef = 44100 / sync_freq;

			}

			break;
		}
		case MIDI_PARSER_TRACK_SYSEX:
			time += parser->vtime;

			break;

		default:
			delete mem;
			return;
		}
	}


}

void MidiPlayer::pos(size_t val)
{
	midi_stream.curr = midi_stream.items.begin();

	while (midi_stream.curr->time_tics < val
			&& midi_stream.curr != midi_stream.items.end())
	{
		midi_stream.curr++;
	}
}

void MidiPlayer::process(const uint64_t& sample_pos)
{
	if(midi_stream.curr != midi_stream.items.end())
	{
		while(midi_stream.curr->time_tics < sample_pos)
		{
			if(!(DMA_SCR(DMA2, DMA_STREAM7) & DMA_SxCR_EN))
			{
				DMA_HIFCR (DMA2) = 0xffffffff;

				dma_set_number_of_data(DMA2, DMA_STREAM7, midi_stream.curr->size);
				dma_set_memory_address(DMA2, DMA_STREAM7, (uint32_t) midi_stream.curr->data);
				dma_enable_stream(DMA2, DMA_STREAM7);
			}

			midi_stream.curr++;
			if (midi_stream.curr == midi_stream.items.end())
				break;
		}
	}
}

void MidiPlayer::reset()
{
	midi_stream.reset();
}
