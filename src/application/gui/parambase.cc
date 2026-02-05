#include "parambase.h"

#include "display.h"
#include "libopencm3/stm32/timer.h"

ParamBase::ParamBase(GuiParamType paramType, const char* name, void* paramValuePtr)
{
	m_type = paramType;
	m_name = name;
	m_valuePtr = (uint8_t*)paramValuePtr;
}

void ParamBase::setDisplayPosition(uint8_t xCoord)
{
	m_xDisplayPosition = xCoord;
}

void ParamBase::setScaling(uint8_t stepSize, int32_t offset)
{
	m_stepSize = stepSize;
	m_offset = offset;
}

void ParamBase::setBounds(int32_t minBound, int32_t maxBound)
{
	m_minValue = minBound;
	m_maxValue = maxBound;
}

const char* ParamBase::name()
{
	if(m_disabled) return " -- ";
	else return m_name;
}

uint8_t ParamBase::nameLength()
{
	if(m_disabled) return strlen(" -- ");
	else return strlen(m_name);
}

uint32_t ParamBase::value() const
{
	if(!m_valuePtr) return 0;
	else
	{
		uint32_t fullValue = 0;
		memcpy(&fullValue, m_valuePtr, m_byteSize);
		return fullValue + m_offset;
	}
}

void ParamBase::setInverse(bool isInverse)
{
	m_inverse = isInverse;
}

void ParamBase::increaseParam()
{
	if(!m_valuePtr) return;

	int32_t data = 0;
	if(m_byteSize>1) memcpy(&data, m_valuePtr, m_byteSize);
	else data = (int8_t)(*m_valuePtr);

	if(data < m_maxValue)
	{
		if(m_type != GUI_PARAMETER_NUM)
			encoderSpeedIncrease();
		else
			*m_valuePtr += m_stepSize;
	}
	else
	{
		*m_valuePtr = m_maxValue;
	}
}

void ParamBase::decreaseParam()
{
	if(!m_valuePtr) return;

	int32_t data = 0;
	if(m_byteSize>1) memcpy(&data, m_valuePtr, m_byteSize);
	else data = (int8_t)(*m_valuePtr);

	if(data > m_minValue)
	{
		if(m_type != GUI_PARAMETER_NUM)
			encoderSpeedDecrease();
		else
			*m_valuePtr -= m_stepSize;
	}
	else
	{
		*m_valuePtr = m_minValue;
	}
}

void ParamBase::printParam(uint8_t yDisplayPosition)
{
	if(!m_valuePtr) return;

	if(m_disabled)
	{
		DisplayTask->Clear_str(m_xDisplayPosition, yDisplayPosition, 3);
		return;
	}

	switch(m_type)
	{
		case ParamBase::GUI_PARAMETER_NUM:
			DisplayTask->NumOut(m_xDisplayPosition, yDisplayPosition, *m_valuePtr + m_offset);
			break;
		case ParamBase::GUI_PARAMETER_NOTE:
			DisplayTask->NoteOut(m_xDisplayPosition, yDisplayPosition, *m_valuePtr + m_offset);
			break;
		default: break;
	}
}

void ParamBase::encoderSpeedIncrease()
{
	TIM6_CR1 &= ~TIM_CR1_CEN; // Disable

	int32_t data = 0;
	if(m_byteSize>1) memcpy(&data, m_valuePtr, m_byteSize);
	else data = (int8_t)(*m_valuePtr);

	if(timer_get_flag(TIM6,TIM_FLAG_UPDATE))
	{
		data += 1;
	}
	else
	{
		if(timer_get_counter(TIM6) > 0x3fff)
		{
			data += 1;
		}
		else
		{
			uint8_t curStep;

			if(timer_get_counter(TIM6) > 0x1fff) curStep = 2 * m_stepSize;
			else
			{
				if(timer_get_counter(TIM6) > 0xfff) curStep = 4 * m_stepSize;
				else
				{
					if(timer_get_counter(TIM6) > 0x7ff) curStep = 8 * m_stepSize;
					else curStep = 16 * m_stepSize;
				}
			}

			if(data < (m_maxValue - curStep - 1)) data += curStep;
			else data += 1;
		}
	}

	memcpy(m_valuePtr, &data, m_byteSize);

	timer_set_counter(TIM6, 0);
	timer_clear_flag(TIM6, TIM_FLAG_UPDATE);
	TIM6_CR1 |= TIM_CR1_CEN;
}

