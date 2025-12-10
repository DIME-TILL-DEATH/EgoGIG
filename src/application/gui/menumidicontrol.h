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

	void show(TShowMode showMode = FirstShow) override;
	void task() override;
	void refresh() override;

	void encoderPress() override;
	void encoderClockwise() override;
	void encoderCounterClockwise() override;

	void keyEsc() override;

	static void write_ctrl(void);
	static void read_ctrl(void);

protected:
	enum SelectionState
	{
		PARAM_NOT_SELECTED = 0,
		PARAM_SELECTED,
		SUBPARAM_SELECTED
	};

	typedef struct
	{
		uint8_t count;
		ParamBase** link;
	}SubParamLinks;

	uint8_t m_currentParamNum{0};
	uint8_t m_paramsCount{0};

	ParamBase** m_params;
	SubParamLinks* m_subParamLinks;

	SelectionState m_selectionState{PARAM_NOT_SELECTED};

	static AbstractMenu* createMidiPcMenu(AbstractMenu* parent);

	void printMenu();
	static bool check_busy(uint8_t type, uint8_t val, uint8_t num);
};

#endif /* SRC_APPLICATION_GUI_MENUMIDICONTROL_H_ */
