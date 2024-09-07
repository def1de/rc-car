/*
 * DDR = Data Direction Register, determines if the pin is an input or output
 * PORT = Data Register used to set the pin high or low
 * PWM = Pulse Width Modulation, a way of sending an analog signal
 *       in this case used to control the speed of the motor
*/


#ifndef PINS_HPP
#define PINS_HPP

#include <avr/io.h>

#define DELAY 1000

class PinOut{
    private:
        volatile uint8_t *ddr;
        volatile uint8_t *port;
        uint8_t p;

    public:
        PinOut(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t p){
            this->ddr = ddr;
            this->port = port;
            this->p = p;
            setOutput();
        }

        void setOutput() const{
            *ddr = *ddr | (1 << p);
        }

        void High() const{
            *port = *port | (1 << p);
        }

        void Low() const{
            *port = *port & ~(1 << p);
        }

        void Set(bool state) const{
            if(state){
                High();
            }else{
                Low();
            }
        }

        void Toggle() const{
            *port = *port ^ (1 << p);
        }
};

class PinIn{
    private:
        volatile uint8_t *ddr;
        volatile uint8_t *pin;
        uint8_t p;

    public:
        PinIn(volatile uint8_t *ddr, volatile uint8_t *pin, uint8_t p){
            this->ddr = ddr;
            this->pin = pin;
            this->p = p;
            setInput();
        }

        void setInput() const{
            *ddr = *ddr & ~(1 << p);
        }

        bool getState() const{
            return *pin & (1 << p);
        }
};

class PinPWM {
private:
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    uint8_t p;

public:
    PinPWM(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t p) {
        this->ddr = ddr;
        this->port = port;
        this->p = p;
        *ddr |= (1 << p);
    }

    void SetupPWM() const {
        switch (p) {
            case PB1:
                // Set Fast PWM mode with non-inverted output for Timer1 Channel A (Pin 9)
                    TCCR1A |= (1 << COM1A1) | (1 << WGM11) | (1 << WGM10);
            TCCR1B |= (1 << WGM12) | (1 << CS11); // Prescaler = 8
            break;
            case PB2:
                // Set Fast PWM mode with non-inverted output for Timer1 Channel B (Pin 10)
                    TCCR1A |= (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
            TCCR1B |= (1 << WGM12) | (1 << CS11); // Prescaler = 8
            break;
            case PB3:
                // Set Fast PWM mode with non-inverted output for Timer2 Channel A (Pin 11)
                    TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
            TCCR2B |= (1 << CS21); // Prescaler = 8
            break;
            case PD3:
                // Set Fast PWM mode with non-inverted output for Timer2 Channel B (Pin 3)
                    TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
            TCCR2B |= (1 << CS21); // Prescaler = 8
            break;
            case PD5:
                // Set Fast PWM mode with non-inverted output for Timer0 Channel B (Pin 5)
                    TCCR0A |= (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
            TCCR0B |= (1 << CS01); // Prescaler = 8
            break;
            case PD6:
                // Set Fast PWM mode with non-inverted output for Timer0 Channel A (Pin 6)
                    TCCR0A |= (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
            TCCR0B |= (1 << CS01); // Prescaler = 8
            break;
        }
    }

    void Set(uint8_t dutyCycle) const{
        // OCR = Output Compare Register
        if (p == PB1) {
            OCR1A = dutyCycle; // Set duty cycle (0-255) for Timer1 Channel A
        } else if (p == PB2) {
            OCR1B = dutyCycle; // Set duty cycle (0-255) for Timer1 Channel B
        } else if (p == PB3) {
            OCR2A = dutyCycle; // Set duty cycle (0-255) for Timer2 Channel A
        }
    }
};

// ADMUX = ADC Multiplexer Selection Register
// REFS0 = Reference Selection Bit 0
// ADCSRA = ADC Control and Status Register A
// ADEN = ADC Enable
// ADPS2:0 = ADC Prescaler Select Bits
// ADSC = ADC Start Conversion

class PinAnalog {
private:
    uint8_t pin;
public:
    PinAnalog(uint8_t pin) {
        // Initialize the ADC
        pin &= 0x07; // Ensure pin is between 0 and 7
        this->pin = pin;
        ADMUX = (1 << REFS0); // Reference voltage set to AVcc
        ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC and set prescaler to 128
    }

    uint16_t read() const{
        ADMUX = (ADMUX & 0xF8) | this->pin; // Select ADC channel

        // Start single conversion
        ADCSRA |= (1 << ADSC);

        // Wait for conversion to complete
        while (ADCSRA & (1 << ADSC));

        // Return the ADC value
        return ADC;
    }

    uint8_t readAsPWM() const{
        uint16_t analogValue = this->read();
        return (analogValue * 255UL) / 1023; // Use unsigned long to prevent overflow
    }
};

#endif // PINS_HPP