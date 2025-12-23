#ifndef SRC_APPLICATION_GUI_PARAMSUBMENU_H_
#define SRC_APPLICATION_GUI_PARAMSUBMENU_H_

#include "abstractmenu.h"
#include "parambase.h"

class ParamSubmenu : public ParamBase
{
public:
	ParamSubmenu(const char* name, AbstractMenu* menu, void* param = nullptr);
	ParamSubmenu(const char* name, AbstractMenu* (*submenuCreationFunction)(AbstractMenu* parent), void* param = nullptr);

	void printParam(uint8_t yDisplayPosition) override;
	uint32_t value() const override {return 0;};

	void showSubmenu(AbstractMenu* parent);

private:
	AbstractMenu* m_menu;
	AbstractMenu* (*m_submenuCreationFunction)(AbstractMenu* parent) = nullptr;
};

#endif /* SRC_APPLICATION_GUI_PARAMSUBMENU_H_ */
