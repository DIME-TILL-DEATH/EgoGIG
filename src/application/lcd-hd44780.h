#ifndef LCD_HD44780_H_
#define LCD_HD44780_H_

#include "appdefs.h"

//************************************************************************//
//			Конфигурация порта
//	используемый порт
#define lcd44780_port				GPIOC
//	используемые выводы
#define lcd44780_pin_RS				GPIO1
#define lcd44780_pin_E				GPIO2

#define lcd44780_RS_1 gpio_set(lcd44780_port, lcd44780_pin_RS);
#define lcd44780_E_1  gpio_set(lcd44780_port, lcd44780_pin_E);
#define lcd44780_RS_0 gpio_clear(lcd44780_port, lcd44780_pin_RS);
#define lcd44780_E_0  gpio_clear(lcd44780_port, lcd44780_pin_E);
//************************************************************************//

// Custom symbols:
#define SYMBOL_ARROW_UP 0
#define SYMBOL_ARROW_DOWN 1
#define SYMBOL_NEXT_MARK 2


void lcd44780_ClearLCD(void);
void lcd44780_SetLCDPosition(uint8_t x, uint8_t y);
void lcd44780_ShowChar(uint8_t x, uint8_t y, uint8_t c);
void lcd44780_ShowStr(uint8_t x, uint8_t y, uint8_t *s);
void lcd44780_Clear_str(uint8_t x, uint8_t y, uint8_t cont);
void lcd44780_ShowNum(uint8_t x, uint8_t y, uint8_t num);
void lcd44780_ShowNote(uint8_t x, uint8_t y, uint8_t note);

void lcd44780_init_pins(void);
void lcd44780_init(void);



void disp_init(void);

extern uint8_t tim_lin[];
inline void sec_print(uint32_t val)
{
	tim_lin[7] = val % 10 + 48;
	tim_lin[5] = (val / 10) % 10 + 48;
	tim_lin[4] = (val / 100) % 6 + 48;
	tim_lin[2] = (val / 600) % 10 + 48;
	tim_lin[1] = (val / 6000) % 10 + 48;
	tim_lin[0] = (val / 60000) % 10 + 48;
	lcd44780_ShowChar(3, 1, 32);
	lcd44780_ShowChar(11, 1, 32);
	lcd44780_ShowStr(4, 1, (uint8_t*) tim_lin);
}

void oem2winstar(emb_string &str);

#endif
