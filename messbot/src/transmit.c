#include "transmit.h"

uint8_t transmitSensors = ALL;

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