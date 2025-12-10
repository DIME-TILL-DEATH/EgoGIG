#ifndef SRC_APPLICATION_GUI_MENUMIDICONTROL_H_
#define SRC_APPLICATION_GUI_MENUMIDICONTROL_H_

#include "abstractmenu.h"
#include "menulinkedparam.h"

#include "parambase.h"

class MenuMidiControl : public MenuLinkedParam
{
public:
	MenuMidiControl(AbstractMenu* parent);

	enum MidiInMode
	{
		MIDI_IN_PC = 0,
		MIDI_IN_CC,
		MIDI_IN_NOTE
	};

	void refresh() override;
	void keyEsc() override;

	static void write_ctrl(void);
	static void read_ctrl(void);

private:
	static AbstractMenu* createMidiPcMenu(AbstractMenu* parent);
	static bool check_busy(uint8_t type, uint8_t val, uint8_t num);
};

#endif /* SRC_APPLICATION_GUI_MENUMIDICONTROL_H_ */
