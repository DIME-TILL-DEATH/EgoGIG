#include "appdefs.h"
#include "display.h"

TDisplayTask *DisplayTask;

void TDisplayTask::Code()
{
	TDisplayCmd cmd;

	disp_init();

	queue = new TQueue(40, sizeof(TDisplayCmd));
	if (!queue)
	{
		Suspend();
	}
	if (!queue->IsCreated())
	{
		Suspend();
	}
	while (1)
	{
		queue->Receive(&cmd, portMAX_DELAY);

		// perform comand
		switch (cmd.cmd)
		{
		case dcClear:
			lcd44780_ClearLCD();
			break;
		case dcSymbolOut:
			lcd44780_ShowChar(cmd.SymbolOutParams.x, cmd.SymbolOutParams.y,
					cmd.SymbolOutParams.symbol);
			break;
		case dcStringOut:
			lcd44780_ShowStr(cmd.StringOutParams.x, cmd.StringOutParams.y,
					cmd.StringOutParams.string);
			break;
		case dcClear_str:
			lcd44780_Clear_str(cmd.Clear_strParams.x, cmd.Clear_strParams.y,
					cmd.Clear_strParams.count);
			break;
		case dcCurPos:
			lcd44780_SetLCDPosition(cmd.CurPos.x, cmd.CurPos.y);
			break;
		case dcSec_Print:
			sec_print(cmd.Sec_PrintParam.data);
			break;
		default:
			break;
		}
	}

}

void TDisplayTask::Clear()
{
	TDisplayCmd cmd;
	cmd.cmd = dcClear;
	Command(&cmd);
}
void TDisplayTask::SymbolOut(uint8_t x, uint8_t y, uint8_t symbol)
{
	TDisplayCmd cmd;
	cmd.cmd = dcSymbolOut;
	cmd.SymbolOutParams.x = x;
	cmd.SymbolOutParams.y = y;
	cmd.SymbolOutParams.symbol = symbol;
	Command(&cmd);
}
void TDisplayTask::StringOut(uint8_t x, uint8_t y, uint8_t *string)
{
	TDisplayCmd cmd;
	cmd.cmd = dcStringOut;
	cmd.StringOutParams.x = x;
	cmd.StringOutParams.y = y;
	strncpy((char*) cmd.StringOutParams.string, (const char*) string,
			FILE_NAME_LENGTH);
	Command(&cmd);
}
void TDisplayTask::Clear_str(uint8_t x, uint8_t y, uint8_t count)
{
	TDisplayCmd cmd;
	cmd.cmd = dcClear_str;
	cmd.Clear_strParams.x = x;
	cmd.Clear_strParams.y = y;
	cmd.Clear_strParams.count = count;
	Command(&cmd);
}
void TDisplayTask::CurPos(uint8_t x, uint8_t y)
{
	TDisplayCmd cmd;
	cmd.cmd = dcCurPos;
	cmd.CurPos.x = x;
	cmd.CurPos.y = y;
	Command(&cmd);
}
void TDisplayTask::Sec_Print(uint32_t val)
{
	TDisplayCmd cmd;
	cmd.cmd = dcSec_Print;
	cmd.Sec_PrintParam.data = val;
	Command(&cmd);
}

