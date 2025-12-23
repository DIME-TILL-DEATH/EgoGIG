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

	void requestPlayNext() { m_requestPlayNext = true; }
	void processPlayNext();

	bool loopModeActive() {return m_loopModeActive;}

	uint8_t num_prog{0};

private:

	uint8_t playPoint1Selected = 0;
	uint8_t playPoint2Selected = 0;
	uint32_t no_file = 0;

	emb_string m_currentSongName;

	bool m_requestPlayNext{false};
	bool m_loopModeActive{true};

	bool loadSong();
	bool test_file();

	void initSong(void);
};


#endif /* SRC_APPLICATION_GUI_MENUPLAYER_H_ */
