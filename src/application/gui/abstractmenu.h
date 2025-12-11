#ifndef SRC_APPLICATION_GUI_ABSTRACTMENU_H_
#define SRC_APPLICATION_GUI_ABSTRACTMENU_H_

#include "appdefs.h"

extern uint8_t tim5_fl; // tim7 actually

enum GuiMenuType
{
	MENU_PLAYER,
	MENU_MAIN,
	MENU_METRONOME,
	MENU_SELECT_PLAYLIST,
	MENU_EDIT_PLAYLIST,
	MENU_SYSTEM,
	MENU_MIDI_CONTROL,
	MENU_MIDI_PC,
	MENU_DELETE_SONG,
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

	enum StingSize
	{
		STRING_SINGLE = 0,
		STRING_DOUBLE
	};

	AbstractMenu() {};
	virtual ~AbstractMenu() {};

	virtual void show(TShowMode showMode = FirstShow) {};
	virtual void refresh() {};
	virtual void task() {};

	virtual void encoderPress() {};
	virtual void encoderLongPress() {};
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

	GuiMenuType menuType();
	void showChild(AbstractMenu* child);
	void returnFromChildMenu();

	static void printRunningName(emb_string name, uint8_t xPos = 0, StingSize  strSize = STRING_SINGLE);

protected:
	AbstractMenu* m_parentMenu = nullptr;
	AbstractMenu* m_childMenu = nullptr;

	GuiMenuType m_menuType{MENU_ABSTRACT};

	static uint8_t subMenusToRoot;

	static uint8_t runningNameLength;
	static uint8_t runningPauseCounter;
	static int8_t runningNamePos;

	void taskDelay(uint32_t ticks);
	void tim7_start(uint8_t val);
};

extern AbstractMenu* currentMenu;



#endif /* SRC_APPLICATION_GUI_ABSTRACTMENU_H_ */
