#define F_CPU 16000000UL  // 16 MHz clock speed for the Pro Micro
#include <avr/io.h>
#include <util/delay.h>

int main(void){
    DDRB = DDRB | (1 << DDB5);

    while(1){
        PORTB = PORTB | (1 << PORTB5);
        _delay_ms(100);
        PORTB = PORTB & ~(1 << PORTB5);
        _delay_ms(100);
    }
}

