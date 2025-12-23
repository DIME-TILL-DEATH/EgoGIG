#ifndef __USB_H__
#define __USB_H__

#include "appdefs.h"

#include "console.h"

#include "libopencm3/usb/usbd.h"

extern const TConsoleTask::readline_io_t cdc_readline_io;

class TUsbTask: public TTask
{
public:
	TUsbTask(const char *name, const int stack_size, const int priority);
	virtual ~TUsbTask()
	{
	}
	;
	inline usbd_device* UsbDev()
	{
		return usbd_dev;
	}

private:
	void Code();
	usbd_device *usbd_dev;
};

extern TUsbTask *UsbTask;

#endif /*__USB_H__*/
