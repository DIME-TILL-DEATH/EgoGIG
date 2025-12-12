#include "menueditplaylist.h"

#include "menudeletesong.h"

#include "init.h"
#include "display.h"
#include "fs_stream.h"

MenuEditPlaylist::MenuEditPlaylist(AbstractMenu* parent)
{
	m_parentMenu = parent;
	m_menuType = MENU_EDIT_PLAYLIST;
}

void MenuEditPlaylist::show(TShowMode showMode)
{
	emb_string path_old = "/SONGS";
	FsStreamTask->enter_dir(path_old.c_str(), "", true);
	m_num_prog_edit = 0;
	load_led(m_num_prog_edit);

	DisplayTask->Clear();
	DisplayTask->StringOut(4, 0, (uint8_t*) "Browser");
	DisplayTask->StringOut(1, 1, (uint8_t*) "Edit Playlist");
	taskDelay(1000);
	DisplayTask->Clear();
	DisplayTask->StringOut(0, 0, (uint8_t*) "1:");

	if (!FsStreamTask->open_song_name(m_num_prog_edit, m_trackName, 0, 0))
	{
		oem2winstar(m_trackName);
		runningNamePos = 0;
		printRunningName(m_trackName, 2, AbstractMenu::STRING_DOUBLE);
	}
	else
		DisplayTask->StringOut(2, 0, (uint8_t*) "  No wav file");
}

void MenuEditPlaylist::refresh()
{
	DisplayTask->Clear();
	runningNamePos = 0;

	switch(m_state)
	{
	case EDIT_TRACK1:
	{
		DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
		break;
	}

	case EDIT_TRACK2:
	{
		DisplayTask->StringOut(0, 0, (uint8_t*) "2:");
		break;
	}
	}
	oem2winstar(m_trackName);
	printRunningName(m_trackName, 2, AbstractMenu::STRING_DOUBLE);
	printPlayNextMark();
	tim7_start(0);
}

void MenuEditPlaylist::task()
{
	if(tim5_fl)
	{
		printRunningName(m_trackName, 2, AbstractMenu::STRING_DOUBLE);
		printPlayNextMark();
	}
}

void MenuEditPlaylist::encoderPress()
{
	emb_string tmp;
	emb_string tmp1;
	FsStreamTask->browser_name(tmp1);
	FsStreamTask->curr_path(tmp);
	if (!(!tmp.compare("/SONGS") && !tmp1.compare("..")))
	{
		FsStreamTask->action_notify(
				m_state ?
						TFsStreamTask::action_param_t::ap_2_wav :
						TFsStreamTask::action_param_t::ap_1_wav,
				m_num_prog_edit, play_next_file);

		FsStreamTask->browser_name(m_trackName);
		DisplayTask->Clear_str(2, 0, 14);
		DisplayTask->Clear_str(0, 1, 16);
		oem2winstar(m_trackName);
		printRunningName(m_trackName, 2, AbstractMenu::STRING_DOUBLE);

		printPlayNextMark();
	}
}

void MenuEditPlaylist::encoderLongPress()
{
	play_next_file = !play_next_file;
	printPlayNextMark();
	encoderPress();
}

void MenuEditPlaylist::encoderClockwise()
{
	FsStreamTask->next_notify();
	FsStreamTask->browser_name(m_trackName);

	refresh();
}

void MenuEditPlaylist::encoderCounterClockwise()
{
	FsStreamTask->prev_notify();
	emb_string tmp;
	FsStreamTask->browser_name(m_trackName);
	FsStreamTask->curr_path(tmp);

	if (!tmp.compare("/SONGS") && !m_trackName.compare(".."))
	{
		FsStreamTask->next_notify();
	}

	FsStreamTask->browser_name(m_trackName);

	refresh();
}

void MenuEditPlaylist::keyStop()
{
	showChild(new MenuDeleteSong(this, m_num_prog_edit));
}

void MenuEditPlaylist::keyStart()
{

}

void MenuEditPlaylist::keyLeftUp()
{
	m_num_prog_edit = (m_num_prog_edit + 10) % 99;
	loadSong();
}

void MenuEditPlaylist::keyLeftDown()
{
	if(m_num_prog_edit > 10) m_num_prog_edit -= 10;
	loadSong();
}

void MenuEditPlaylist::keyRightUp()
{
	if(m_num_prog_edit) m_num_prog_edit--;
	loadSong();
}

void MenuEditPlaylist::keyRightDown()
{
	m_num_prog_edit = (m_num_prog_edit + 1) % 99;
	loadSong();
}

void MenuEditPlaylist::keyReturn()
{
	m_state = EDIT_TRACK1;

	DisplayTask->Clear();
	DisplayTask->StringOut(0, 0, (uint8_t*) "1:");

	if(!FsStreamTask->open_song_name(m_num_prog_edit, m_trackName, 0, 0))
	{
		oem2winstar(m_trackName);
		runningNamePos = 0;
		printRunningName(m_trackName, 2, AbstractMenu::STRING_DOUBLE);
	}
	else
	{
		DisplayTask->Clear_str(0, 1, 16);
		m_trackName = " No wav file";
		DisplayTask->StringOut(2, 0, (uint8_t*)m_trackName.c_str());
	}

	printPlayNextMark();
}

void MenuEditPlaylist::keyForward()
{
	m_state = EDIT_TRACK2;

	DisplayTask->Clear();
	DisplayTask->StringOut(0, 0, (uint8_t*) "2:");

	if (!FsStreamTask->open_song_name(m_num_prog_edit, m_trackName, 0, 1))
	{
		oem2winstar(m_trackName);
		runningNamePos = 0;
		printRunningName(m_trackName, 2, AbstractMenu::STRING_DOUBLE);
	}
	else
	{
		DisplayTask->Clear_str(0, 1, 16);
		m_trackName = " No wav file";
		DisplayTask->StringOut(2, 0, (uint8_t*)m_trackName.c_str());
	}

	printPlayNextMark();
}

void MenuEditPlaylist::keyEsc()
{
	m_parentMenu->returnFromChildMenu();
}

void MenuEditPlaylist::loadSong()
{
	runningNamePos = 0;
	DisplayTask->Clear();

	load_led(m_num_prog_edit);

	DisplayTask->Clear_str(2, 0, 30);
	DisplayTask->StringOut(0, 0, (uint8_t*) "1:");
	if (!FsStreamTask->open_song_name(m_num_prog_edit, m_trackName, 0, 0))
	{
		oem2winstar(m_trackName);
		printRunningName(m_trackName, 2, AbstractMenu::STRING_DOUBLE);
	}
	else
	{
		DisplayTask->Clear_str(0, 1, 16);
		m_trackName = " No wav file";
		DisplayTask->StringOut(2, 0, (uint8_t*)m_trackName.c_str());
	}
	play_next_file = FsStreamTask->file_flag();

//	refresh();
	tim7_start(0);
}

void MenuEditPlaylist::printPlayNextMark()
{
	if (play_next_file == true)
	{
		if(play_next_file)
			DisplayTask->SymbolOut(15, 1, SYMBOL_NEXT_MARK);
		else
			DisplayTask->Clear_str(15, 1, 1);
	}
	else
		DisplayTask->Clear_str(15, 1, 1);
}
