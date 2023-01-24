#include "button.h"

uint8_t buttons = 0;

void button_init() {
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2); // interner pull up
    PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2); // pin change interrupts am pin aktivieren
    PCICR |= (1 << PCIE0); // pin change interrupt gruppe 0 aktivieren
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