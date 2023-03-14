#include "adc.h"
#include "custom-lcd.h"
#include "button.h"

#define UART_STX 91
#define UART_ETX 93
#define UART_ACK 6
#define UART_NAK 21

extern uint8_t transmitSensors;
#define ALL 0
#define SWITCH 1
#define PHOTO_RESISTOR 2

void transmitAll();
void transmitSwitch();
void transmitLight();

void transmitNAK();
void transmitACK();

void transmit();
void transmit_off();
void transmit_on();