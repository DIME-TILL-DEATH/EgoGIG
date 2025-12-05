#include "AbstractMenu.h"

AbstractMenu* currentMenu;
uint8_t AbstractMenu::subMenusToRoot = 1;

gui_menu_type AbstractMenu::menuType()
{
	return m_menuType;
}

void AbstractMenu::returnFromChildMenu()
{
	currentMenu = this;


	if(childMenu)
	{
		delete childMenu;
		childMenu = nullptr;
	}

	if(parentMenu)
	{
		if(subMenusToRoot > 1)
		{
			subMenusToRoot--;
			parentMenu->returnFromChildMenu();
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
	childMenu = child;
	currentMenu = childMenu;
	child->show();
}
