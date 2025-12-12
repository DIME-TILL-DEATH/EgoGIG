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
	void keyStart() override;


	void keyLeftUp() override;
	void keyLeftDown() override;
	void keyRightUp() override;
	void keyRightDown() override;
	void keyReturn() override;
	void keyForward() override;
	void keyEsc() override;

	void requestPlayNext() { m_requestPlayNext = true; }
	void processPlayNext();

private:

	uint8_t stop_fl = 0;
	uint8_t play_point1_fl = 0;
	uint8_t play_point2_fl = 0;
	uint32_t no_file = 0;

	emb_string m_currentSongName;

	bool m_requestPlayNext{false};

	bool loadSong();
	bool test_file();

	void jump_rand_pos(uint32_t pos);
	void init_prog(void);
};


#endif /* SRC_APPLICATION_GUI_MENUPLAYER_H_ */
