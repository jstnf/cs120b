/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #6 Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *      Demo Link: <>
 */
#include <avr/io.h>
#include "timer.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum SM_STATES { SM_SMStart, SM_Pattern1, SM_Pattern2, SM_Pattern3 } SM_STATE;

void TickFct_Pattern() {
    switch (SM_STATE) {
        case SM_SMStart:
        case SM_Pattern3:
            SM_STATE = SM_Pattern1;
            break;
        case SM_Pattern1:
            SM_STATE = SM_Pattern2;
            break;
        case SM_Pattern2:
            SM_STATE = SM_Pattern3;
            break;
    }
    
    switch (SM_STATE) {
        case SM_Pattern1:
            PORTB = 0x01;
            break;
        case SM_Pattern2:
            PORTB = 0x02;
            break;
        case SM_Pattern3:
            PORTB = 0x04;
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
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
