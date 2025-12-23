#ifndef SRC_APPLICATION_GUI_MENUMIDIPC_H_
#define SRC_APPLICATION_GUI_MENUMIDIPC_H_

#include "abstractmenu.h"

#include "parambase.h"
#include "paramstringlist.h"

class MenuMidiPc : public AbstractMenu
{
public:
	MenuMidiPc(AbstractMenu* parent);
	~MenuMidiPc();

	void show(TShowMode showMode = FirstShow) override;
	void task() override;

	void encoderPress() override;
	void encoderClockwise() override;
	void encoderCounterClockwise() override;

	void keyEsc() override;

	static void write_map();

	static void read_map();

private:
	enum SelectionState
	{
		CHANGE_SONG = 0,
		CHANGE_TYPE,
		CHANGE_VALUE
	};

	SelectionState m_state;

	void printPage();
	void updateParamsPtr();

	uint8_t m_songNum{0};
	ParamBase* m_songNumParam{nullptr};
	ParamStringList* m_eventTypeParam{nullptr};
	ParamBase* m_pcParam{nullptr};
	ParamBase* m_ccParam{nullptr};
	ParamBase* m_noteParam{nullptr};
	ParamBase* m_actualParam{nullptr};
};


#endif /* SRC_APPLICATION_GUI_MENUMIDIPC_H_ */
