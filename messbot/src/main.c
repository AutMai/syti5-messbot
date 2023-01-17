#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>
#include <util/atomic.h>
#include "eeprom.h"
#include "global.h"

versionControl myversionControl = {
        sizeof(myversionControl),
        1,
        23,
        12,
        1,
        0};

#define UART_BAUD_RATE 9600

// button bits in buttons variable
#define DIGITAL_INPUT_BTN 0
#define ON_OFF_BTN 1
#define TRANSMIT_INTERVAL_BTN 2

// status bits for the status byte
#define TRANSMIT_INTERVAL_L 0
#define TRANSMIT_INTERVAL_H 1
#define ON  2
#define TRANSMIT_ON 3
#define TRANSMIT 4

#define ALL 0
#define SWITCH 1
#define PHOTO_RESISTOR 2

volatile uint8_t transmitSensors = 0;
volatile uint8_t buttons = 0;
volatile uint8_t status = 0;

volatile int16_t adc = 0;

float get_adc_volt() {
    float adc_val;
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        adc_val = (float) adc;
    }
    return adc_val / 1024.0f * 5.0f;
}

int timerOneSec = 15624;

void adc_init() {
    ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADATE) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
    ADCSRB |= (0 << ADTS0) | (0 << ADTS1) | (0 << ADTS2);
    ADMUX |= (0 << REFS1) | (1 << REFS0);
    ADMUX |= 5;
    ADCSRA |= (1 << ADSC); // start
}

void timer_init() {
    TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode
    sei(); //  Enable global interrupts
    OCR1A = timerOneSec * (myversionControl.TransmitInterval +
                           1); // Set CTC compare value to 1Hz at 1MHz AVR clock, with a prescaler of 64
    TCCR1B |= ((1 << CS10) | (1 << CS12)); // Start timer at Fcpu/1024
}

void button_init() {
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2); // interner pull up
    PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2); // pin change interrupts am pin aktivieren
    PCICR |= (1 << PCIE0); // pin change interrupt gruppe 0 aktivieren
}

void lcd_say(char *text) {
    lcd_clrscr();
    lcd_home();
    lcd_puts(text);
    _delay_ms(1000);
    lcd_clrscr();
}

void transmit_off() {
    lcd_say("transmit off");
    TIMSK1 &= ~(1 << OCIE1A); // Enable CTC interrupt
    status &= ~(1 << TRANSMIT_ON);
}

void transmit_on() {
    lcd_say("transmit on");
    TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt
    status |= (1 << TRANSMIT_ON);
}

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

void transmitAll() {
    char buffer[10];
    float volts = get_adc_volt();
    char buttonStatus[3];
    strcpy(buttonStatus, bit_is_set(buttons, DIGITAL_INPUT_BTN) ? "on" : "off");
    char buffer2[64];
    snprintf(buffer2, sizeof(buffer2), "light: %s, switch: %s", dtostrf(volts, 0, 2, buffer), buttonStatus);
    uart_putc(2);
    uart_puts(buffer2);
    uart_putc(3);
}

void transmitSwitch() {
    char message[64];
    snprintf(message, sizeof(message), "switch: %s", bit_is_set(buttons, DIGITAL_INPUT_BTN) ? "on" : "off");
    uart_putc(2);
    uart_puts(message);
    uart_putc(3);
}

void transmitLight() {
    char buffer[10];
    float volts = get_adc_volt();
    char message[64];
    snprintf(message, sizeof(message), "light: %s", dtostrf(volts, 0, 2, buffer));
    uart_putc(2);
    uart_puts(message);
    uart_putc(3);
}

void transmit() {
    switch (transmitSensors) {
        case ALL:
            transmitAll();
            break;
        case SWITCH:
            transmitSwitch();
            break;
        case PHOTO_RESISTOR:
            transmitLight();
            break;
        default:
            lcd_say("error");
            break;
    }
}

// bits for uart stx received, data buffer and etx received
volatile uint8_t stx = 0;
volatile uint8_t uart_data[64];
volatile uint8_t uart_data_index = 0;

void handleUartInput() {
    // read uart input
    unsigned int input = uart_getc();
    // get lower byte from input as data and higher byte as status
    uint8_t data = input & 0xFF;

    if (input == UART_NO_DATA) {
        return;
    }
    if (stx == 0) {
        if (data == 50) {
            stx = 1;
            // clear data buffer
            memset(uart_data, 0, sizeof(uart_data));
            uart_data_index = 0;
        }
    } else {
        if (data == 51) {
            stx = 0;
            // handle data
            if (uart_data[0] == 'q') {
                transmit_off();
            } else {
                if (bit_is_clear(status, TRANSMIT_ON)) {
                    transmit_on();
                }
            }
            switch (uart_data[0]) {
                case 'q':
                    transmit_off();
                    break;
                case 'a':
                    transmitSensors = ALL;
                    break;
                case 's':
                    transmitSensors = SWITCH;
                    break;
                case 'l':
                    transmitSensors = PHOTO_RESISTOR;
                    break;
                default:
                    break;
            }
            memset(uart_data, 0, sizeof(uart_data));
            uart_data_index = 0;
        } else {
            uart_data[uart_data_index] = data;
            uart_data_index++;
        }
    }
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
                    /*status &= ~(1 << TRANSMIT_INTERVAL_L | 1 << TRANSMIT_INTERVAL_H); // clear last two bits
                    status |= transmit_interval; // set last two bits to transmitAll interval*/

                    uint8_t seconds = transmit_interval + 1; // add 1 to get the actual seconds
                    OCR1A = timerOneSec * seconds; // set timer to new interval

                    lcd_say(itoa(seconds, tint, 10));
                }
                buttons &= ~(1 << TRANSMIT_INTERVAL_BTN);
            }
        }
    }
}

ISR(PCINT0_vect) {
    if (bit_is_clear(PINB, PB0)) {
        buttons ^= (1 << DIGITAL_INPUT_BTN);
    } else if (bit_is_clear(PINB, PB1)) {
        buttons |= (1 << ON_OFF_BTN);
    } else if (bit_is_clear(PINB, PB2)) {
        buttons |= (1 << TRANSMIT_INTERVAL_BTN);
    }
}

ISR(TIMER1_COMPA_vect) {
    status |= (1 << TRANSMIT);
    //transmitAll();
    /*PORTD |= (1 << PD3);
    _delay_ms(50);
    PORTD &= ~(1 << PD3);*/
}

ISR(ADC_vect) {
    adc = ADCW;
}