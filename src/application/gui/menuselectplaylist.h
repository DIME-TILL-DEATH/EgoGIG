#ifndef SRC_APPLICATION_GUI_MENUSELECTPLAYLIST_H_
#define SRC_APPLICATION_GUI_MENUSELECTPLAYLIST_H_

#include "abstractmenu.h"

class MenuSelectPlaylist : public AbstractMenu
{
public:
	MenuSelectPlaylist(AbstractMenu* parent);

	void show(TShowMode showMode = FirstShow) override;
	void refresh() override;
	void task() override;

	void encoderPress() override;
	void encoderLongPress() override;
	void encoderClockwise() override;
	void encoderCounterClockwise() override;

	void keyEsc() override;
};


#endif /* SRC_APPLICATION_GUI_MENUSELECTPLAYLIST_H_ */
