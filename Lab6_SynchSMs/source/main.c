/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #6 Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "timer.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum SM_STATES { SM_SMStart, SM_Pattern1, SM_Pattern2Down, SM_Pattern2Up, SM_Pattern3, SM_Stopped, SM_Wait } SM_STATE;

void TickFct_Pattern() {
    switch (SM_STATE) {
        case SM_SMStart:
            SM_STATE = SM_Pattern1;
            break;
        case SM_Pattern3:
            if (~PINA & 0x01) {
                SM_STATE = SM_Stopped;
            } else {
                SM_STATE = SM_Pattern2Up;
            }
            break;
        case SM_Pattern1:
            if (~PINA & 0x01) {
                SM_STATE = SM_Stopped;
            } else {
                SM_STATE = SM_Pattern2Down;
            }
            break;
        case SM_Pattern2Up:
            if (~PINA & 0x01) {
                SM_STATE = SM_Stopped;
            } else {
                SM_STATE = SM_Pattern1;
            }
            break;
        case SM_Pattern2Down:
            if (~PINA & 0x01) {
                SM_STATE = SM_Stopped;
            } else {
                SM_STATE = SM_Pattern3;
            }
            break;
        case SM_Stopped:
            if (~PINA & 0x01) {
                SM_STATE = SM_Stopped; // Do nothing since button is still pressed
            } else {
                SM_STATE = SM_Wait; // Now transition to wait since button is not pressed
            }
            break;
        case SM_Wait:
            if (~PINA & 0x01) {
                SM_STATE = SM_Pattern1;
            }
            break;
    }
    
    switch (SM_STATE) {
        case SM_Pattern1:
            PORTB = 0x01;
            break;
        case SM_Pattern2Up:
        case SM_Pattern2Down:
            PORTB = 0x02;
            break;
        case SM_Pattern3:
            PORTB = 0x04;
            break;
        default:
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0x07; PORTB = 0x00;
    
    TimerSet(300);
    TimerOn();
    
    SM_STATE = SM_SMStart;

    /* Insert your solution below */
    while (1) {
        TickFct_Pattern();
        
        while (!TimerFlag) { }
        TimerFlag = 0;
    }
    return 1;
}
