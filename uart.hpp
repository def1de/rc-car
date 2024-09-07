/* instantiate with
    UART_Init(MYUBRR);
    stdout = &uart_output;
 */

#ifndef UART_HPP
#define UART_HPP

#include <avr/io.h>
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

void UART_Init(unsigned int ubrr) {
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    // Set frame format: 8 data bits, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_Transmit(unsigned char data) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));
    // Put data into buffer, sends the data
    UDR0 = data;
}

int UART_putchar(char c, FILE *stream) {
    if (c == '\n') {
        UART_putchar('\r', stream);
    }
    UART_Transmit(c);
    return 0;
}

FILE uart_output = FDEV_SETUP_STREAM(UART_putchar, NULL, _FDEV_SETUP_WRITE);
#endif //UART_HPP
