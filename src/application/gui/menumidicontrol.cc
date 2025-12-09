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

	m_params[0] = new ParamStringList("Play ", &ctrl_param[ctrl1_t], { "PC  ", "CC  ", "Note" }, 5);
	m_subParams[0] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "PC  ", &ctrl_param[ctrl1]);
	m_subParams[1] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "CC  ", &ctrl_param[ctrl1]);
	m_subParams[2] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Note", &ctrl_param[ctrl1]);

	m_params[1] = new ParamStringList("Pause ", &ctrl_param[ctrl2_t], { "PC  ", "CC  ", "Note" }, 5);
	m_subParams[3] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "PC  ", &ctrl_param[ctrl2]);
	m_subParams[4] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "CC  ", &ctrl_param[ctrl2]);
	m_subParams[5] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Note", &ctrl_param[ctrl2]);

	m_params[2] = new ParamStringList("Stop  ", &ctrl_param[ctrl3_t], { "PC  ", "CC  ", "Note" }, 5);
	m_subParams[6] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "PC  ", &ctrl_param[ctrl3]);
	m_subParams[7] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "CC  ", &ctrl_param[ctrl3]);
	m_subParams[8] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Note", &ctrl_param[ctrl3]);

	m_params[3] = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Channel", &ctrl_param[chann]);
	m_params[4] = new ParamSubmenu("MIDI PC Set", &MenuMidiControl::createMidiPcMenu, nullptr);
}

MenuMidiControl::~MenuMidiControl()
{
	for(int i = 0; i < paramsCount; i++)
	{
		delete m_params[i];
	}

	for(int i = 0; i < subParamsCount; i++)
	{
		delete m_subParams[i];
	}
}

void MenuMidiControl::show(TShowMode showMode)
{
	printMenu();
}

void MenuMidiControl::task()
{
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
			DisplayTask->Clear_str(0, 1, 5);
		else
		{
			if(m_currentParamNum < 3)
				DisplayTask->StringOut(0, 1, (uint8_t*)(m_subParams[m_params[m_currentParamNum]->value() + 3 * m_currentParamNum]->name()));

			if(m_currentParamNum == 3)
				DisplayTask->StringOut(0, 1, (uint8_t*)"Num ");

		}
		break;
		break;
	}
	case SUBPARAM_SELECTED:
	{
		if(tim5_fl)
			DisplayTask->Clear_str(5, 1, 6);
		else
		{
			if(m_currentParamNum < 3)
			{
				uint8_t valueToShow = m_subParams[m_params[m_currentParamNum]->value() + 3 * m_currentParamNum]->value();

				switch(m_params[m_currentParamNum]->value())
				{
				case MIDI_IN_PC: DisplayTask->NumOut(5, 1, valueToShow + 1); break;
				case MIDI_IN_CC: DisplayTask->NumOut(5, 1, valueToShow); break;
				case MIDI_IN_NOTE: DisplayTask->NoteOut(5, 1, valueToShow); break;
				default: break;
				}
			}

			if(m_currentParamNum == 3)
			{
				DisplayTask->NumOut(5, 1, m_params[m_currentParamNum]->value() +1 );
			}
		}
		break;
	}
	}
}

void MenuMidiControl::encoderPress()
{
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
		if(m_currentParamNum < 3)
			DisplayTask->StringOut(0, 1, (uint8_t*)(m_subParams[m_params[m_currentParamNum]->value() + 3 * m_currentParamNum]->name()));

		if(m_currentParamNum == 3)
			DisplayTask->StringOut(0, 1, (uint8_t*)"Num ");

		m_selectionState = SUBPARAM_SELECTED;
		break;
	}
	case SUBPARAM_SELECTED:
	{
		if(m_currentParamNum < 3)
		{
			uint8_t valueToShow = m_subParams[m_params[m_currentParamNum]->value() + 3 * m_currentParamNum]->value();

			switch(m_params[m_currentParamNum]->value())
			{
			case MIDI_IN_PC: DisplayTask->NumOut(5, 1, valueToShow + 1); break;
			case MIDI_IN_CC: DisplayTask->NumOut(5, 1, valueToShow); break;
			case MIDI_IN_NOTE: DisplayTask->NoteOut(5, 1, valueToShow); break;
			default: break;
			}
		}

		if(m_currentParamNum == 3)
		{
			DisplayTask->NumOut(5, 1, m_params[m_currentParamNum]->value() + 1);
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
		if(m_currentParamNum < paramsCount-1) m_currentParamNum++;
		break;
	}
	case PARAM_SELECTED:
	{
		m_params[m_currentParamNum]->increaseParam();
		break;
	}
	case SUBPARAM_SELECTED:
	{
		if(m_currentParamNum == 3) m_params[m_currentParamNum]->increaseParam();
		else m_subParams[m_params[m_currentParamNum]->value() + 3 * m_currentParamNum]->increaseParam();
		break;
	}
	}

	printMenu();
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
		if(m_currentParamNum == 3) m_params[m_currentParamNum]->decreaseParam();
		else m_subParams[m_params[m_currentParamNum]->value() + 3 * m_currentParamNum]->decreaseParam();
		break;
	}
	}
	printMenu();
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

	if(m_currentParamNum < 3)
	{
		DisplayTask->StringOut(0, 1, (uint8_t*)(m_subParams[m_params[m_currentParamNum]->value() + 3 * m_currentParamNum]->name()));

		uint8_t valueToShow = m_subParams[m_params[m_currentParamNum]->value() + 3 * m_currentParamNum]->value();

		switch(m_params[m_currentParamNum]->value())
		{
		case MIDI_IN_PC: DisplayTask->NumOut(5, 1, valueToShow + 1); break;
		case MIDI_IN_CC: DisplayTask->NumOut(5, 1, valueToShow); break;
		case MIDI_IN_NOTE: DisplayTask->NoteOut(5, 1, valueToShow); break;
		default: break;
		}

		if(check_busy(ctrl_param[m_currentParamNum * 2], ctrl_param[m_currentParamNum * 2 + 1], m_currentParamNum))
			DisplayTask->StringOut(12, 1, (uint8_t*)"BUSY");
	}

	if(m_currentParamNum == 3)
	{
		DisplayTask->StringOut(0, 1, (uint8_t*)"Num ");
		DisplayTask->NumOut(5, 1, m_params[m_currentParamNum]->value() + 1);
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
