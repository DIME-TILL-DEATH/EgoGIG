#ifndef SRC_APPLICATION_GUI_MENUSYSTEM_H_
#define SRC_APPLICATION_GUI_MENUSYSTEM_H_

#include "menuparamlist.h"

class MenuSystem : public MenuParamList
{
public:
	MenuSystem(AbstractMenu* parent);

	void keyEsc() override;

	static void read_sys(void);
	static void write_sys(void);
};

#endif /* SRC_APPLICATION_GUI_MENUSYSTEM_H_ */
