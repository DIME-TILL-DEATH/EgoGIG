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
		parseFile();
		midi_stream.sortAndMerge();
	}
}

void MidiPlayer::parseFile()
{
	m_midiTracks.clear();

	MidiParser parser;

	size_t readed;
	f_read(&m_midiFile, m_parserBuffer, 512, &readed);
	parser.size = readed;
	parser.in = m_parserBuffer;

	size_t trackNum = 0;
	uint32_t time;
	uint32_t dataOffset = 0;
	midi_parser_state status;


	while (1)
	{
		status = parser.parseData();
		switch(status)
		{
		case MIDI_PARSER_EOB:
		{
			memmove(m_parserBuffer, parser.in, parser.size);
			f_read(&m_midiFile, m_parserBuffer, 512, &readed);

			if(readed == 0) // error, incorrect file ending
			{
				midi_stream.clear();
				return;
			}
			parser.size = readed;
			parser.in = m_parserBuffer;
			break;
		}
		case MIDI_PARSER_ERROR:
			midi_stream.clear();
			return;

		case MIDI_PARSER_INIT:
			break;

		case MIDI_PARSER_HEADER:
		{
			m_currentHeader = parser.result.header;

			float sync_freq = ((uint64_t) m_currentHeader.time_division * 1000000);
			m_systemTimeCoef = 44100 / sync_freq;

			dataOffset += 14;
			break;
		}

		case MIDI_PARSER_TRACK:
		{
			MidiTrack track;

			dataOffset += 8;

			track.num = trackNum;
			track.startPosition = dataOffset;
			track.currentPosition = track.startPosition;
			track.size = parser.result.track.size;
			m_midiTracks.push_back(track);

			dataOffset += track.size;

			trackNum++;
			time = 0;
			break;
		}

		case MIDI_PARSER_TRACK_VTIME:
		{
			time += parser.result.vtime;
			break;
		}

		case MIDI_PARSER_TRACK_EVENT:
		{
			midi_stream.add(time * m_systemTimeCoef, parser.result.midi.length, parser.result.midi.data);
			break;
		}

		case MIDI_PARSER_TRACK_META:
		{
			switch(parser.result.meta.type)
			{
				case MIDI_META_SET_TEMPO:
				{
					union
					{
						uint32_t val;
						uint8_t bytes[4];
					} temp;

					temp.bytes[3] = 0;
					temp.bytes[2] = parser.result.meta.bytes[3];
					temp.bytes[1] = parser.result.meta.bytes[4];
					temp.bytes[0] = parser.result.meta.bytes[5];

					uint32_t music_temp = temp.val;

					float sync_freq = ((uint64_t) m_currentHeader.time_division * 1000000) / music_temp;
					m_systemTimeCoef = 44100 / sync_freq;
					break;
				}

				case MIDI_META_END_OF_TRACK:
				{
					if(trackNum != m_currentHeader.tracks_count)
					{
						parser.setState(MIDI_PARSER_TRACK);
					}
					else return;
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

void MidiPlayer::readEvents(const uint64_t& start, const uint64_t& stop)
{
	while(DMA_SCR(DMA2, DMA_STREAM7) & DMA_SxCR_EN) {}

	for(auto track_it = m_midiTracks.begin(); track_it != m_midiTracks.end(); ++track_it)
	{

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

void MidiPlayer::process(const uint64_t& songPos)
{
	m_songPos = songPos;
	if(midi_stream.items.size() != 0)
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
	std::list<MidiStream::EventItem>::iterator currentEvent = midi_stream.items.begin();
	if(currentEvent->played) return;

	if(currentEvent->time_tics < m_songPos)
	{
		if(!(DMA_SCR(DMA2, DMA_STREAM7) & DMA_SxCR_EN))
		{
			DMA_HIFCR (DMA2) = 0xffffffff;

			currentEvent->played = 1;
			dma_set_number_of_data(DMA2, DMA_STREAM7, currentEvent->size);
			dma_set_memory_address(DMA2, DMA_STREAM7, (uint32_t) currentEvent->data);
			dma_enable_stream(DMA2, DMA_STREAM7);
		}
	}
}

extern "C" void DMA2_Stream7_IRQHandler()
{
	dma_clear_interrupt_flags(DMA2, DMA_STREAM7, DMA_TCIF);

	if(midiPlayer.midi_stream.items.size() != 0)
	{
		std::list<MidiStream::EventItem>::const_iterator currentEvent = midiPlayer.midi_stream.items.begin();
		delete[] currentEvent->data;
		midiPlayer.midi_stream.items.pop_front();
	}
}
