/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #8 Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: <>
 */
#include <avr/io.h>
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

enum SM1_STATES { SM1_SMStart, SM1_OnRise, SM1_OnFall, SM1_OffRise, SM1_OffFall } SM1_STATE;
enum SM2_STATES { SM2_SMStart, SM2_Wait, SM2_IncrementRise, SM2_IncrementFall, SM2_DecrementRise, SM2_DecrementFall } SM2_STATE;

double freqTable[8] = { 261.63, 293.63, 329.63, 349.23, 392, 440, 493.88, 523.25 };
unsigned short index = 0x00;

void TickFct_SM1() {
    switch (SM1_STATE) {
        case SM1_SMStart:
            SM1_STATE = SM1_OffFall;
            break;
        case SM1_OnRise:
            if ((~PINA & 0x01) != 0x01) {
                SM1_STATE = SM1_OnFall;
            }
            break;
        case SM1_OnFall:
            if (~PINA & 0x01) {
                SM1_STATE = SM1_OffRise;
            }
            break;
        case SM1_OffRise:
            if ((~PINA & 0x01) != 0x01) {
                SM1_STATE = SM1_OffFall;
            }
            break;
        case SM1_OffFall:
            if (~PINA & 0x01) {
                SM1_STATE = SM1_OnRise;
            }
            break;
    }
    
    switch (SM1_STATE) {
        case SM1_OnRise:
        case SM1_OnFall:
            set_PWM(freqTable[index]);
            break;
        case SM1_OffRise:
        case SM1_OffFall:
            set_PWM(0);
            break;
    }
    
}

void TickFct_SM2() {
    switch (SM2_STATE) {
        case SM2_SMStart:
            SM2_STATE = SM2_Wait;
            break;
        case SM2_Wait:
            if ((~PINA & 0x06) == 0x02) {
                SM2_STATE = SM2_IncrementRise;
            } else if ((~PINA & 0x06) == 0x04) {
                SM2_STATE = SM2_DecrementRise;
            }
            break;
        case SM2_IncrementRise:
            SM2_STATE = SM2_IncrementFall;
            break;
        case SM2_DecrementRise:
            SM2_STATE = SM2_DecrementFall;
            break;
        case SM2_DecrementFall:
        case SM2_IncrementFall:
            if ((~PINA & 0x06) == 0x00) {
                SM2_STATE = SM2_Wait;
            }
            break;
    }
    
    switch (SM2_STATE) {
        case SM2_IncrementRise:
            index = index == 0x07 ? index : index + 1;
            break;
        case SM2_DecrementRise:
            index = index == 0x00 ? index : index - 1;
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;

    /* Insert your solution below */
    SM1_STATE = SM1_SMStart;
    SM2_STATE = SM2_SMStart;
    PWM_on();
    while (1) {
        TickFct_SM1();
        TickFct_SM2();
    }
    return 1;
}
