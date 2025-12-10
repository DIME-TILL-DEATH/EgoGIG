#include "menumidicontrol.h"

#include "paramstringlist.h"
#include "paramsubmenu.h"

#include "init.h"
#include "display.h"
#include "fs_stream.h"

MenuMidiControl::MenuMidiControl(AbstractMenu* parent)
{
	m_parentMenu = parent;
	m_menuType = MENU_MIDI_CONTROL;

	m_paramsCount = 5;

	m_params = new ParamBase*[m_paramsCount];
	m_subParamLinks = new SubParamLinks[m_paramsCount];

	m_params[0] = new ParamStringList("Play ", &ctrl_param[ctrl1_t], { "PC  ", "CC  ", "Note" }, 5);
	m_subParamLinks[0].count = m_params[0]->maxValue() + 1;
	m_subParamLinks[0].link = new ParamBase*[m_subParamLinks[0].count];
	m_subParamLinks[0].link[0] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "PC  ", &ctrl_param[ctrl1]);
	m_subParamLinks[0].link[0]->setScaling(1, 1);
	m_subParamLinks[0].link[1] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "CC  ", &ctrl_param[ctrl1]);
	m_subParamLinks[0].link[2] = new ParamBase(ParamBase::GUI_PARAMETER_NOTE, "Note", &ctrl_param[ctrl1]);

	m_params[1] = new ParamStringList("Pause ", &ctrl_param[ctrl2_t], { "PC  ", "CC  ", "Note" }, 5);
	m_subParamLinks[1].count = m_params[1]->maxValue() + 1;
	m_subParamLinks[1].link = new ParamBase*[m_subParamLinks[1].count];
	m_subParamLinks[1].link[0] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "PC  ", &ctrl_param[ctrl2]);
	m_subParamLinks[1].link[0]->setScaling(1, 1);
	m_subParamLinks[1].link[1] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "CC  ", &ctrl_param[ctrl2]);
	m_subParamLinks[1].link[2] = new ParamBase(ParamBase::GUI_PARAMETER_NOTE, "Note", &ctrl_param[ctrl2]);

	m_params[2] = new ParamStringList("Stop  ", &ctrl_param[ctrl3_t], { "PC  ", "CC  ", "Note" }, 5);
	m_subParamLinks[2].count = m_params[2]->maxValue() + 1;
	m_subParamLinks[2].link = new ParamBase*[m_subParamLinks[2].count];
	m_subParamLinks[2].link[0] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "PC  ", &ctrl_param[ctrl3]);
	m_subParamLinks[2].link[0]->setScaling(1, 1);
	m_subParamLinks[2].link[1] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "CC  ", &ctrl_param[ctrl3]);
	m_subParamLinks[2].link[2] = new ParamBase(ParamBase::GUI_PARAMETER_NOTE, "Note", &ctrl_param[ctrl3]);

	m_params[3] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Channel", nullptr);
	m_subParamLinks[3].count = 1;
	m_subParamLinks[3].link = new ParamBase*[m_subParamLinks[3].count];
	m_subParamLinks[3].link[0] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Num", &ctrl_param[chann]);
	m_subParamLinks[3].link[0]->setScaling(1, 1);

	m_params[4] = new ParamSubmenu("MIDI PC Set", &MenuMidiControl::createMidiPcMenu, nullptr);

	for(int i = 0; i < m_paramsCount; i++)
	{
		for(uint8_t j = 0; j < m_subParamLinks[i].count; j++)
		{
			m_subParamLinks[i].link[j]->setDisplayPosition(5);
		}
	}
}

MenuMidiControl::~MenuMidiControl()
{
	for(int i = 0; i < m_paramsCount; i++)
	{
		delete m_params[i];
		for(uint8_t j = 0; i < m_subParamLinks[j].count; j++)
		{
			delete m_subParamLinks[i].link[j];
		}
		delete[] m_subParamLinks[i].link;
	}
	delete[] m_params;
	delete[] m_subParamLinks;
}

