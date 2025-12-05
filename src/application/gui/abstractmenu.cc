#include "abstractmenu.h"
#include "cs.h"

extern TCSTask *CSTask; // for delayTask function!

AbstractMenu* currentMenu;
uint8_t AbstractMenu::subMenusToRoot = 1;

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
