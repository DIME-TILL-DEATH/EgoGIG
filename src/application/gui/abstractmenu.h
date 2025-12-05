#ifndef SRC_APPLICATION_GUI_ABSTRACTMENU_H_
#define SRC_APPLICATION_GUI_ABSTRACTMENU_H_

#include "appdefs.h"

enum gui_menu_type
{
	MENU_PLAYER,

	MENU_ABSTRACT = 255
};

class AbstractMenu
{
public:
	enum TShowMode
	{
		FirstShow,
		ReturnShow
	};


	AbstractMenu() {};
	virtual ~AbstractMenu() {};

	virtual void show(TShowMode showMode = FirstShow) {};
	virtual void refresh() {};
	virtual void task() {};

	virtual void encoderPressed() {};
	virtual void encoderClockwise() {};
	virtual void encoderCounterClockwise() {};

	virtual void keyStop() {};
	virtual void keyStart() {};

	virtual void keyLeftUp() {};
	virtual void keyLeftDown() {};
	virtual void keyRightUp() {};
	virtual void keyRightDown() {};
	virtual void keyReturn() {};
	virtual void keyForward() {};
	virtual void keyEsc() {};

	gui_menu_type menuType();
	void showChild(AbstractMenu* child);
	void returnFromChildMenu();

protected:
	AbstractMenu* m_parentMenu = nullptr;
	AbstractMenu* m_childMenu = nullptr;

	gui_menu_type m_menuType{MENU_ABSTRACT};

	static uint8_t subMenusToRoot;

	void taskDelay(uint32_t ticks);
};

extern AbstractMenu* currentMenu;



#endif /* SRC_APPLICATION_GUI_ABSTRACTMENU_H_ */
