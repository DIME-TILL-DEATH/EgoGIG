#include "menulinkedparam.h"

#include "paramstringlist.h"
#include "paramsubmenu.h"

#include "init.h"
#include "display.h"

MenuLinkedParam::MenuLinkedParam(AbstractMenu* parent, GuiMenuType menuType)
{
	m_parentMenu = parent;
	m_menuType = menuType;
}

MenuLinkedParam::~MenuLinkedParam()
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

void MenuLinkedParam::setParams(ParamBase** settlingParamList, SubParamLinks* links, uint8_t setltingParamCount)
{
	m_params = settlingParamList;
	m_paramsCount = setltingParamCount;
	m_subParamLinks = links;
//	for(int i=0; i<maxParamCount; i++)
//	{
//		m_paramsList[i] = nullptr; // clean up
//	}
//
//	m_paramsCount = setlingParamCount;
//
//	for(int i=0; i<m_paramsCount; i++)
//	{
//		m_paramsList[i] = settlingParamList[i];
//	}
//
//	m_pagesCount = ceil((float)m_paramsCount/(float)paramsOnPage);
}

void MenuLinkedParam::show(TShowMode showMode)
{
	refresh();
}

void MenuLinkedParam::refresh()
{
	printMenu();
}

void MenuLinkedParam::task()
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

void MenuLinkedParam::encoderPress()
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

void MenuLinkedParam::encoderClockwise()
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

void MenuLinkedParam::encoderCounterClockwise()
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

void MenuLinkedParam::keyEsc()
{
	m_parentMenu->returnFromChildMenu();
}

void MenuLinkedParam::printMenu()
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
