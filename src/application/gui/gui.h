#ifndef SRC_APPLICATION_GUI_GUI_H_
#define SRC_APPLICATION_GUI_GUI_H_

#include "appdefs.h"
#include "libopencm3/stm32/timer.h"


enum
{
	player,
	edit_playlist,
	select_folder,
	play_next_fil,
	name_ind_play,
	delete_file,
	menu,
	sys_menu,
	metronome,
	midi_ctrl_menu,
	midi_pc_menu
};

enum
{
	key_stop = 1,
	key_start,
	key_left_up,
	key_left_down,
	key_right_up,
	key_right_down,
	key_return,
	key_forward,
	key_esc,
	key_encoder
};
enum
{
	auto_next_track = 0,
	direction_counter,
	direction_scrol_playlist,
	loop_points
};
enum
{
	ctrl1_t, ctrl1, ctrl2_t, ctrl2, ctrl3_t, ctrl3, jjj, chann
};

void processGui();

uint8_t test_file(void);

void write_sys(void);
void read_sys(void);
void write_ctrl(void);
void read_ctrl(void);
void write_map(void);
void read_map(void);
uint8_t tap_temp_global(void);
void ind_temp(void);

extern uint8_t blink_en;

extern uint32_t song_size;

#endif /* SRC_APPLICATION_GUI_GUI_H_ */
