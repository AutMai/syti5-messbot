#include "eeprom.h"

extern versionControl myversionControl;

void eeprom_init()
{
    eeprom_read();
    eeprom_check_ccc();
}

void eeprom_save()
{
    myversionControl.CRC = crc_8((const unsigned char *)&myversionControl, myversionControl.size - 1);
    eeprom_update_block(&myversionControl, 0, myversionControl.size); // was soll gespeichert werden, wo ist die länge der daten
}

void eeprom_read()
{
    eeprom_read_block(&myversionControl, 0, myversionControl.size); // was soll gespeichert werden, wo ist die länge der daten
}

void eeprom_check_ccc()
{
    myversionControl.CRC = crc_8((const unsigned char*)&myversionControl, myversionControl.size - 1);

    if (crc_8((const unsigned char *)&myversionControl, myversionControl.size) == 0)
    {

        lcd_puts("crc check successfull");
        //_delay_ms(2000);
    }
    else
    {
        lcd_puts("crc check failed");
        //_delay_ms(2000);
    }
}