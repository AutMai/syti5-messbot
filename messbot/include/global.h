#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>
#include "uart.h"
#include "lcd.h"

#ifndef global_def
#define global_def

extern uint8_t status;
#define ON  2
#define TRANSMIT_ON 3
#define TRANSMIT 4

typedef struct _versionControl {
    uint8_t size;
    uint8_t MajorVersion;
    uint8_t MinorVersion;
    uint8_t HeaderLength;
    uint8_t TransmitInterval;
    uint8_t CRC;
} versionControl;

extern versionControl myversionControl;

#endif