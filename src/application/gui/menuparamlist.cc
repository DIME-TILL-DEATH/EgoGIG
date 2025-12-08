#include "menuparamlist.h"

#include "display.h"

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

void MenuParamList::task()
{
	if(m_paramsCount == 1) m_encoderKnobSelected = true;

	if(!m_encoderKnobSelected)
	{
//		DisplayTask->StringOut(leftPad, m_currentParamNum % paramsOnPage, Font::fntSystem,
//								blinkFlag_fl * 2, (uint8_t*)(m_paramsList[m_currentParamNum]->name()));
	}
}

void MenuParamList::encoderPress()
{
	if(m_paramsList[m_currentParamNum]->disabled()) return;

	// CustomParam
	if(m_paramsList[m_currentParamNum]->type() == ParamBase::GUI_PARAMETER_SUBMENU)
	{
//		SubmenuParam* submenuParam = static_cast<SubmenuParam*>(m_paramsList[m_currentParamNum]);
//		submenuParam->showSubmenu(this);
	}
	else
	{
		if(!m_encoderKnobSelected)
		{
			m_encoderKnobSelected = true;
//			DisplayTask->StringOut(leftPad, m_currentParamNum % paramsOnPage, Font::fntSystem,
//									2, (uint8_t*)(m_paramsList[m_currentParamNum]->name()));
		}
		else
		{
			m_encoderKnobSelected = false;
//			DisplayTask->StringOut(leftPad, m_currentParamNum % paramsOnPage, Font::fntSystem,
//									0, (uint8_t*)(m_paramsList[m_currentParamNum]->name()));
		}
	}

//	tim5_start(1);
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
//			tim5_start(0);
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
//			tim5_start(0);
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
//		strelka_t drawStrelka;
//
//		if(newPageNumber < m_pagesCount - 1) drawStrelka = STRELKA_DOWN;
//		if(newPageNumber > 0 && newPageNumber < m_pagesCount) drawStrelka = STRELKA_UPDOWN;
//		if(newPageNumber == m_pagesCount - 1) drawStrelka = STRELKA_UP;
//		if(m_pagesCount == 1) drawStrelka = STRELKA_NONE;

		DisplayTask->Clear();
	}
	m_currentPageNumber = newPageNumber;

	uint8_t stringCount = m_paramsCount - m_currentPageNumber * paramsOnPage;

	for(uint8_t i = 0; i < min(stringCount, (uint8_t)4); i++)
	{
		uint8_t displayParamNum = i + m_currentPageNumber * paramsOnPage;

		bool highlight = (m_currentParamNum == displayParamNum) && (m_paramsCount > 1);

		if(m_paramsList[displayParamNum]->type() == ParamBase::GUI_PARAMETER_DUMMY) continue;

		m_paramsList[displayParamNum]->printParam(i);
//		DisplayTask->StringOut(leftPad, i, Font::fntSystem , 2 * highlight, (uint8_t*)(m_paramsList[displayParamNum]->name()));
	}
}

