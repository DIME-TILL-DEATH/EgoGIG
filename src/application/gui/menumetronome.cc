#include "menumetronome.h"

#include "init.h"
#include "display.h"
#include "enc.h"
#include "fs_stream.h"
#include "leds.h"

MenuMetronome::MenuMetronome(AbstractMenu* parent)
{
	m_menuType = MENU_METRONOME;
	m_parentMenu = parent;
}

MenuMetronome::~MenuMetronome()
{

}


void MenuMetronome::show(TShowMode showMode)
{
	DisplayTask->Clear();
	DisplayTask->StringOut(0, 0, (uint8_t*) "    Metronome");
	DisplayTask->StringOut(0, 1, (uint8_t*) "    Tempo 120");
	tempo = 120;
}

void MenuMetronome::refresh()
{

}

void MenuMetronome::task()
{

}

void MenuMetronome::encoderPress()
{
	if(!metronom_start)
	{
		emb_string tmp = FsStreamTask->selectedSong.songName();
//		FsStreamTask->sound_name(tmp);
		oem2winstar(tmp);

		m_parentMenu->returnFromChildMenu();
	}
}

void MenuMetronome::encoderClockwise()
{
	if(tempo < 240)
		tempo = enc_speed_inc(tempo, 240);

	metronom_int = 44100.0f / (tempo / 60.0f) + 0.5f;
	ind_temp();
}

void MenuMetronome::encoderCounterClockwise()
{
	if(tempo > 20)
		tempo = enc_speed_dec(tempo, 20);

	metronom_int = 44100.0f / (tempo / 60.0f) + 0.5f;
	ind_temp();
}

void MenuMetronome::keyStop()
{
	Leds::redOn();
	Leds::greenOff();

	metronom_start = 0;
}

void MenuMetronome::keyStart()
{
	metronom_int = 44100.0f / (tempo / 60.0f) + 0.5f;
	metronom_counter = temp_counter = 0;
	metronom_start = 1;

	Leds::redOff();
	Leds::greenOn();
}

void MenuMetronome::keyForward()
{
	if (tap_temp_global())
	{
		if ((tap_global < 132300) && (tap_global > 11025))
		{
			metronom_int = tap_global;
			tempo = 44100.0f / tap_global * 60.0f;
			ind_temp();
		}
	}
}

uint8_t MenuMetronome::tap_temp_global(void)
{
	uint8_t a = 0;
	if (tap_temp2 < 170000)
	{
		tap_temp2 = 0;
		if (tap_temp1 < 150000)
		{
			if (tap_temp < 132300)
				tap_global = tap_temp;
		}
		else
			tap_global = 132300;
		a = 1;
	}
	tap_temp = 0;
	tap_temp1 = 0;
	tap_temp2 = 0;
	return a;
}

void MenuMetronome::ind_temp(void)
{
	uint8_t str[4] =
	{ 0, 0, 0, 0 };
	if (tempo < 100)
		str[0] = 32;
	else
		str[0] = tempo / 100 + 48;
	str[2] = (tempo % 100);
	str[1] = str[2] / 10 + 48;
	str[2] = str[2] % 10 + 48;
	DisplayTask->StringOut(10, 1, (uint8_t*) str);
}
