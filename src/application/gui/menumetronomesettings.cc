#include "menumetronomesettings.h"

#include "ff.h"
#include "init.h"
#include "leds.h"

#include "paramstringlist.h"
#include "paramsubmenu.h"

#include "display.h"

#include "metronome.h"

MenuMetronomeSettings::MenuMetronomeSettings(AbstractMenu* parent)
	:MenuParamList(parent, MENU_METRONOME_SETTINGS)
{
	const uint8_t paramNum = 2;
	ParamBase* params[paramNum];

	params[0] = new ParamStringList("Out  ", &sys_param[metronome_out], {"4", "3", "2", "1",}, 2);
	params[0]->setDisplayPosition(6);
	params[0]->setBounds(0, 3);
	params[0]->setInverse(true);

	params[1] = new ParamStringList("Sound", &sys_param[metronome_type], {"Default  ", "Ableton  ", "Cubase   ", "MPC      ", "Pro Tools", "Wood     "}, 9);
	params[1]->setDisplayPosition(6);

	setParams(params, paramNum);
}

void MenuMetronomeSettings::task()
{
	if(m_paramsCount == 1) m_encoderKnobSelected = true;

	if(!m_encoderKnobSelected)
	{
		if(tim5_fl)
			DisplayTask->Clear_str(0, m_currentParamNum % paramsOnPage, m_paramsList[m_currentParamNum]->nameLength());
		else
			DisplayTask->StringOut(0, m_currentParamNum % paramsOnPage, (uint8_t*)(m_paramsList[m_currentParamNum]->name()));
	}
	else
	{
		if(tim5_fl)
			DisplayTask->Clear_str(m_paramsList[m_currentParamNum]->nameLength(), m_currentParamNum % paramsOnPage, 16 - m_paramsList[m_currentParamNum]->nameLength() - 1);
		else
			m_paramsList[m_currentParamNum]->printParam(m_currentParamNum % paramsOnPage);

		Metronome::setMetronomeType(static_cast<Metronome::MetronomeType>(sys_param[metronome_type]));
	}
}

void MenuMetronomeSettings::keyEsc()
{
	MenuSystem::write_sys();
	m_parentMenu->returnFromChildMenu();
}

void MenuMetronomeSettings::keyStop()
{
	player.stopMetronome();

	Leds::redOn();
	Leds::greenOff();
}

void MenuMetronomeSettings::keyStart()
{
	player.starMetronome();

	Leds::redOff();
	Leds::greenOn();
}
