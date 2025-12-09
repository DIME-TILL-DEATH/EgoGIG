#include "menuparamlist.h"

#include "paramsubmenu.h"

#include "display.h"
#include "gui.h"

MenuParamList::MenuParamList(AbstractMenu* parent, gui_menu_type menuType)
{
	m_parentMenu = parent;
	m_menuType = menuType;

	for(int i=0; i<maxParamCount; i++)
	{
			m_paramsList[i] = nullptr;
	}
}

MenuParamList::~MenuParamList()
{
	for(int i=0; i<m_paramsCount; i++)
	{
		if(m_paramsList[i]) delete m_paramsList[i];
	}
}

void MenuParamList::setParams(ParamBase** settlingParamList, uint8_t setlingParamCount)
{
	for(int i=0; i<maxParamCount; i++)
	{
		if(m_paramsList[i])
		{
			delete m_paramsList[i];
			m_paramsList[i] = nullptr; // clean up
		}

	}

	m_paramsCount = setlingParamCount;

	for(int i=0; i<m_paramsCount; i++)
	{
		m_paramsList[i] = settlingParamList[i];
	}

	m_pagesCount = ceil((float)m_paramsCount/(float)paramsOnPage);
}


void MenuParamList::show(TShowMode showMode)
{
	currentMenu = this;

	DisplayTask->Clear();

	if(showMode == FirstShow)
	{
		m_currentPageNumber = -1;

		for(int i = 0; i<m_paramsCount; i++)
		{
			if(m_paramsList[i]->type() == ParamBase::GUI_PARAMETER_LIST)
			{
//				StringListParam* strParam = static_cast<StringListParam*>(m_paramsList[i]);
//				uint8_t* valDisableMask = strParam->getDisableMask();
//				for(int a=0; a<m_paramsCount; a++)
//				{
//					uint8_t disableByMask = 0;
//					if(valDisableMask) disableByMask = valDisableMask[a];
//
//					m_paramsList[a]->setDisabled(m_paramsList[a]->disabled() | disableByMask);
//				}
			}
		}
		m_encoderKnobSelected = false;
		m_firstSelectableParam = 0;
		while(m_paramsList[m_firstSelectableParam]->type() == ParamBase::GUI_PARAMETER_DUMMY
			||m_paramsList[m_firstSelectableParam]->type() == ParamBase::GUI_PARAMETER_STRING_OUT) m_firstSelectableParam++;

		m_currentParamNum = m_firstSelectableParam;
		m_lastSelectableParam = m_paramsCount - 1;
	}

	printPage();
}

void MenuParamList::refresh()
{
	printPage();
}

void MenuParamList::keyEsc()
{
	m_parentMenu->returnFromChildMenu();
}

void MenuParamList::task()
{
	if(m_paramsCount == 1) m_encoderKnobSelected = true;

	if(!m_encoderKnobSelected)
	{
		if(tim5_fl)
			DisplayTask->Clear_str(0, m_currentParamNum % paramsOnPage, m_paramsList[m_currentParamNum]->nameLength());
		else
			DisplayTask->StringOut(0, m_currentParamNum % paramsOnPage, (uint8_t*)(m_paramsList[m_currentParamNum]->name()));
	}
	else
	{
		if(tim5_fl)
			DisplayTask->Clear_str(m_paramsList[m_currentParamNum]->nameLength(), m_currentParamNum % paramsOnPage, 16 - m_paramsList[m_currentParamNum]->nameLength() - 1);
		else
			m_paramsList[m_currentParamNum]->printParam(m_currentParamNum % paramsOnPage);
	}
}

void MenuParamList::encoderPress()
{
	if(m_paramsList[m_currentParamNum]->disabled()) return;

	if(m_paramsList[m_currentParamNum]->type() == ParamBase::GUI_PARAMETER_SUBMENU)
	{
		ParamSubmenu* submenuParam = static_cast<ParamSubmenu*>(m_paramsList[m_currentParamNum]);
		submenuParam->showSubmenu(this);
	}
	else
	{
		if(!m_encoderKnobSelected)
		{
			m_encoderKnobSelected = true;
			DisplayTask->StringOut(0, m_currentParamNum % paramsOnPage, (uint8_t*)(m_paramsList[m_currentParamNum]->name()));
		}
		else
		{
			m_encoderKnobSelected = false;
			m_paramsList[m_currentParamNum]->printParam(m_currentParamNum % paramsOnPage);
		}
	}

	tim7_start(1);
}

