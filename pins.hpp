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

    void SetupPWM() {
        if (p == PB1) {
            // Set Fast PWM mode with non-inverted output for Timer1
            TCCR1A |= (1 << COM1A1) | (1 << WGM11) | (1 << WGM10);
            TCCR1B |= (1 << WGM12) | (1 << CS11); // Prescaler = 8
        } else if (p == PB2) {
            // Set Fast PWM mode with non-inverted output for Timer1
            TCCR1A |= (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
            TCCR1B |= (1 << WGM12) | (1 << CS11); // Prescaler = 8
        } else if (p == PB3) {
            // Set Fast PWM mode with non-inverted output for Timer2
            TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
            TCCR2B |= (1 << CS21); // Prescaler = 8
        }
    }

    void Set(uint8_t dutyCycle) {
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

#endif // PINS_HPP