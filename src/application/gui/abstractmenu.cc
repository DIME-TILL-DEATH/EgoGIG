#include "abstractmenu.h"
#include "cs.h"

#include "display.h"

extern TCSTask *CSTask; // for delayTask function!

AbstractMenu* currentMenu;
uint8_t AbstractMenu::subMenusToRoot = 1;
uint8_t AbstractMenu::runningNameLength = 15;
int8_t AbstractMenu::runningNamePos = 0;

gui_menu_type AbstractMenu::menuType()
{
	return m_menuType;
}

void AbstractMenu::returnFromChildMenu()
{
	currentMenu = this;


	if(m_childMenu)
	{
		delete m_childMenu;
		m_childMenu = nullptr;
	}

	if(m_parentMenu)
	{
		if(subMenusToRoot > 1)
		{
			subMenusToRoot--;
			m_parentMenu->returnFromChildMenu();
		}
		else
		{
			subMenusToRoot = 1;
			show(TShowMode::ReturnShow);
		}
	}
	else
	{
		subMenusToRoot = 1;
		show(TShowMode::ReturnShow);
	}
}

void AbstractMenu::showChild(AbstractMenu* child)
{
	m_childMenu = child;
	currentMenu = m_childMenu;
	child->show();
}

void AbstractMenu::taskDelay(uint32_t ticks)
{
	CSTask->Delay(ticks);
}


void AbstractMenu::tim7_start(uint8_t val)
{
	extern uint32_t led_blink_count3;
	led_blink_count3 = 300000;
	tim5_fl = 1 - val;
	//CSTask->Give();
}

void AbstractMenu::printRunningName(emb_string name, uint8_t xPos, uint8_t yPos)
{
	runningNameLength = name.size();

	uint8_t printString[16+1];
	memset(printString, 0, 16+1);
	memcpy(printString, name.c_str() + runningNamePos, 16 - xPos);

	DisplayTask->StringOut(xPos, yPos, printString);

	if((runningNameLength - 16 - runningNamePos) <= 0) runningNamePos = 0;
	else runningNamePos++;
}
