#ifndef SRC_APPLICATION_GUI_MENUMETRONOME_H_
#define SRC_APPLICATION_GUI_MENUMETRONOME_H_

#include "abstractmenu.h"


class MenuMetronome : public AbstractMenu
{
public:

	MenuMetronome(AbstractMenu* parent);
	~MenuMetronome();

	void show(TShowMode showMode = FirstShow) override;
	void refresh() override;
	void task() override;

	void encoderPress() override;
	void encoderClockwise() override;
	void encoderCounterClockwise() override;

	void keyEsc() override;

	void keyStop() override;
	void keyStart() override;

	void keyForward() override;

private:
	uint8_t tempo = 120;

	uint8_t tap_temp_global(void);
	void ind_temp(void);

	uint32_t tap_global;
};

#endif /* SRC_APPLICATION_GUI_MENUMETRONOME_H_ */
