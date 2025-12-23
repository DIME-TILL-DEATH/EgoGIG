#ifndef SRC_APPLICATION_GUI_PARAMSTRINGLIST_H_
#define SRC_APPLICATION_GUI_PARAMSTRINGLIST_H_

#include "parambase.h"

class ParamStringList : public ParamBase
{
public:
	ParamStringList(const char* name, uint8_t* paramValuePtr,
			std::initializer_list<const char*> stringList, uint8_t maxStringLength);
	~ParamStringList();

	uint8_t* getString();
	uint8_t* getString(uint8_t stringNum);

	void setAffectedParamsList(ParamBase** affectedParamList, uint8_t affectedParamCount);
	void setDisableMask(uint8_t stringNum, std::initializer_list<uint8_t> disableMask);
	uint8_t* getDisableMask();

	void increaseParam() override;
	void decreaseParam() override;

	void printParam(uint8_t yPos) override;

private:
	char** m_strings{nullptr};
	uint8_t m_stringCount;
	uint8_t m_maxStringLength;

	static constexpr uint8_t maxAffectedParamsCount = 16;
	uint8_t** m_disableMask{nullptr};
	ParamBase* m_affectedParamsList[maxAffectedParamsCount];
	uint8_t m_affectedParamsCount{0};
};


#endif /* SRC_APPLICATION_GUI_PARAMSTRINGLIST_H_ */
