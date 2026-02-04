#include "menumidicontrol.h"

#include "menumidipc.h"

#include "paramstringlist.h"
#include "paramsubmenu.h"

#include "init.h"
#include "display.h"
#include "fs_stream.h"

MenuMidiControl::MenuMidiControl(AbstractMenu* parent)
	: MenuLinkedParam(parent, MENU_MIDI_CONTROL)
{
	uint8_t paramsCount = 5;
	ParamBase** params = new ParamBase*[paramsCount];
	SubParamLinks* subParamLinks = new SubParamLinks[paramsCount];

	params[0] = new ParamStringList("Play ", &ctrl_param[ctrl1_t], { "PC  ", "CC  ", "Note" }, 5);
	subParamLinks[0].count = params[0]->maxValue() + 1;
	subParamLinks[0].link = new ParamBase*[subParamLinks[0].count];
	subParamLinks[0].link[0] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "PC  ", &ctrl_param[ctrl1]);
	subParamLinks[0].link[0]->setScaling(1, 1);
	subParamLinks[0].link[1] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "CC  ", &ctrl_param[ctrl1]);
	subParamLinks[0].link[2] = new ParamBase(ParamBase::GUI_PARAMETER_NOTE, "Note", &ctrl_param[ctrl1]);

	params[1] = new ParamStringList("Pause ", &ctrl_param[ctrl2_t], { "PC  ", "CC  ", "Note" }, 5);
	subParamLinks[1].count = params[1]->maxValue() + 1;
	subParamLinks[1].link = new ParamBase*[subParamLinks[1].count];
	subParamLinks[1].link[0] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "PC  ", &ctrl_param[ctrl2]);
	subParamLinks[1].link[0]->setScaling(1, 1);
	subParamLinks[1].link[1] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "CC  ", &ctrl_param[ctrl2]);
	subParamLinks[1].link[2] = new ParamBase(ParamBase::GUI_PARAMETER_NOTE, "Note", &ctrl_param[ctrl2]);

	params[2] = new ParamStringList("Stop  ", &ctrl_param[ctrl3_t], { "PC  ", "CC  ", "Note" }, 5);
	subParamLinks[2].count = params[2]->maxValue() + 1;
	subParamLinks[2].link = new ParamBase*[m_subParamLinks[2].count];
	subParamLinks[2].link[0] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "PC  ", &ctrl_param[ctrl3]);
	subParamLinks[2].link[0]->setScaling(1, 1);
	subParamLinks[2].link[1] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "CC  ", &ctrl_param[ctrl3]);
	subParamLinks[2].link[2] = new ParamBase(ParamBase::GUI_PARAMETER_NOTE, "Note", &ctrl_param[ctrl3]);

	params[3] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Channel", nullptr);
	subParamLinks[3].count = 1;
	subParamLinks[3].link = new ParamBase*[subParamLinks[3].count];
	subParamLinks[3].link[0] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Num", &ctrl_param[chann]);
	subParamLinks[3].link[0]->setScaling(1, 1);
	subParamLinks[3].link[0]->setBounds(0, 15);

	params[4] = new ParamSubmenu("MIDI PC Set", &MenuMidiControl::createMidiPcMenu, nullptr);
	subParamLinks[4].count = 0;

	for(int i = 0; i < paramsCount; i++)
	{
		for(uint8_t j = 0; j < subParamLinks[i].count; j++)
		{
			subParamLinks[i].link[j]->setDisplayPosition(5);
		}
	}

	setParams(params, subParamLinks, paramsCount);
}

void MenuMidiControl::refresh()
{
	printMenu();
	if(m_currentParamNum < 3)
	{
		if(check_busy(ctrl_param[m_currentParamNum * 2], ctrl_param[m_currentParamNum * 2 + 1], m_currentParamNum))
			DisplayTask->StringOut(12, 1, (uint8_t*)"BUSY");
	}
}

void MenuMidiControl::keyEsc()
{
	write_ctrl();
	m_parentMenu->returnFromChildMenu();
}

AbstractMenu* MenuMidiControl::createMidiPcMenu(AbstractMenu* parent)
{
	return new MenuMidiPc(parent);
}

void MenuMidiControl::write_ctrl(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/ctrl.ego", FA_WRITE);
	f_write(&fsys, ctrl_param, 32, &br);
	f_close(&fsys);
}

void MenuMidiControl::read_ctrl(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/ctrl.ego", FA_OPEN_ALWAYS | FA_READ);
	f_read(&fsys, ctrl_param, 32, &br);
	f_close(&fsys);
}

bool MenuMidiControl::check_busy(uint8_t type, uint8_t val, int8_t currentParamNum, int8_t currentSongNum)
{
	for (uint8_t i = 0; i < 3; i++)
	{
		if (val == ctrl_param[i * 2 + 1] && currentParamNum != i)
		{
			if (type == ctrl_param[i * 2]) return true;
		}
	}

	for (uint8_t j = 0; j < 99; j++)
	{
		if (val == pc_param[j * 2 + 1] && currentSongNum != j)
		{
			if (type == pc_param[j * 2]) return true;
		}
	}

	return false;
}
