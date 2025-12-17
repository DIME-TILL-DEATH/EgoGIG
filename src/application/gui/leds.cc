#include "leds.h"

void Leds::greenOn()
{
	key_reg_out[0] |= 0x02;
}

void Leds::greenOff()
{
	key_reg_out[0] &= ~0x02;
}

void Leds::redOn()
{
	key_reg_out[0] |= 0x80;
}
void Leds::redOff()
{
	key_reg_out[0] &= ~0x80;
}

void Leds::digitPoint1On()
{
	key_reg_out[1] &= ~(1 << 7);
}

void Leds::digitPoint1Off()
{
	key_reg_out[1] |= 1 << 7;
}

void Leds::digitPoint2On()
{
	key_reg_out[1] &= ~(1 << 15);
}
void Leds::digitPoint2Off()
{
	key_reg_out[1] |= 1 << 15;
}

void Leds::lockOn()
{
	key_reg_out[0] |= 4;
}
void Leds::lockOff()
{
	key_reg_out[0] &= ~4;
}

void Leds::menuGreenOn()
{
	key_reg_out[0] |= 0x10;
	key_reg_out[0] &= ~0x08;
}
void Leds::menuRedOn()
{
	key_reg_out[0] &= ~0x10;
	key_reg_out[0] |= 0x08;
}

void Leds::digit(uint8_t num)
{
	const uint8_t led_sym[10] =
	{ 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90 };

	uint8_t temp = num + 1;
	key_reg_out[1] = (led_sym[temp / 10]) | ((led_sym[temp % 10]) << 8);
}
