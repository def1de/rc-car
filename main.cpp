#include <avr/io.h>
#include <util/delay.h>
#include "pins.hpp"

#define F_CPU 16000000UL  // 16 MHz clock speed for the Pro Micro

int main() {
    PinPWM red(&DDRB, &PORTB, PB1);
    PinPWM green(&DDRB, &PORTB, PB2);
    PinPWM blue(&DDRB, &PORTB, PB3);

    red.SetupPWM();
    green.SetupPWM();
    blue.SetupPWM();
    
    red.Set(255);
    green.Set(128); // Phase shift for green
    blue.Set(0); // Phase shift for blue

    while (true) {
    }
}