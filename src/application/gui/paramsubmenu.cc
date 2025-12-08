#include "paramsubmenu.h"

#include "enc.h"
#include "display.h"
#include "system.h"

ParamSubmenu::ParamSubmenu(gui_param_type paramType, const char* name, AbstractMenu* menu, void* param)
				:ParamBase(paramType, name, param)
{
	m_menu = menu;
}

ParamSubmenu::ParamSubmenu(gui_param_type paramType, const char* name,
		AbstractMenu* (*submenuCreationFunction)(AbstractMenu* parent), void* param)
				:ParamBase(paramType, name, param)
{
	m_submenuCreationFunction = submenuCreationFunction;
}

void ParamSubmenu::printParam(uint8_t yDisplayPosition)
{
	if(m_disabled) return;

	switch(m_type)
	{
		case ParamBase::GUI_PARAMETER_SUBMENU:
			break;
		default: break;
	}
}

void ParamSubmenu::showSubmenu(AbstractMenu* parent)
{
	if(m_submenuCreationFunction)
	{
		AbstractMenu* newSubMenu = m_submenuCreationFunction(parent);
		currentMenu = newSubMenu;
		if(newSubMenu) newSubMenu->show();
	}
}
