#include "menueditplaylist.h"

#include "menudeletesong.h"

#include "init.h"
#include "display.h"
#include "leds.h"
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
	m_numProgEdit = 0;
	Leds::digit(m_numProgEdit);

	DisplayTask->Clear();
	DisplayTask->StringOut(4, 0, (uint8_t*) "Browser");
	DisplayTask->StringOut(1, 1, (uint8_t*) "Edit Playlist");
	taskDelay(1000);
	DisplayTask->Clear();

	loadSong();
}

void MenuEditPlaylist::refresh()
{
	runningNamePos = 0;
	DisplayTask->Clear_str(7, 0, 8);

//	emb_string currentPath;
//	FsStreamTask->get_browser_name(currentPath);

	if(m_state == EDITING)
	{
		if(FsStreamTask->currentPathIsDirectory())
		{
			DisplayTask->StringOut(7, 0, (uint8_t*)"(dir.)");
		}
		else
		{
			if(!FsStreamTask->editingSong.isValidWave(m_chosenTrackPath))
				DisplayTask->StringOut(7, 0, (uint8_t*)"(err.)");
			else if(MidiPlayer::checkMidiAvaliable(m_chosenTrackPath))
				DisplayTask->StringOut(7, 0, (uint8_t*)"(+midi)");

		}
	}

	DisplayTask->Clear_str(0, 1, 16);
	printRunningName(m_chosenTrackName, 0, 1);
}

void MenuEditPlaylist::task()
{
	if(tim5_fl)
	{
		printRunningName(m_chosenTrackName, 0, 1);
		printPlayNextMark();
	}
}

void MenuEditPlaylist::encoderPress()
{
	if(FsStreamTask->currentPathIsDirectory())
	{
		FsStreamTask->action_notify(TFsStreamTask::action_param_t::enter_directory, m_numProgEdit);
		FsStreamTask->get_browser_name(m_chosenTrackName);
		FsStreamTask->curr_path(m_chosenTrackPath);
		m_chosenTrackPath += "/" + m_chosenTrackName;
		refresh();
	}
	else
	{
		if(FsStreamTask->editingSong.isValidWave(m_chosenTrackPath))
		{
			FsStreamTask->get_browser_name(m_chosenTrackName);
			FsStreamTask->curr_path(m_chosenTrackPath);
			m_chosenTrackPath += "/" + m_chosenTrackName;

			FsStreamTask->editingSong.trackPath[m_editingTrack] = m_chosenTrackPath;
			FsStreamTask->action_notify(TFsStreamTask::action_param_t::save_song, m_numProgEdit);

			Leds::requestBlinking();

			DisplayTask->Clear();

			emb_string trackNumString;
			emb_printf::sprintf(trackNumString, "Track %1 selected", m_editingTrack + 1);
			DisplayTask->StringOut(0, 0, (uint8_t*)trackNumString.c_str());
			taskDelay(500);

			loadSong(m_editingTrack);
		}
	}
}

void MenuEditPlaylist::encoderLongPress()
{
	FsStreamTask->editingSong.playNext = !FsStreamTask->editingSong.playNext;
	printPlayNextMark();
	FsStreamTask->action_notify(TFsStreamTask::action_param_t::save_song, m_numProgEdit);
}

void MenuEditPlaylist::encoderClockwise()
{
	FsStreamTask->next_notify();
	FsStreamTask->get_browser_name(m_chosenTrackName);
	m_chosenTrackPath = m_chosenTrackName;

	m_state = EDITING;

	refresh();
}

void MenuEditPlaylist::encoderCounterClockwise()
{
	FsStreamTask->prev_notify();
	emb_string tmp;
	FsStreamTask->get_browser_name(m_chosenTrackName);
	FsStreamTask->curr_path(tmp);

	if (!tmp.compare("/SONGS") && !m_chosenTrackName.compare(".."))
	{
		FsStreamTask->next_notify();
	}

	FsStreamTask->get_browser_name(m_chosenTrackName);
	m_chosenTrackPath = m_chosenTrackName;

	m_state = EDITING;

	refresh();
}

