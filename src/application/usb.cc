#include "usb.h"
#include "console.h"

#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"

#include <libopencm3/usb/msc.h>
#include "libopencm3/cm3/scb.h"
#include "libopencm3/stm32/otg_fs.h"

#include "sd_storage.h"
#include "libopencm3/cm3/nvic.h"
#include "libopencm3/cm3/scb.h"

TUsbTask *UsbTask;

TUsbTask::TUsbTask(const char *name, const int stack_size, const int priority) :
		TTask(name, stack_size, priority, false)
{

}

static const struct usb_device_descriptor dev_descr =
{ .bLength =
USB_DT_DEVICE_SIZE, .bDescriptorType = USB_DT_DEVICE, .bcdUSB = 0x0110,
		.bDeviceClass = 0, .bDeviceSubClass = 0, .bDeviceProtocol = 0,
		.bMaxPacketSize0 = 64, .idVendor = USBD_VID, .idProduct = USBD_PID,
		.bcdDevice = 0x0200, .iManufacturer = 1, .iProduct = 2, .iSerialNumber =
				3, .bNumConfigurations = 1, };

static const struct usb_endpoint_descriptor msc_endp[] =
{
{ .bLength =
USB_DT_ENDPOINT_SIZE, .bDescriptorType = USB_DT_ENDPOINT, .bEndpointAddress =
		0x01, .bmAttributes = USB_ENDPOINT_ATTR_BULK, .wMaxPacketSize = 64,
		.bInterval = 0, .extra = NULL, .extralen = 0 },
{ .bLength = USB_DT_ENDPOINT_SIZE, .bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x82, .bmAttributes = USB_ENDPOINT_ATTR_BULK,
		.wMaxPacketSize = 64, .bInterval = 0, .extra = NULL, .extralen = 0 } };

static const struct usb_interface_descriptor msc_iface[] =
{
{ .bLength =
USB_DT_INTERFACE_SIZE, .bDescriptorType = USB_DT_INTERFACE, .bInterfaceNumber =
		0, .bAlternateSetting = 0, .bNumEndpoints = 2, .bInterfaceClass =
		USB_CLASS_MSC, .bInterfaceSubClass =
USB_MSC_SUBCLASS_SCSI, .bInterfaceProtocol =
USB_MSC_PROTOCOL_BBB, .iInterface = 0, .endpoint = msc_endp, .extra = NULL,
		.extralen = 0 } };

static const struct usb_config_descriptor::usb_interface ifaces[] =
{
{ .cur_altsetting = NULL, .num_altsetting = 1, .iface_assoc = NULL,
		.altsetting = msc_iface, } };

static const struct usb_config_descriptor config_descr =
{ .bLength =
USB_DT_CONFIGURATION_SIZE, .bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = 0, .bNumInterfaces = 1, .bConfigurationValue = 1,
		.iConfiguration = 0, .bmAttributes = 0x80, .bMaxPower = 0x32,
		.interface = ifaces, };

static const char *usb_strings[] =
{ "AMT", "SD multiplayer firmware", "SD multiplayer", };

static usbd_device *msc_dev;
/* Buffer to be used for control requests. */
static uint8_t usbd_control_buffer[128];

static int read_block(uint64_t lba, uint8_t *copy_to)
{
	return sd_storage_read(copy_to, lba, 1);
}
static int write_block(uint64_t lba, const uint8_t *copy_from)
{
	return sd_storage_write(copy_from, lba, 1);
}

void TUsbTask::Code()
{
	rcc_periph_clock_enable (RCC_GPIOA);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
			GPIO9 | GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);

	msc_dev = usbd_init(&otgfs_usb_driver, &dev_descr, &config_descr,
			usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));

	uint32_t sectors;
	sd_storage_initialize();
	sd_storage_ioctl(GET_SECTOR_COUNT, &sectors);
	usb_msc_init(msc_dev, 0x82, 64, 0x01, 64, "VendorID", "ProductID", "0.00",
			sectors, read_block, write_block);

	for (;;)
	{
		if (!(GPIOA_IDR & GPIO9))
			scb_reset_system();
		else
			usbd_poll(msc_dev);
	}
}
