#include "menumain.h"

#include "display.h"
#include "parambase.h"
#include "paramsubmenu.h"

//uint8_t menu_list[][16] =
//{ "Select Playlist", "Edit Playlist", "System", "Set MIDI Ctrl" };

MenuMain::MenuMain(AbstractMenu* parent)
	: MenuParamList(parent, MENU_MAIN)
{
	const uint8_t paramNum = 4;
	ParamBase* params[paramNum];

	params[0] = new ParamSubmenu(ParamBase::GUI_PARAMETER_SUBMENU, "Select Playlist", &MenuMain::createSelectPlaylistMenu, nullptr);
	params[1] = new ParamSubmenu(ParamBase::GUI_PARAMETER_SUBMENU, "Edit Playlist", &MenuMain::createEditPlaylistMenu, nullptr);
	params[2] = new ParamSubmenu(ParamBase::GUI_PARAMETER_SUBMENU, "System", &MenuMain::createSystemMenu, nullptr);
	params[3] = new ParamSubmenu(ParamBase::GUI_PARAMETER_SUBMENU, "Set MIDI Ctrl", &MenuMain::createMidiMenu, nullptr);

	setParams(params, paramNum);
}

AbstractMenu* MenuMain::createSelectPlaylistMenu(AbstractMenu* parent)
{
	return nullptr;
//	return new FswTypeMenu(parent);
}

AbstractMenu* MenuMain::createEditPlaylistMenu(AbstractMenu* parent)
{
	return nullptr;
//	return new FswTypeMenu(parent);
}

AbstractMenu* MenuMain::createSystemMenu(AbstractMenu* parent)
{
	return nullptr;
//	return new FswTypeMenu(parent);
}

AbstractMenu* MenuMain::createMidiMenu(AbstractMenu* parent)
{
	return nullptr;
//	return new FswTypeMenu(parent);
}
