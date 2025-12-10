#ifndef SRC_APPLICATION_GUI_MENUEDITPLAYLIST_H_
#define SRC_APPLICATION_GUI_MENUEDITPLAYLIST_H_

#include "abstractmenu.h"

class MenuEditPlaylist : public AbstractMenu
{
public:
	MenuEditPlaylist(AbstractMenu* parent);

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

private:
	emb_string m_trackName;

	bool play_next_file = 0;

	uint8_t m_num_prog_edit{0};

	void loadSong();
	void printPlayNextMark();
};



#endif /* SRC_APPLICATION_GUI_MENUEDITPLAYLIST_H_ */
