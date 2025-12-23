#ifndef SRC_APPLICATION_GUI_PARAMBASE_H_
#define SRC_APPLICATION_GUI_PARAMBASE_H_

#include "appdefs.h"

class ParamBase
{
public:
	enum GuiParamType
	{
		GUI_PARAMETER_DUMMY,
		GUI_PARAMETER_NUM,
		GUI_PARAMETER_NOTE,
		GUI_PARAMETER_LIST,
		GUI_PARAMETER_SUBMENU,
		GUI_PARAMETER_CUSTOM,
		GUI_PARAMETER_REAL,
		GUI_PARAMETER_STRING_OUT
	};

	// Сделан только для RealParam
	enum TIndicatorType{
		IndNone,
		IndBarFilled,
		IndBarTransparent,
		IndMix,
		IndPan
	};

	ParamBase(GuiParamType paramType, const char* name, void* paramValuePtr);
	virtual ~ParamBase() {};

	GuiParamType type() const {return m_type;};
	virtual const char* name();
	virtual uint8_t nameLength();

	void setValuePtr(uint8_t* valuePtr) {m_valuePtr = valuePtr;};
	uint8_t* valuePtr() const {return m_valuePtr;};

	virtual uint32_t value() const;
	virtual void increaseParam();
	virtual void decreaseParam();

	virtual void printParam(uint8_t yPos);

	bool disabled() {return m_disabled;};
	void setDisabled(bool disabled) {m_disabled = disabled;};

	void setBounds(int32_t minBound, int32_t maxBound);
	int32_t minValue() const {return m_minValue;};
	int32_t maxValue() const {return m_maxValue;};

	void setScaling(uint8_t stepSize, int32_t offset);
	int32_t offset() const {return m_offset;};
	uint8_t stepSize() {return m_stepSize;};

	void setInverse(bool isInverse);
	bool inverse() {return m_inverse;}

	void setDisplayPosition(uint8_t xCoord);
	uint8_t xDisplayPosition() const {return m_xDisplayPosition;};

	static int16_t encSpeedInc(int16_t data, int16_t max, uint8_t stepSize = 1);
	static int16_t encSpeedDec(int16_t data, int16_t min, uint8_t stepSize = 1);

protected:
	const char* m_name;

	TIndicatorType m_indicatorType{IndNone};

	bool m_disabled{false};
	bool m_inverse{false};

	uint8_t m_xDisplayPosition{0};

	uint8_t m_byteSize{1};
	int32_t m_offset{0};
	uint8_t m_stepSize{1};

	GuiParamType m_type{GUI_PARAMETER_DUMMY};

	uint8_t* m_valuePtr;

	int32_t m_minValue{0};
	int32_t m_maxValue{127};

	void encoderSpeedIncrease();
	void encoderSpeedDecrease();
};





#endif /* SRC_APPLICATION_GUI_PARAMBASE_H_ */
