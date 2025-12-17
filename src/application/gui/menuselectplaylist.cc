#include "menuselectplaylist.h"

#include "init.h"
#include "display.h"
#include "fs_stream.h"

MenuSelectPlaylist::MenuSelectPlaylist(AbstractMenu* parent)
{
	m_parentMenu = parent;
	m_menuType = MENU_SELECT_PLAYLIST;
}

void MenuSelectPlaylist::show(TShowMode showMode)
{
	FsStreamTask->enter_dir("/PLAYLIST", "", true);
	FsStreamTask->next_notify();

	DisplayTask->Clear();
	DisplayTask->StringOut(0, 0, (uint8_t*) "Select playlist ");
	DisplayTask->StringOut(0, 1, (uint8_t*) "    folder      ");


	taskDelay(1000);
	DisplayTask->Clear();

	emb_string tmp;
	FsStreamTask->browser_name(tmp);
	DisplayTask->Clear();

	oem2winstar(tmp);
	DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
	num_prog = 0;
}

void MenuSelectPlaylist::refresh()
{

}

void MenuSelectPlaylist::task()
{

}

void MenuSelectPlaylist::encoderPress()
{
	FsStreamTask->action_notify(TFsStreamTask::action_param_t::enter_directory, 0);
	emb_string tmp;
	FsStreamTask->browser_name(tmp);
	DisplayTask->Clear();
	oem2winstar(tmp);
	DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
}

void MenuSelectPlaylist::encoderLongPress()
{
	if (!FsStreamTask->play_list_folder())
	{
		DisplayTask->Clear();
		DisplayTask->StringOut(2, 0, (uint8_t*) "Not Folder!");
	}
	else
	{
		DisplayTask->Clear();
		DisplayTask->StringOut(3, 0, (uint8_t*) "Select Ok!");
		taskDelay(1000);
		emb_string tmp;
		FsStreamTask->browser_name(tmp);
		DisplayTask->Clear();
		oem2winstar(tmp);
		DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
	}
}

void MenuSelectPlaylist::encoderClockwise()
{
	FsStreamTask->next_notify();
	emb_string tmp;
	FsStreamTask->browser_name(tmp);
	DisplayTask->Clear();
	oem2winstar(tmp);
	DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
}

void MenuSelectPlaylist::encoderCounterClockwise()
{
	FsStreamTask->prev_notify();
	emb_string tmp;
	emb_string tmp1;
	FsStreamTask->browser_name(tmp1);
	FsStreamTask->curr_path(tmp);
	if (!tmp.compare("/PLAYLIST") && !tmp1.compare(".."))
		FsStreamTask->next_notify();
	else
	{
		DisplayTask->Clear();
		oem2winstar(tmp1);
		DisplayTask->StringOut(0, 0, (uint8_t*) tmp1.c_str());
	}
}

void MenuSelectPlaylist::keyEsc()
{
	m_parentMenu->returnFromChildMenu();
}
