#include "timer.h"

int timerOneSec = 15624;

void timer_init() {
    TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode
    sei(); //  Enable global interrupts
    OCR1A = timerOneSec * (myversionControl.TransmitInterval +
                           1); // Set CTC compare value to 1Hz at 1MHz AVR clock, with a prescaler of 64
    TCCR1B |= ((1 << CS10) | (1 << CS12)); // Start timer at Fcpu/1024
}

ISR(TIMER1_COMPA_vect) {
    status |= (1 << TRANSMIT);
    PORTD |= (1 << PD3);
    _delay_ms(50);
    PORTD &= ~(1 << PD3);

}