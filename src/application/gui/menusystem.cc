#include "menusystem.h"

#include "ff.h"
#include "init.h"

#include "paramstringlist.h"
#include "paramsubmenu.h"

MenuSystem::MenuSystem(AbstractMenu* parent)
	:MenuParamList(parent, MENU_SYSTEM)
{
	const uint8_t paramNum = 5;
	ParamBase* params[paramNum];

	params[0] = new ParamStringList("Auto next", &sys_param[auto_next_track], {"Off", "On "}, 5);
	params[0]->setDisplayPosition(11);

	params[1] = new ParamStringList("Count dir", &sys_param[direction_counter], {"Up  ", "Down"}, 5);
	params[1]->setDisplayPosition(11);

	params[2] = new ParamStringList("Scroll dir", &sys_param[direction_scrol_playlist], {"Up  ", "Down"}, 5);
	params[2]->setDisplayPosition(11);

	params[3] = new ParamStringList("LB points", &sys_param[loop_points], {"Off", "On "}, 5);
	params[3]->setDisplayPosition(11);

	params[4] = new ParamStringList("METRONOME", &sys_param[metronome_out], {"OUT4", "OUT3", "OUT2", "OUT1",}, 5);
	params[4]->setDisplayPosition(11);
	params[4]->setBounds(0, 3);
	params[4]->setInverse(true);

	setParams(params, paramNum);
}

void MenuSystem::keyEsc()
{
	write_sys();
	m_parentMenu->returnFromChildMenu();
}

void MenuSystem::read_sys(void)
{
	FIL fsys;
	UINT br;

	memset(sys_param, 0, 64);
	f_open(&fsys, "/system.ego", FA_OPEN_ALWAYS | FA_READ);
	f_read(&fsys, sys_param, 64, &br);
	f_close(&fsys);
}

void MenuSystem::write_sys(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/system.ego", FA_WRITE);
	f_write(&fsys, sys_param, 64, &br);
	f_close(&fsys);
}

