#include <avr/eeprom.h>
#include "global.h"
#include "checksum.h"


void eeprom_init();
void eeprom_save();
void eeprom_read();
void eeprom_check_ccc();