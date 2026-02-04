#ifndef SRC_APPLICATION_GUI_LEDS_H_
#define SRC_APPLICATION_GUI_LEDS_H_

#include "init.h"

namespace Leds
{

void greenOn();
void greenOff();

void redOn();
void redOff();

void digitPoint1On();
void digitPoint1Off();

void digitPoint2On();
void digitPoint2Off();

void lockOn();
void lockOff();

void menuGreenOn();
void menuRedOn();

void digit(uint8_t num);


void requestBlinking();
void requestLed1Blinking();
void requestLed2Blinking();
void processBlinking();
}


#endif /* SRC_APPLICATION_GUI_LEDS_H_ */
