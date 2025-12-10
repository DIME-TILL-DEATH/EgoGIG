#ifndef SRC_APPLICATION_GUI_MENULINKEDPARAM_H_
#define SRC_APPLICATION_GUI_MENULINKEDPARAM_H_

#include "abstractmenu.h"

#include "parambase.h"

class MenuLinkedParam : public AbstractMenu
{
	enum SelectionState
	{
		PARAM_NOT_SELECTED = 0,
		PARAM_SELECTED,
		SUBPARAM_SELECTED
	};
public:
	MenuLinkedParam(AbstractMenu* parent, GuiMenuType menuType);
	virtual ~MenuLinkedParam();

	typedef struct
	{
		uint8_t count;
		ParamBase** link;
	}SubParamLinks;

	virtual void show(TShowMode showMode = FirstShow) override;
	virtual void task() override;
	virtual void refresh() override;

	virtual void encoderPress() override;
	virtual void encoderClockwise() override;
	virtual void encoderCounterClockwise() override;

	void keyEsc() override;

	void setParams(ParamBase** settlingParamList, SubParamLinks* links, uint8_t setlingParamCount);

protected:


	uint8_t m_currentParamNum{0};
	uint8_t m_paramsCount{0};

	ParamBase** m_params{nullptr};
	SubParamLinks* m_subParamLinks{nullptr};

	SelectionState m_selectionState{PARAM_NOT_SELECTED};

	void printMenu();
};

#endif /* SRC_APPLICATION_GUI_MENULINKEDPARAM_H_ */
