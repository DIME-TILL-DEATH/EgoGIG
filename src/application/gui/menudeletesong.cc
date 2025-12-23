#include "menudeletesong.h"

#include "display.h"
#include "fs_stream.h"

MenuDeleteSong::MenuDeleteSong(AbstractMenu* parent, uint8_t numSong)
{
	m_parentMenu = parent;
	m_menuType = MENU_DELETE_SONG;
	m_numSongToDelete = numSong;
}

void MenuDeleteSong::show(TShowMode showMode)
{
	DisplayTask->Clear();
	DisplayTask->StringOut(2, 0, (uint8_t*) "Remove song");
	DisplayTask->StringOut(1, 1, (uint8_t*) "from playlist?");
}

void MenuDeleteSong::task()
{

}

void MenuDeleteSong::keyStop()
{
	DisplayTask->Clear();
	m_parentMenu->returnFromChildMenu();
}

void MenuDeleteSong::keyStart()
{
	switch(m_state)
	{
	case DELETE_REQUEST:
	{
		DisplayTask->Clear();
		DisplayTask->StringOut(5, 0, (uint8_t*) "SURE?");
		m_state = DELETE_CONFIRM;
		break;
	}

	case DELETE_CONFIRM:
	{
		FsStreamTask->delete_track(m_numSongToDelete);
		DisplayTask->StringOut(2, 0, (uint8_t*) "Song deleted");
		taskDelay(1000);
		m_parentMenu->returnFromChildMenu();
		break;
	}
	}
}