void MenuEditPlaylist::keyStop()
{
	showChild(new MenuDeleteSong(this, m_numProgEdit));
}

void MenuEditPlaylist::keyStart()
{

}

void MenuEditPlaylist::keyLeftUp()
{
	m_numProgEdit = (m_numProgEdit + 10) % 99;
	loadSong();
}

void MenuEditPlaylist::keyLeftDown()
{
	if(m_numProgEdit > 10) m_numProgEdit -= 10;
	loadSong();
}

void MenuEditPlaylist::keyRightUp()
{
	if(m_numProgEdit) m_numProgEdit--;
	loadSong();
}

void MenuEditPlaylist::keyRightDown()
{
	m_numProgEdit = (m_numProgEdit + 1) % 99;
	loadSong();
}

void MenuEditPlaylist::keyReturn()
{
	m_editingTrack = 0;

	DisplayTask->Clear();
	emb_string trackString;
	emb_printf::sprintf(trackString, "Track %1", m_editingTrack + 1);
	DisplayTask->StringOut(0, 0, (uint8_t*) trackString.c_str());

	m_chosenTrackName = FsStreamTask->editingSong.trackName[m_editingTrack];
	m_chosenTrackPath = FsStreamTask->editingSong.trackPath[m_editingTrack];

	if(m_chosenTrackPath.empty())
	{
		m_chosenTrackName = noWavString;
		m_state = SELECTED_NO_FILE;
	}
	else
	{
		m_state = SELECTED_FILE_EXIST;
	}
	refresh();
}

void MenuEditPlaylist::keyForward()
{
	m_editingTrack = 1;

	DisplayTask->Clear();
	emb_string trackString;
	emb_printf::sprintf(trackString, "Track %1", m_editingTrack + 1);
	DisplayTask->StringOut(0, 0, (uint8_t*) trackString.c_str());

	m_chosenTrackName = FsStreamTask->editingSong.trackName[m_editingTrack];
	m_chosenTrackPath = FsStreamTask->editingSong.trackPath[m_editingTrack];

	if(m_chosenTrackPath.empty())
	{
		m_chosenTrackName = noWavString;
		m_state = SELECTED_NO_FILE;
	}
	else
	{
		m_state = SELECTED_FILE_EXIST;
	}
	refresh();
}

void MenuEditPlaylist::keyEsc()
{
	m_parentMenu->returnFromChildMenu();
}

void MenuEditPlaylist::loadSong(uint8_t showState)
{
	runningNamePos = 0;
	DisplayTask->Clear();

	Leds::digit(m_numProgEdit);

	DisplayTask->Clear();
	emb_string trackString;
	emb_printf::sprintf(trackString, "Track %1", showState + 1);
	DisplayTask->StringOut(0, 0, (uint8_t*) trackString.c_str());

	emb_string songPath;
	emb_printf::sprintf(songPath, "%s/%1.ego", FsStreamTask->browserPlaylistFolder().c_str(), m_numProgEdit);
	if(FsStreamTask->editingSong.load(songPath) == Song::eOk)
	{
		m_chosenTrackName = FsStreamTask->editingSong.trackName[showState];
		m_chosenTrackPath = FsStreamTask->editingSong.trackPath[showState];
		printRunningName(m_chosenTrackName, 0, 1);

		m_state = SELECTED_FILE_EXIST;
	}
	else
	{
		FsStreamTask->editingSong = Song();
		m_chosenTrackPath.clear();
		m_chosenTrackName = noWavString;

		m_state = SELECTED_NO_FILE;
	}

	m_editingTrack = showState;

	tim7_start(0);
}

void MenuEditPlaylist::printPlayNextMark()
{
	if(FsStreamTask->editingSong.playNext == true)
		DisplayTask->SymbolOut(15, 0, SYMBOL_NEXT_MARK);
	else
		DisplayTask->Clear_str(15, 0, 1);
}