void MenuMidiControl::show(TShowMode showMode)
{
	refresh();
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

void MenuMidiControl::task()
{
	uint8_t submenuNum = m_params[m_currentParamNum]->value();
	switch(m_selectionState)
	{
	case PARAM_NOT_SELECTED:
	{
		if(tim5_fl)
			DisplayTask->Clear_str(0, 0, m_params[m_currentParamNum]->nameLength());
		else
			DisplayTask->StringOut(0, 0, (uint8_t*)(m_params[m_currentParamNum]->name()));
		break;
	}
	case PARAM_SELECTED:
	{
		if(tim5_fl)
		{
			DisplayTask->Clear_str(0, 1, m_subParamLinks[m_currentParamNum].link[submenuNum]->xDisplayPosition() - 1);
		}
		else
		{
			if(m_params[m_currentParamNum]->type() != ParamBase::GUI_PARAMETER_SUBMENU)
			{
				DisplayTask->StringOut(0, 1, (uint8_t*)(m_subParamLinks[m_currentParamNum].link[submenuNum]->name()));
			}
		}
		break;
	}
	case SUBPARAM_SELECTED:
	{
		if(tim5_fl)
		{
			DisplayTask->Clear_str(m_subParamLinks[m_currentParamNum].link[submenuNum]->xDisplayPosition(), 1, 6);
		}
		else
		{
			if(m_params[m_currentParamNum]->type() != ParamBase::GUI_PARAMETER_SUBMENU)
			{
				m_subParamLinks[m_currentParamNum].link[submenuNum]->printParam(1);
			}
		}
		break;
	}
	}
}

void MenuMidiControl::encoderPress()
{
	uint8_t submenuNum = m_params[m_currentParamNum]->value();
	switch(m_selectionState)
	{
	case PARAM_NOT_SELECTED:
	{
		DisplayTask->StringOut(0, 0, (uint8_t*)(m_params[m_currentParamNum]->name()));

		if(m_currentParamNum == 4)
		{
			ParamSubmenu* subMenu = static_cast<ParamSubmenu*>(m_params[m_currentParamNum]);
			subMenu->showSubmenu(this);
		}
		else if(m_currentParamNum == 3) m_selectionState = SUBPARAM_SELECTED;
		else m_selectionState = PARAM_SELECTED;
		break;
	}
	case PARAM_SELECTED:
	{
		if(m_params[m_currentParamNum]->type() != ParamBase::GUI_PARAMETER_SUBMENU)
		{
			DisplayTask->StringOut(0, 1, (uint8_t*)(m_subParamLinks[m_currentParamNum].link[submenuNum]->name()));
		}

		m_selectionState = SUBPARAM_SELECTED;
		break;
	}
	case SUBPARAM_SELECTED:
	{
		if(m_params[m_currentParamNum]->type() != ParamBase::GUI_PARAMETER_SUBMENU)
		{
			m_subParamLinks[m_currentParamNum].link[submenuNum]->printParam(1);
		}
		else
		{
			ParamSubmenu* submenu = static_cast<ParamSubmenu*>(m_params[m_currentParamNum]);
			submenu->showSubmenu(this);
		}

		m_selectionState = PARAM_NOT_SELECTED;
		break;
	}
	}
	tim7_start(1);
}

void MenuMidiControl::encoderClockwise()
{
	switch(m_selectionState)
	{
	case PARAM_NOT_SELECTED:
	{
		if(m_currentParamNum < m_paramsCount-1) m_currentParamNum++;
		break;
	}
	case PARAM_SELECTED:
	{
		m_params[m_currentParamNum]->increaseParam();
		break;
	}
	case SUBPARAM_SELECTED:
	{
		uint8_t submenuNum = m_params[m_currentParamNum]->value();
		m_subParamLinks[m_currentParamNum].link[submenuNum]->increaseParam();
	}
	}

	refresh();
}

void MenuMidiControl::encoderCounterClockwise()
{
	switch(m_selectionState)
	{
	case PARAM_NOT_SELECTED:
	{
		if(m_currentParamNum > 0) m_currentParamNum--;
		break;
	}
	case PARAM_SELECTED:
	{
		m_params[m_currentParamNum]->decreaseParam();
		break;
	}
	case SUBPARAM_SELECTED:
	{
		uint8_t submenuNum = m_params[m_currentParamNum]->value();
		m_subParamLinks[m_currentParamNum].link[submenuNum]->decreaseParam();
		break;
	}
	}
	refresh();
}

void MenuMidiControl::keyEsc()
{
	write_ctrl();
	m_parentMenu->returnFromChildMenu();
}

AbstractMenu* MenuMidiControl::createMidiPcMenu(AbstractMenu* parent)
{
	return nullptr;
//	return new MenuMidiControl(parent);
}

void MenuMidiControl::printMenu()
{
	DisplayTask->Clear();

	DisplayTask->StringOut(0, 0, (uint8_t*)m_params[m_currentParamNum]->name());

	if(m_params[m_currentParamNum]->type() != ParamBase::GUI_PARAMETER_SUBMENU)
	{
		uint8_t submenuNum = m_params[m_currentParamNum]->value();
		DisplayTask->StringOut(0, 1, (uint8_t*)(m_subParamLinks[m_currentParamNum].link[submenuNum]->name()));
		m_subParamLinks[m_currentParamNum].link[submenuNum]->printParam(1);
	}
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

bool MenuMidiControl::check_busy(uint8_t type, uint8_t val, uint8_t currentParamNum)
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
		if (val == pc_param[j * 2 + 1] && currentParamNum != j)
		{
			if (type == pc_param[j * 2]) return true;
		}
	}

	return false;
}
