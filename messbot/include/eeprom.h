#include <avr/eeprom.h>
#include "lcd.h"
#include "checksum.h"
#include "global.h"

void eeprom_init();
void eeprom_save();
void eeprom_read();
void eeprom_check_ccc();