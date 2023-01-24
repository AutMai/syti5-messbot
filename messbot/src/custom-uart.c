#include "custom-uart.h"

// bits for uart stx received, data buffer and etx received
uint8_t stx = 0;
uint8_t uart_data[64];
uint8_t uart_data_index = 0;

void handleUartInput() {
    // read uart input
    unsigned int input = uart_getc();
    uart_putc(input);
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