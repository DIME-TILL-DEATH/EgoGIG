#include "menumidipc.h"
#include "menumidicontrol.h"

#include "init.h"
#include "display.h"
#include "fs_stream.h"

MenuMidiPc::MenuMidiPc(AbstractMenu* parent)
{
	m_menuType = MENU_MIDI_PC;
	m_parentMenu = parent;

	m_songNumParam = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Song", &m_songNum);
	m_songNumParam->setScaling(1, 1);
	m_songNumParam->setBounds(0, 98);
	m_songNumParam->setDisplayPosition(6);

	m_eventTypeParam = new ParamStringList("Pause ", &pc_param[m_songNum * 2], { "PC  ", "CC  ", "Note" }, 5);

	m_pcParam = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Song", &pc_param[m_songNum * 2 + 1]);
	m_pcParam->setDisplayPosition(4);
	m_pcParam->setScaling(1, 1);
	m_ccParam = new ParamBase(ParamBase::GUI_PARAMETER_NUM, "Song", &pc_param[m_songNum * 2 + 1]);
	m_ccParam->setDisplayPosition(4);
	m_noteParam = new ParamBase(ParamBase::GUI_PARAMETER_NOTE, "Song", &pc_param[m_songNum * 2 + 1]);
	m_noteParam->setDisplayPosition(6);
}

MenuMidiPc::~MenuMidiPc()
{
	delete m_songNumParam;
	delete m_eventTypeParam;
	delete m_pcParam;
	delete m_ccParam;
	delete m_noteParam;
}

void MenuMidiPc::show(TShowMode showMode)
{
	updateParamsPtr();
	printPage();
}

void MenuMidiPc::task()
{
	switch(m_state)
	{
		case CHANGE_SONG:
		{
			if(tim5_fl)
				DisplayTask->Clear_str(m_songNumParam->xDisplayPosition(), 0, 2);
			else
				m_songNumParam->printParam(0);
			break;
		}
		case CHANGE_TYPE:
		{
			if(tim5_fl)
				DisplayTask->Clear_str(0, 1, strlen((const char*)m_eventTypeParam->getString()));
			else
				m_eventTypeParam->printParam(1);
			break;
		}
		case CHANGE_VALUE:
		{
			if(tim5_fl)
				DisplayTask->Clear_str(m_actualParam->xDisplayPosition(), 1, 4);
			else
				m_actualParam->printParam(1);
			break;
		}
	}
}

void MenuMidiPc::encoderPress()
{
	switch(m_state)
	{
		case CHANGE_SONG:
		{
			m_songNumParam->printParam(0);
			m_state = CHANGE_TYPE;
			break;
		}
		case CHANGE_TYPE:
		{
			m_eventTypeParam->printParam(1);
			m_state = CHANGE_VALUE;
			break;
		}
		case CHANGE_VALUE:
		{
			m_actualParam->printParam(1);
			m_state = CHANGE_SONG;
			break;
		}
	}
	tim7_start(1);
}

void MenuMidiPc::encoderClockwise()
{
	switch(m_state)
	{
		case CHANGE_SONG:
		{
			m_songNumParam->increaseParam();
			break;
		}
		case CHANGE_TYPE:
		{
			m_eventTypeParam->increaseParam();
			break;
		}
		case CHANGE_VALUE:
		{
			m_actualParam->increaseParam();
			break;
		}
	}

	updateParamsPtr();
	printPage();
}

void MenuMidiPc::encoderCounterClockwise()
{
	switch(m_state)
	{
		case CHANGE_SONG:
		{
			m_songNumParam->decreaseParam();
			break;
		}
		case CHANGE_TYPE:
		{
			m_eventTypeParam->decreaseParam();
			break;
		}
		case CHANGE_VALUE:
		{
			m_actualParam->decreaseParam();
			break;
		}
	}

	updateParamsPtr();
	printPage();
}

void MenuMidiPc::keyEsc()
{
	write_map();
	m_parentMenu->returnFromChildMenu();
}

void MenuMidiPc::printPage()
{
	DisplayTask->Clear();

	DisplayTask->StringOut(0, 0, (uint8_t*)m_songNumParam->name());
	m_songNumParam->printParam(0);
	m_eventTypeParam->printParam(1);
	m_actualParam->printParam(1);

	if(MenuMidiControl::check_busy(pc_param[m_songNum * 2], pc_param[m_songNum * 2 + 1], -1, m_songNum))
		DisplayTask->StringOut(10, 1, (uint8_t*)"BUSY");
}

void MenuMidiPc::updateParamsPtr()
{
	m_eventTypeParam->setValuePtr(&pc_param[m_songNum * 2]);
	m_pcParam->setValuePtr(&pc_param[m_songNum * 2 + 1]);
	m_ccParam->setValuePtr(&pc_param[m_songNum * 2 + 1]);
	m_noteParam->setValuePtr(&pc_param[m_songNum * 2 + 1]);

	switch((MenuMidiControl::MidiInMode)m_eventTypeParam->value())
	{
	case MenuMidiControl::MIDI_IN_PC: m_actualParam = m_pcParam; break;
	case MenuMidiControl::MIDI_IN_CC: m_actualParam = m_ccParam; break;
	case MenuMidiControl::MIDI_IN_NOTE: m_actualParam = m_noteParam; break;
	}
}

void MenuMidiPc::write_map(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/midi_map.ego", FA_WRITE);
	f_write(&fsys, pc_param, 256, &br);
	f_close(&fsys);
}

void MenuMidiPc::read_map(void)
{
	FIL fsys;
	UINT br;
	f_open(&fsys, "/midi_map.ego", FA_OPEN_ALWAYS | FA_READ);
	f_read(&fsys, pc_param, 256, &br);
	f_close(&fsys);
}
