#ifndef SRC_APPLICATION_GUI_MENUMETRONOMESETTINGS_H_
#define SRC_APPLICATION_GUI_MENUMETRONOMESETTINGS_H_


#include "menuparamlist.h"
#include "menusystem.h"

class MenuMetronomeSettings: public MenuParamList
{
public:
	MenuMetronomeSettings(AbstractMenu* parent);

	void task() override;
	void keyEsc() override;

	void keyStop() override;
	void keyStart() override;
};


#endif /* SRC_APPLICATION_GUI_MENUMETRONOMESETTINGS_H_ */
