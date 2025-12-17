#include "abstractmenu.h"
#include "cs.h"

#include "display.h"

extern TCSTask *CSTask; // for delayTask function!

AbstractMenu* currentMenu;
uint8_t AbstractMenu::subMenusToRoot = 1;
uint8_t AbstractMenu::runningNameLength = 15;
int8_t AbstractMenu::runningNamePos = 0;
uint8_t AbstractMenu::runningPauseCounter = 0;

GuiMenuType AbstractMenu::menuType()
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

void AbstractMenu::printRunningName(emb_string name, uint8_t xPos, uint8_t yPos, StingSize strSize)
{
	runningNameLength = name.size();

	uint8_t stringSize = 16;
	if(strSize == STRING_DOUBLE) stringSize = 32;

	uint8_t printString[stringSize+1];
	memset(printString, 0, stringSize+1);
	memcpy(printString, name.c_str() + runningNamePos, stringSize - xPos);

	DisplayTask->StringOut(xPos, yPos, printString);

	if((runningNameLength - (stringSize - xPos) - runningNamePos) <= 0)
	{
		if(runningPauseCounter < 4)
		{
			runningPauseCounter++;
		}
		else
		{
			runningPauseCounter = 0;
			runningNamePos = 0;
		}
	}
	else if(runningNamePos == 0)
	{
		if(runningPauseCounter < 4)
		{
			runningPauseCounter++;
		}
		else
		{
			runningPauseCounter = 0;
			runningNamePos++;
		}
	}
	else
	{
		runningNamePos++;
	}
}
