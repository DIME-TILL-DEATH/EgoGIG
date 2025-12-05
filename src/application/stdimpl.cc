#include "appdefs.h"
#include "mmgr.h"

extern "C" void _init(void)
{
	heap_init();
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t *pxTask,
		char *pcTaskName)
{
	while (pcTaskName)
		NOP();
}

extern "C" void vApplicationTickHook()
{
	NOP();
}

extern "C" void vApplicationIdleHook()
{
	NOP();
}
//---------------------------------------------------------------------------
// reimplement HAL call of  HAL_GetTick()
extern "C" uint32_t HAL_GetTick(void)
{
	return xTaskGetTickCount();
}
//---------------------------------------------------------------------------
// reimplement HAL call of  HAL_Delay()
extern "C" void HAL_Delay(volatile uint32_t Delay)
{
	vTaskDelay(Delay);
}
//---------------------------------------------------------------------------
