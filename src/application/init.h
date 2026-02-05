#ifndef __INIT_H__
#define __INIT_H__

#include "libopencm3/stm32/exti.h"
#include "libopencm3/stm32/timer.h"

#include "menuplayer.h"
#include "player.h"
#include "midi_player.h"

enum
{
	auto_next_track = 0,
	direction_counter,
	direction_scrol_playlist,
	loop_points,
	metronome_out
};

enum MetronomeOut
{
	METRONOME_TO_OUT4 = 0,
	METRONOME_TO_OUT3,
	METRONOME_TO_OUT2,
	METRONOME_TO_OUT1
};

enum
{
	MDUMMY = 0, MCHANNEL,
	MCTRL_START_TYPE, MCTRL_START_VALUE,
	MCTRL_PAUSE_TYPE, MCTRL_PAUSE_VALUE,
	MCTRL_STOP_TYPE, MCTRL_STOP_VALUE,
	MCTRL_NEXT_SONG_TYPE, MCTRL_NEXT_SONG_VALUE,
	MCTRL_PREV_SONG_TYPE, MCTRL_PREV_SONG_VALUE,
	MCTRL_PARAMETER_COUNT
};

extern MenuPlayer* menuPlayer;
extern Player player;
extern MidiPlayer midiPlayer;

extern uint16_t key_reg_out[];

extern uint8_t sys_param[];
extern uint8_t ctrl_param[];
extern uint8_t pc_param[];

extern volatile uint32_t metronom_int;
extern uint32_t tap_temp;
extern uint32_t tap_temp1;
extern uint32_t tap_temp2;

extern uint16_t key_reg_in[2];
extern uint16_t key_reg_out[2];

extern uint8_t us_buf1;
extern uint8_t tim5_fl;

void init(void);
void i2s_dma_interrupt_enable();
void i2s_dma_interrupt_disable();
void tim_start(uint16_t del);

inline void __attribute__ ((always_inline)) dela(uint32_t p)
{
	for (uint32_t i = 0; i < p; i++)
		NOP();
}
#endif /*__INIT_H__*/
