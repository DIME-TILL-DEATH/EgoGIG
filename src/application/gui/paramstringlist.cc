#include "paramstringlist.h"

#include "display.h"

ParamStringList::ParamStringList(const char* name, uint8_t* paramValuePtr,
		std::initializer_list<const char*>stringList, uint8_t maxStringLength)
			:ParamBase(ParamBase::GUI_PARAMETER_LIST, name, paramValuePtr)
{
	m_stringCount = stringList.size();
	m_maxStringLength = maxStringLength;

	m_strings = new char*[m_stringCount];
	for(int i = 0; i<m_stringCount; i++)
		m_strings[i] = new char[m_maxStringLength];

	uint8_t strCounter = 0;
	for(auto strIter = stringList.begin(); strIter != stringList.end(); ++strIter)
	{
		strcpy(m_strings[strCounter], *strIter);
		strCounter++;
	}

	m_maxValue = m_stringCount - 1;

	if(*paramValuePtr < m_minValue) *paramValuePtr = m_minValue;
	if(*paramValuePtr > m_maxValue) *paramValuePtr = m_maxValue;
}

ParamStringList::~ParamStringList()
{
	if(m_strings)
	{
		for(int i = 0; i<m_stringCount; i++)
				delete[] m_strings[i];
	}

	if(m_disableMask)
	{
		for(int i = 0; i<m_stringCount; i++)
				delete[] m_disableMask[i];
	}
}

uint8_t* ParamStringList::getString(uint8_t stringNum)
{
	return (uint8_t*)m_strings[stringNum];
}

void ParamStringList::setAffectedParamsList(ParamBase** affectedParamList, uint8_t affectedParamCount)
{
	for(int i=0; i<affectedParamCount; i++)
	{
		m_affectedParamsList[i] = affectedParamList[i];
	}

	m_affectedParamsCount = affectedParamCount;
}

void ParamStringList::setDisableMask(uint8_t stringNum, std::initializer_list<uint8_t> disableMask)
{
	if(!m_disableMask)
	{
		m_disableMask = new uint8_t*[m_stringCount];

		for(int i = 0; i<m_stringCount; i++)
		{
			m_disableMask[i] = new uint8_t[maxAffectedParamsCount]; // max affected params count
			memset(m_disableMask[i], 0, maxAffectedParamsCount);
		}
	}

	uint8_t i = 0;
	for (auto val = disableMask.begin(); val != disableMask.end() && i!=16; ++val)
	{
		m_disableMask[stringNum][i] = *val;
		i++;
	}
}

uint8_t* ParamStringList::getDisableMask()
{
	if(!m_disableMask) return nullptr;

	return m_disableMask[value()];
}

void ParamStringList::increaseParam()
{
	if(!m_valuePtr) return;

	if(*m_valuePtr < m_maxValue)
	{
		*m_valuePtr += 1;

		for(int i=0; i<m_affectedParamsCount; i++)
		{
			m_affectedParamsList[i]->setDisabled(m_disableMask[*m_valuePtr][i]);
		}
	}
	else
	{
		*m_valuePtr = m_maxValue;
	}
}

void ParamStringList::decreaseParam()
{
	if(!m_valuePtr) return;

	if(*m_valuePtr > m_minValue)
	{
		*m_valuePtr -= 1;

		for(int i=0; i<m_affectedParamsCount; i++)
		{
			m_affectedParamsList[i]->setDisabled(m_disableMask[*m_valuePtr][i]);
		}
	}
	else
	{
		*m_valuePtr = m_minValue;
	}
}

void ParamStringList::printParam(uint8_t yDisplayPosition)
{
	if(m_disabled) return;

	if(*m_valuePtr < m_stringCount)
		DisplayTask->StringOut(m_xDisplayPosition, yDisplayPosition, getString(*m_valuePtr));
}
