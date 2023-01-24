#include "global.h"
#include "eeprom.h"
#include "adc.h"
#include "timer.h"
#include "custom-uart.h"
#include "custom-lcd.h"
#include "button.h"
#include "lcd.h"

uint8_t status = 0;
versionControl myversionControl = {
        sizeof(myversionControl),
        1,
        23,
        12,
        1,
        0};

void power_on() {
    lcd_say("Hello");
    transmit_on();
    status |= (1 << ON);
}

void power_off() {
    lcd_say("Goodbye");
    transmit_off();
    status &= ~(1 << ON);
}

char *getLightLevel() {
    char buffer[10];
    float volts = get_adc_volt();
    char message[64];
    snprintf(message, sizeof(message), "light: %s", dtostrf(volts, 0, 2, buffer));
    return message;
}

char *getDigitalInput() {
    char *digitalInput = malloc(3);
    digitalInput = bit_is_set(buttons, DIGITAL_INPUT_BTN) ? "on " : "off";
    return digitalInput;
}

int main() {
    DDRD |= (1 << PD3);
    sei();
    eeprom_init();
    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));
    lcd_init(LCD_DISP_ON);
    adc_init();
    button_init();
    timer_init();
    lcd_clrscr();

    power_on();

    while (1) {
        lcd_home();
        lcd_puts("OFF");
        if (bit_is_set(buttons, ON_OFF_BTN)) {
            power_on();
            buttons &= ~(1 << ON_OFF_BTN);
        }

        while (bit_is_set(status, ON)) {
            lcd_home();
            lcd_puts("RUNNING...");
            lcd_gotoxy(0, 1);
            lcd_puts(getDigitalInput());

            handleUartInput();

            if (bit_is_set(status, TRANSMIT)) {
                transmit();
                status &= ~(1 << TRANSMIT);
            }

            if (bit_is_set(buttons, ON_OFF_BTN)) {
                power_off();
                buttons &= ~(1 << ON_OFF_BTN);
            }
            if (bit_is_set(buttons, TRANSMIT_INTERVAL_BTN)) {
                // read transmitAll interval from last two bits of status byte
                uint8_t transmit_interval = myversionControl.TransmitInterval;
                if ((transmit_interval == 3) &&
                    bit_is_set(status, TRANSMIT_ON)) { // if transmit interval is 3 and transmitAll is on
                    transmit_off();
                } else if (bit_is_clear(status, TRANSMIT_ON)) { // if transmit interval is 3 and transmitAll is off
                    transmit_on();
                    transmit_interval = 0;
                } else {
                    transmit_interval++;
                }

                char tint[20];
                if (bit_is_set(status, TRANSMIT_ON)) {
                    myversionControl.TransmitInterval = transmit_interval;
                    eeprom_save();

                    uint8_t seconds = transmit_interval + 1; // add 1 to get the actual seconds
                    OCR1A = timerOneSec * seconds; // set timer to new interval

                    lcd_say(itoa(seconds, tint, 10));
                }
                buttons &= ~(1 << TRANSMIT_INTERVAL_BTN);
            }
        }
    }
}

