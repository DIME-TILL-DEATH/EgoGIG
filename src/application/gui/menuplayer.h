#ifndef SRC_APPLICATION_GUI_MENUPLAYER_H_
#define SRC_APPLICATION_GUI_MENUPLAYER_H_

#include "abstractmenu.h"

class MenuPlayer : public AbstractMenu
{
public:

	MenuPlayer();
	~MenuPlayer();

	void show(TShowMode showMode = FirstShow) override;
	void refresh() override;
	void task() override;

	void encoderPress() override;
	void encoderLongPress() override;
	void encoderClockwise() override;
	void encoderCounterClockwise() override;

	void keyStop() override;
	void keyStopLong() override;
	void keyStart() override;

	void keyLeftUp() override;
	void keyLeftDown() override;
	void keyRightUp() override;
	void keyRightDown() override;
	void keyReturn() override;
	void keyReturnLong() override;
	void keyForward() override;
	void keyForwardLong() override;
	void keyEsc() override;

	bool loopModeActive() {return m_loopModeActive;}

	void setSongNum(uint8_t songNum);
	uint8_t songNum() { return m_currentSongNum; };

	bool loadSong(uint8_t songNum);

private:
	uint8_t m_currentSongNum{0};
	uint8_t	m_requestedSongNum{0};

	uint8_t playPoint1Selected = 0;
	uint8_t playPoint2Selected = 0;
	uint32_t no_file = 0;

	emb_string m_currentSongName;

	bool m_loopModeActive{true};


	bool test_file();

	void initSong(void);
};


#endif /* SRC_APPLICATION_GUI_MENUPLAYER_H_ */
