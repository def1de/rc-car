#define F_CPU 16000000UL  // 16 MHz clock speed for the Pro Micro
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    // Set PD5 (pin for RX LED) as output
    DDRD |= (1 << PD5);  // PD5 is now an output pin

    while (1) {
        // Turn the RX LED on (active low)
        PORTD &= ~(1 << PD5);  // Clear bit PD5 to 0 (LOW)
        _delay_ms(1000);       // Wait for 1 second

        // Turn the RX LED off
        PORTD |= (1 << PD5);   // Set bit PD5 to 1 (HIGH)
        _delay_ms(1000);       // Wait for 1 second
    }

    return 0;  // Should never reach this
}