void ParamBase::encoderSpeedDecrease()
{
	TIM6_CR1 &= ~TIM_CR1_CEN; // Disable

	int32_t data = 0;
	if(m_byteSize>1) memcpy(&data, m_valuePtr, m_byteSize);
	else data = (int8_t)(*m_valuePtr);

	if(timer_get_flag(TIM6,TIM_FLAG_UPDATE))
	{
		data -= 1;
	}
	else
	{
		if(timer_get_counter(TIM6) > 0x3fff)
		{
			data -= 1;
		}
		else
		{
			uint8_t curStep;

			if(timer_get_counter(TIM6) > 0x1fff) curStep = 2 * m_stepSize;
			else
			{
				if(timer_get_counter(TIM6) > 0xfff) curStep = 4 * m_stepSize;
				else
				{
					if(timer_get_counter(TIM6) > 0x7ff) curStep = 8 * m_stepSize;
					else curStep = 16 * m_stepSize;
				}
			}

			if(data > (m_minValue + curStep - 1)) data -= curStep;
			else data -= 1;
		}
	}

	memcpy(m_valuePtr, &data, m_byteSize);

	timer_set_counter(TIM6, 0);
	timer_clear_flag(TIM6, TIM_FLAG_UPDATE);
	TIM6_CR1 |= TIM_CR1_CEN;
}

int16_t ParamBase::encSpeedInc(int16_t data, int16_t max, uint8_t stepSize)
{
	TIM6_CR1 &= ~TIM_CR1_CEN; // Disable

	if(timer_get_flag(TIM6, TIM_FLAG_UPDATE))
	{
		data += stepSize;
	}
	else
	{
		if(timer_get_counter(TIM6) > 0x3fff)
			data += stepSize;
		else
		{
			if(timer_get_counter(TIM6) > 0x1fff)
			{
				if(data < (max - 1)) data += 2 * stepSize;
				else data += stepSize;
			}
			else
			{
				if(timer_get_counter(TIM6) > 0xfff)
				{
					if(data < (max - 3)) data += 4 * stepSize;
					else data += 1 * stepSize;
				}
				else
				{
					if(timer_get_counter(TIM6) > 0x7ff)
					{
						if(data < (max - 7)) data += 8 * stepSize;
						else data += 1 * stepSize;
					}
					else
					{
						if(data < (max - 49)) data += 50 * stepSize;
						else data += 1 * stepSize;
					}
				}
			}
		}
	}

	timer_set_counter(TIM6, 0);
	timer_clear_flag(TIM6, TIM_FLAG_UPDATE);
	TIM6_CR1 |= TIM_CR1_CEN;
	return data;
}

int16_t ParamBase::encSpeedDec(int16_t data, int16_t min, uint8_t stepSize)
{
	TIM6_CR1 &= ~TIM_CR1_CEN; // Disable

	if(timer_get_flag(TIM6, TIM_FLAG_UPDATE))
	{
		data -= stepSize;
	}
	else
	{
		if(timer_get_counter(TIM6) > 0x3fff)
			data -= stepSize;
		else
		{
			if(timer_get_counter(TIM6) > 0x1fff)
			{
				if(data > (min + 1)) data -= 2 * stepSize;
				else data -= stepSize;
			}
			else
			{
				if(timer_get_counter(TIM6) > 0xfff)
				{
					if(data > (min + 3)) data -= 4 * stepSize;
					else data -= stepSize;
				}
				else
				{
					if(timer_get_counter(TIM6) > 0x7ff)
					{
						if(data > (min + 7)) data -= 8 * stepSize;
						else data -= stepSize;
					}
					else
					{
						if(data > (min + 49)) data -= 50 * stepSize;
						else data -= stepSize;
					}
				}
			}
		}
	}

	timer_set_counter(TIM6, 0);
	timer_clear_flag(TIM6, TIM_FLAG_UPDATE);
	TIM6_CR1 |= TIM_CR1_CEN;
	return data;
}
