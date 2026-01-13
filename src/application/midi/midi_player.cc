#include "midi_player.h"

#include "sdk.h"
#include "arch.h"
#include "libopencm3/stm32/dma.h"
#include <libopencm3/cm3/nvic.h>
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/usart.h"

#include "init.h"
#include "fs_stream.h"
#include "player.h"

FIL MidiPlayer::m_midiFile;

MidiPlayer::MidiPlayer()
{
	memset(&m_currentHeader, 0, sizeof(midi_header));

	rcc_periph_clock_enable(RCC_DMA2);
	dma_stream_reset(DMA2, DMA_STREAM7);
	dma_set_priority(DMA2, DMA_STREAM7, DMA_SxCR_PL_MEDIUM);
	dma_set_memory_size(DMA2, DMA_STREAM7, DMA_SxCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA2, DMA_STREAM7, DMA_SxCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(DMA2, DMA_STREAM7);
	dma_disable_peripheral_increment_mode(DMA2, DMA_STREAM7);
	dma_set_transfer_mode(DMA2, DMA_STREAM7, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	dma_set_peripheral_address(DMA2, DMA_STREAM7, (uint32_t) &USART1_DR);
	dma_set_memory_address(DMA2, DMA_STREAM7, (uint32_t) 0);
	//dma_enable_circular_mode(DMA2, DMA_STREAM7);
	dma_set_number_of_data(DMA2, DMA_STREAM7, 0);
	dma_channel_select(DMA2, DMA_STREAM7, DMA_SxCR_CHSEL_4);

	nvic_enable_irq (NVIC_DMA2_STREAM7_IRQ);
	nvic_set_priority(NVIC_DMA2_STREAM7_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY + 16);

	dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM7);
}

void MidiPlayer::openMidiFile(const char* fileName)
{
	midi_stream.clear();
	f_close(&m_midiFile);

	if(f_open(&m_midiFile, fileName, FA_READ) == FR_OK)
	{
		if(f_size(&m_midiFile) < 4096)
		{
			parseFile();
			f_close(&m_midiFile);

			midi_stream.sortAndMerge();
		}
		else
			f_close(&m_midiFile);
	}
}

void MidiPlayer::parseFile()
{
	m_midiTracks.clear();

	if(m_parser) delete m_parser;
		m_parser = new MidiParser;

//	size_t file_size = f_size(&m_midiFile);
//
//	uint8_t *mem = new uint8_t[file_size];
		memset(m_parser->buffer, 0, 1024);

	size_t readed;
	f_read(&m_midiFile, m_parser->buffer, 64, &readed);
	m_parser->size = readed;
	m_parser->in = m_parser->buffer;

	size_t track = 0;
	uint32_t time;
	midi_parser_status status;

	while (1)
	{
		status = m_parser->parseData();
		switch(status)
		{
		case MIDI_PARSER_EOB:
		{
			memmove(m_parser->buffer, m_parser->in, m_parser->size);
			f_read(&m_midiFile, m_parser->buffer, 64, &readed);

			if(readed == 0) // error, incorrect file ending
			{
				midi_stream.clear();
				return;
			}
			m_parser->size = readed;
			m_parser->in = m_parser->buffer;
			break;
		}
		case MIDI_PARSER_ERROR:
			midi_stream.clear();
			return;

		case MIDI_PARSER_INIT:
			break;

		case MIDI_PARSER_HEADER:
		{
			m_currentHeader = m_parser->header;

			float sync_freq = ((uint64_t) m_parser->header.time_division * 1000000) / m_parser->music_temp;
			m_systemTimeCoef = 44100 / sync_freq;
			break;
		}

		case MIDI_PARSER_TRACK:
		{
			track++;
			time = 0;
			break;
		}

		case MIDI_PARSER_TRACK_VTIME:
		{
			time += m_parser->vtime;
			break;
		}

		case MIDI_PARSER_TRACK_EVENT:
		{
//			time += m_parser->vtime;
			midi_stream.add(time * m_systemTimeCoef, m_parser->midi.size, m_parser->midi.bytes);
			break;
		}

		case MIDI_PARSER_TRACK_META:
		{
//			time += m_parser->vtime;

			// метасобытия не добвляются в список, изза не надобности в выводе

//			if ((*((uint32_t*) &m_parser->buff[0]) & 0xffffff) == 0x0351ff)
//			{
			switch(m_parser->buff[1])
			{
				case MIDI_META_SET_TEMPO:
				{
					union
					{
						uint32_t val;
						uint8_t bytes[4];
					} temp;

					temp.bytes[3] = 0;
					temp.bytes[2] = m_parser->buff[3];
					temp.bytes[1] = m_parser->buff[4];
					temp.bytes[0] = m_parser->buff[5];

					m_parser->music_temp = temp.val;

					float sync_freq = ((uint64_t) m_parser->header.time_division * 1000000) / m_parser->music_temp;
					m_systemTimeCoef = 44100 / sync_freq;
					break;
				}

				case MIDI_META_END_OF_TRACK:
				{
					if(track == m_currentHeader.tracks_count) return;

					break;
				}
			}
			break;
		}

		default:
			midi_stream.clear();
			return;
		}
	}
}

void MidiPlayer::pos(size_t val)
{
//	midi_stream.curr = midi_stream.items.begin();
//
//	while (midi_stream.curr->time_tics < val
//			&& midi_stream.curr != midi_stream.items.end())
//	{
//		midi_stream.curr++;
//	}
}

volatile uint8_t dma_busy;
void MidiPlayer::process(const uint64_t& songPos)
{
	m_songPos = songPos;
	if(midi_stream.items.size() != 0 && !dma_busy)
	{
		processEvents();
	}

	uint16_t bufferTimeInterval = Player::wav_buff_size * 4;
	if(m_songPos % bufferTimeInterval == 0)
	{
		FsStreamTask->midi_notify(m_songPos + bufferTimeInterval, m_songPos + bufferTimeInterval * 2);
	}
}

void MidiPlayer::processEvents()
{
	std::vector<MidiStream::EventItem>::const_iterator currentEvent = midi_stream.items.begin();
	if(currentEvent->time_tics < m_songPos)
	{
		if(!(DMA_SCR(DMA2, DMA_STREAM7) & DMA_SxCR_EN))
		{
			DMA_HIFCR (DMA2) = 0xffffffff;

			dma_set_number_of_data(DMA2, DMA_STREAM7, currentEvent->size);
			dma_set_memory_address(DMA2, DMA_STREAM7, (uint32_t) currentEvent->data);
			dma_enable_stream(DMA2, DMA_STREAM7);

			dma_busy = 1;
		}
	}
}

extern "C" void DMA2_Stream7_IRQHandler()
{
	dma_clear_interrupt_flags(DMA2, DMA_STREAM7, DMA_TCIF);

	dma_busy = 0;
	if(midiPlayer.midi_stream.items.size() != 0)
	{
		std::vector<MidiStream::EventItem>::const_iterator currentEvent = midiPlayer.midi_stream.items.begin();
		delete[] currentEvent->data;
		midiPlayer.midi_stream.items.erase(currentEvent);
		midiPlayer.processEvents();
	}
}
