#include "menuselectplaylist.h"

#include "init.h"
#include "display.h"
#include "leds.h"
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

	emb_string tmp;
	FsStreamTask->browser_name(tmp);
	DisplayTask->Clear();

	oem2winstar(tmp);
	DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
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
		taskDelay(1000);

		emb_string tmp;
		FsStreamTask->browser_name(tmp);
		DisplayTask->Clear();

		oem2winstar(tmp);
		DisplayTask->StringOut(0, 0, (uint8_t*) tmp.c_str());
	}
	else
	{
		menuPlayer->num_prog = 0;
		Leds::digit(menuPlayer->num_prog);

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
	emb_string selectedPath;
	FsStreamTask->browser_name(selectedPath);

	while(selectedPath.find_first_of('.') == 0)
	{
		FsStreamTask->next_notify();
		FsStreamTask->browser_name(selectedPath);
	}

	DisplayTask->Clear();
	oem2winstar(selectedPath);
	DisplayTask->StringOut(0, 0, (uint8_t*) selectedPath.c_str());
}

void MenuSelectPlaylist::encoderCounterClockwise()
{
	FsStreamTask->prev_notify();
	emb_string selectedPath;
	emb_string tmp1;
	FsStreamTask->browser_name(selectedPath);
	FsStreamTask->curr_path(tmp1);

	if (!tmp1.compare("/PLAYLIST") && selectedPath.compare("..") == 0)
	{
		encoderClockwise();//FsStreamTask->next_notify();
	}
	else
	{
		while(selectedPath.find_first_of('.') == 0 && selectedPath.compare("..") != 0)
		{
			FsStreamTask->prev_notify();
			FsStreamTask->browser_name(selectedPath);
		}

		DisplayTask->Clear();
		oem2winstar(selectedPath);
		DisplayTask->StringOut(0, 0, (uint8_t*) selectedPath.c_str());
	}
}

void MenuSelectPlaylist::keyEsc()
{
	m_parentMenu->returnFromChildMenu();
}
