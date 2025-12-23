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
	enum State
	{
		SELECTED_NO_FILE,
		SELECTED_FILE_EXIST,
		EDITING
	};
	State m_state{SELECTED_NO_FILE};

	uint8_t m_editingTrack{0};

	uint8_t m_numProgEdit{0};

	emb_string m_chosenTrackName;
	emb_string m_chosenTrackPath;

	const char* noWavString = "  No wav file";

	void loadSong(uint8_t showState = 0);
	void printPlayNextMark();
};



#endif /* SRC_APPLICATION_GUI_MENUEDITPLAYLIST_H_ */
