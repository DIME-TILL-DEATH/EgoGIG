#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "appdefs.h"
#include "lcd-hd44780.h"

#define FILE_NAME_LENGTH 64

class TDisplayTask: public TTask
{
public:
	inline TDisplayTask(const char *name, const int stack_size,
			const int priority) :
			TTask(name, stack_size, priority, false)
	{

	}
	typedef enum
	{
		fnt12x13 = 0, fnt33x30, fntSystem, fntCount
	} TFontName;
	typedef enum
	{
		fnsBlack = 0, fnsWhite, fnsBlackUnderline, fnsCount
	} TFontState;

	typedef struct
	{
		TFontName name;
		uint8_t curs;
	} TFont;

	typedef struct
	{
		uint8_t x;
		uint8_t y;
	} TPos;

	typedef enum
	{
		dcWriteReg = 0,
		dcClear,
		dcClear_str,
		dcSymbolOut,
		dcStringOut,
		dcNumOut,
		dcNoteOut,
		dcCurPos,
		dcLed_Write,
		dcSec_Print,
		dcCount
	} TCommand;

	typedef struct
	{
		uint8_t x;
		uint8_t y;
	} TCurPos;
	typedef struct
	{
		uint32_t data;
	} TSec_PrintParam;
	typedef struct
	{
		uint8_t x;
		uint8_t y;
		uint8_t count;
	} TClear_strParams;
	typedef struct
	{
		uint8_t x;
		uint8_t y;
		uint8_t string[FILE_NAME_LENGTH];
	} TStringOutParams;
	typedef struct
	{
		uint8_t x;
		uint8_t y;
		uint8_t value;
	} TValueOutParams;
	typedef struct
	{
		TCommand cmd;
		union
		{
			TClear_strParams Clear_strParams;
			TValueOutParams ValueOutParams;
			TStringOutParams StringOutParams;
			TCurPos CurPos;
			TSec_PrintParam Sec_PrintParam;
		};
	} TDisplayCmd;

	// send command from code/tasks
	inline TQueue::TQueueSendResult Command(TDisplayCmd *cmd)
	{
		if (cortex_isr_num())
		{
			// send comand from ISR
			BaseType_t HigherPriorityTaskWoken;
			TQueue::TQueueSendResult result;
			result = queue->SendToBackFromISR(cmd, &HigherPriorityTaskWoken);
			if (HigherPriorityTaskWoken)
				TScheduler::Yeld();
			return result;
		}
		else
		{
			return queue->SendToBack(cmd, 0);
		}
	}

	void Clear();
	void Clear_str(uint8_t x, uint8_t y, uint8_t count);
	void SymbolOut(uint8_t x, uint8_t y, uint8_t symbol);
	void StringOut(uint8_t x, uint8_t y, uint8_t *string);
	void NumOut(uint8_t x, uint8_t y, uint8_t num);
	void NoteOut(uint8_t x, uint8_t y, uint8_t note);
	void CurPos(uint8_t x, uint8_t y);
	void Sec_Print(uint32_t val);
private:
	void Code();
	TQueue *queue;

};

extern TDisplayTask *DisplayTask;

#endif /*__DISPLAY_H__*/
