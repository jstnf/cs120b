/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #8 Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://youtu.be/erxG3HV5qKE
 */
#include <avr/io.h>
#include <timer.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void set_PWM(double frequency) {
    static double current_frequency;
    if (frequency != current_frequency) {
        if (!frequency) { TCCR3B &= 0x08; }
        else { TCCR3B |= 0x03; }
        
        if (frequency < 0.954) { OCR3A = 0xFFFF; }
        else if (frequency > 31250) { OCR3A = 0x0000; }
        else { OCR3A = (short) (8000000 / (128 * frequency)) - 1; }
        
        TCNT3 = 0;
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

enum SM_STATES { SM_SMStart, SM_Wait, SM_C, SM_D, SM_E } SM_STATE;

void TickFct_ThreeTones() {
    switch (SM_STATE) {
        case SM_SMStart:
            SM_STATE = SM_Wait;
            break;
        case SM_Wait:
            if ((~PINA & 0x07) == 0x01) {
               SM_STATE = SM_C;
            } else if ((~PINA & 0x07) == 0x02) {
                SM_STATE = SM_D;
            } else if ((~PINA & 0x07) == 0x04) {
                SM_STATE = SM_E;
            }
            break;
        case SM_C:
            if ((~PINA & 0x07) != 0x01) {
                SM_STATE = SM_Wait;
            }
            break;
        case SM_D:
            if ((~PINA & 0x07) != 0x02) {
                SM_STATE = SM_Wait;
            }
            break;
        case SM_E:
            if ((~PINA & 0x07) != 0x04) {
                SM_STATE = SM_Wait;
            }
            break;
    }
    
    switch (SM_STATE) {
        case SM_C:
            set_PWM(261.63);
            break;
        case SM_D:
            set_PWM(293.66);
            break;
        case SM_E:
            set_PWM(329.63);
            break;
        default:
            set_PWM(0);
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;

    /* Insert your solution below */
    SM_STATE = SM_SMStart;
    PWM_on();
    while (1) {
        TickFct_ThreeTones();
    }
    return 1;
}
