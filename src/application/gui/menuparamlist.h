#ifndef SRC_APPLICATION_GUI_MENUPARAMLIST_H_
#define SRC_APPLICATION_GUI_MENUPARAMLIST_H_

#include "abstractmenu.h"
#include "parambase.h"

class MenuParamList : public AbstractMenu
{
public:
	MenuParamList(AbstractMenu* parent, gui_menu_type menuType);
	~MenuParamList() override;

	void setParams(ParamBase** settlingParamList, uint8_t setlingParamCount);

	void show(TShowMode showMode = FirstShow) override;
	void refresh() override;
	void task() override;

	void encoderPress() override;
	void encoderLongPress() override;
	void encoderClockwise() override;
	void encoderCounterClockwise() override;

	void keyStop() override;
	void keyStart() override;


	void keyLeftUp() override;
	void keyLeftDown() override;
	void keyRightUp() override;
	void keyRightDown() override;
	void keyReturn() override;
	void keyForward() override;
	void keyEsc() override;

	static constexpr uint8_t maxParamCount = 16;
	static constexpr uint8_t paramsOnPage = 2;
	static constexpr uint8_t leftPad = 3;

protected:
	uint8_t m_currentParamNum = 0;

	ParamBase* m_paramsList[maxParamCount];
	uint8_t m_paramsCount{0};

	uint8_t m_firstSelectableParam;
	uint8_t m_lastSelectableParam;

	int8_t m_currentPageNumber{-1};

	uint8_t m_pagesCount;

	bool m_encoderKnobSelected;

	void printPage();
};

#endif /* SRC_APPLICATION_GUI_MENUPARAMLIST_H_ */
