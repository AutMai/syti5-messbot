#include "adc.h"

volatile int16_t adc = 0;

float get_adc_volt() {
    float adc_val;
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        adc_val = (float) adc;
    }
    return adc_val / 1024.0f * 5.0f;
}

void adc_init() {
    ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADATE) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
    ADCSRB |= (0 << ADTS0) | (0 << ADTS1) | (0 << ADTS2);
    ADMUX |= (0 << REFS1) | (1 << REFS0);
    ADMUX |= 5;
    ADCSRA |= (1 << ADSC); // start
}

ISR(ADC_vect) {
        adc = ADCW;
}
