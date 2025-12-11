#ifndef SRC_APPLICATION_GUI_MENUDELETESONG_H_
#define SRC_APPLICATION_GUI_MENUDELETESONG_H_

#include "abstractmenu.h"

class MenuDeleteSong : public AbstractMenu
{
public:
	MenuDeleteSong(AbstractMenu* parent, uint8_t numSong);

	void show(TShowMode showMode = FirstShow) override;
	void task() override;


	void keyStop() override;
	void keyStart() override;
private:
	enum State
	{
		DELETE_REQUEST,
		DELETE_CONFIRM
	};

	State m_state{DELETE_REQUEST};
	uint8_t m_numSongToDelete{0};
};


#endif /* SRC_APPLICATION_GUI_MENUDELETESONG_H_ */
