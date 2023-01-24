#include "adc.h"
#include "custom-lcd.h"
#include "button.h"

extern uint8_t transmitSensors;
#define ALL 0
#define SWITCH 1
#define PHOTO_RESISTOR 2

void transmitAll();
void transmitSwitch();
void transmitLight();

void transmit();
void transmit_off();
void transmit_on();