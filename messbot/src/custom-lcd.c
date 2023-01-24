#include "custom-lcd.h"

void lcd_say(char *text) {
    lcd_clrscr();
    lcd_home();
    lcd_puts(text);
    _delay_ms(1000);
    lcd_clrscr();
}