#ifndef SRC_APPLICATION_GUI_MENUMIDICONTROL_H_
#define SRC_APPLICATION_GUI_MENUMIDICONTROL_H_

#include "abstractmenu.h"

#include "parambase.h"

class MenuMidiControl : public AbstractMenu
{
public:
	MenuMidiControl(AbstractMenu* parent);
	~MenuMidiControl();

	enum MidiInMode
	{
		MIDI_IN_PC = 0,
		MIDI_IN_CC,
		MIDI_IN_NOTE
	};

	void show(TShowMode showMode = FirstShow);
	void task();

	void encoderPress();
	void encoderClockwise();
	void encoderCounterClockwise();

	void keyEsc();

	static void write_ctrl(void);
	static void read_ctrl(void);

private:
	uint8_t m_currentParamNum{0};

	static constexpr uint8_t paramsCount = 5;
	static constexpr uint8_t subParamsCount = 9;

	ParamBase* m_params[paramsCount];
	ParamBase* m_subParams[subParamsCount];

	enum SelectionState
	{
		PARAM_NOT_SELECTED = 0,
		PARAM_SELECTED,
		SUBPARAM_SELECTED
	};
	SelectionState m_selectionState{PARAM_NOT_SELECTED};

	static AbstractMenu* createMidiPcMenu(AbstractMenu* parent);

	void printMenu();
	static bool check_busy(uint8_t type, uint8_t val, uint8_t num);
};

#endif /* SRC_APPLICATION_GUI_MENUMIDICONTROL_H_ */
