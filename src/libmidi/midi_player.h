/*
 * midi_player.h
 *
 *  Created on: 16 февр. 2020 г.
 *      Author: klen
 */
#include "arch.h"

#include "midi_stream.h"

#include "libopencm3/stm32/dma.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/usart.h"

struct midi_player_t
{

	midi_player_t()
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

	void pos(size_t val)
	{
		sample_count = val;

		midi_stream.curr = midi_stream.items.begin();

		while (midi_stream.curr->time_tics < sample_count
				&& midi_stream.curr != midi_stream.items.end())
		{
			midi_stream.curr++;
		}
	}

	void process()
	{

		if (midi_stream.curr != midi_stream.items.end())
		{
			while (midi_stream.curr->time_tics < sample_count)
			{
				// настройка dma

				if (!(DMA_SCR(DMA2, DMA_STREAM7) & DMA_SxCR_EN))
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

		sample_count++;

	}

	void reset()
	{
		midi_stream.reset();
		sample_count = 0;
	}

	midi_stream_t midi_stream;
	uint64_t sample_count;

};

