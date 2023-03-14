#include "custom-uart.h"

extern versionControl myversionControl;

// bits for uart stx received, data buffer and etx received
uint8_t stx = 0;
char uart_data[64];
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
        if (data == UART_STX) {
            stx = 1;
            // clear data buffer
            memset(uart_data, 0, sizeof(uart_data));
            uart_data_index = 0;
        }
    } else {
        if (data == UART_ETX) {
            stx = 0;
            if (strcmp(uart_data, "q") == 0) {
                transmitACK();
                transmit_off();
            } else if (strcmp(uart_data, "req_a") == 0) {
                transmitACK();
                transmitAll();
            } else if (strcmp(uart_data, "req_s") == 0) {
                transmitACK();
                transmitSwitch();
            } else if (strcmp(uart_data, "req_l") == 0) {
                transmitACK();
                transmitLight();
            } else if (strcmp(uart_data, "a") == 0) {
                transmit_on();
                transmitACK();
                transmitSensors = ALL;
            } else if (strcmp(uart_data, "s") == 0) {
                transmit_on();
                transmitACK();
                transmitSensors = SWITCH;
            } else if (strcmp(uart_data, "l") == 0) {
                transmit_on();
                transmitACK();
                transmitSensors = PHOTO_RESISTOR;
            } else if (strcmp(uart_data, "ti1") == 0) {
                transmitACK();
                myversionControl.TransmitInterval = 0;
            } else if (strcmp(uart_data, "ti2") == 0) {
                transmitACK();
                myversionControl.TransmitInterval = 1;
            } else if (strcmp(uart_data, "ti3") == 0) {
                transmitACK();
                myversionControl.TransmitInterval = 2;
            } else if (strcmp(uart_data, "ti4") == 0) {
                transmitACK();
                myversionControl.TransmitInterval = 3;
            } else {
                transmitNAK();
            }

            memset(uart_data, 0, sizeof(uart_data));
            uart_data_index = 0;
        } else {
            uart_data[uart_data_index] = data;
            uart_data_index++;
        }
    }

}