void MenuParamList::encoderClockwise()
{
	if(!m_encoderKnobSelected)
	{
		if(m_currentParamNum < m_lastSelectableParam) //(paramsCount - 1)
		{
			do{
				m_currentParamNum++; // Порядок важен!
			}while((m_paramsList[m_currentParamNum]->type() == ParamBase::GUI_PARAMETER_DUMMY
					|| m_paramsList[m_currentParamNum]->type() == ParamBase::GUI_PARAMETER_STRING_OUT)
					&& m_currentParamNum < m_lastSelectableParam);
			printPage();
			tim7_start(0);
		}
	}
	else
	{
		if(!m_paramsList[m_currentParamNum]->inverse()) m_paramsList[m_currentParamNum]->increaseParam();
		else m_paramsList[m_currentParamNum]->decreaseParam();

		// Support CustomParam
		if(m_paramsList[m_currentParamNum]->type() == ParamBase::GUI_PARAMETER_LIST)
			printPage();	// whole page to show "disabled" param changes
		else
			m_paramsList[m_currentParamNum]->printParam(m_currentParamNum % paramsOnPage);
	}
}

void MenuParamList::encoderCounterClockwise()
{
	if(!m_encoderKnobSelected)
	{
		if(m_currentParamNum > m_firstSelectableParam)
		{
			do{
				m_currentParamNum--; // Порядок важен!
			}while((m_paramsList[m_currentParamNum]->type() == ParamBase::GUI_PARAMETER_DUMMY
					|| m_paramsList[m_currentParamNum]->type() == ParamBase::GUI_PARAMETER_STRING_OUT)
					&& m_currentParamNum > m_firstSelectableParam);
			printPage();
//			tim7_start(0);
		}
	}
	else
	{
		if(!m_paramsList[m_currentParamNum]->inverse()) m_paramsList[m_currentParamNum]->decreaseParam();
				else m_paramsList[m_currentParamNum]->increaseParam();

		if(m_paramsList[m_currentParamNum]->type() == ParamBase::GUI_PARAMETER_LIST)
			printPage();	// whole page to show "disabled" param changes
		else
			m_paramsList[m_currentParamNum]->printParam(m_currentParamNum % paramsOnPage);
	}
}

void MenuParamList::printPage()
{
	int8_t newPageNumber;
	if(m_currentParamNum > 0) newPageNumber = m_currentParamNum / paramsOnPage;
	else newPageNumber = 0;

	if((newPageNumber != m_currentPageNumber))
	{
		DisplayTask->Clear();
	}
	m_currentPageNumber = newPageNumber;

	uint8_t stringCount = m_paramsCount - m_currentPageNumber * paramsOnPage;

	for(uint8_t i = 0; i < min(stringCount, (uint8_t)paramsOnPage); i++)
	{
		uint8_t displayParamNum = i + m_currentPageNumber * paramsOnPage;

		bool highlight = (m_currentParamNum == displayParamNum) && (m_paramsCount > 1);

		if(m_paramsList[displayParamNum]->type() == ParamBase::GUI_PARAMETER_DUMMY) continue;

		m_paramsList[displayParamNum]->printParam(i);
		DisplayTask->StringOut(0, i, (uint8_t*)(m_paramsList[displayParamNum]->name()));
	}

	uint8_t strelkaUp=' ';
	uint8_t strelkaDown= ' ';

	if(newPageNumber < m_pagesCount - 1)
	{
		strelkaUp = ' ';
		strelkaDown = SYMBOL_ARROW_DOWN;
	}
	if(newPageNumber > 0 && newPageNumber < m_pagesCount)
	{
		strelkaUp = SYMBOL_ARROW_UP;
		strelkaDown = SYMBOL_ARROW_DOWN;
	}
	if(newPageNumber == m_pagesCount - 1)
	{
		strelkaUp = SYMBOL_ARROW_UP;
		strelkaDown = ' ';
	}
	if(m_pagesCount == 1) strelkaUp = strelkaDown = ' ';

	DisplayTask->SymbolOut(15, 0, strelkaUp);
	DisplayTask->SymbolOut(15, 1, strelkaDown);
}

