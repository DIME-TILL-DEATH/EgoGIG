#ifndef SRC_APPLICATION_GUI_MENUMAIN_H_
#define SRC_APPLICATION_GUI_MENUMAIN_H_

#include "menuparamlist.h"

class MenuMain : public MenuParamList
{
public:
	MenuMain(AbstractMenu* parent);

private:

	static AbstractMenu* createSelectPlaylistMenu(AbstractMenu* parent);
	static AbstractMenu* createEditPlaylistMenu(AbstractMenu* parent);
	static AbstractMenu* createSystemMenu(AbstractMenu* parent);
	static AbstractMenu* createMidiMenu(AbstractMenu* parent);
};



#endif /* SRC_APPLICATION_GUI_MENUMAIN_H_ */
