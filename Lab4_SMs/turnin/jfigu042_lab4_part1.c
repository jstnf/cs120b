/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #4 Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum SM_STATES { SM_SMStart, SM_WaitRise, SM_Switch, SM_WaitFall } SM_STATE;

unsigned char currLightState = 0x01;

void TickFct_Switch() {
    switch (SM_STATE) {
        case SM_SMStart:
            SM_STATE = SM_WaitRise;
            break;
        case SM_WaitRise:
            if ((PINA & 0x01) == 0x01) SM_STATE = SM_Switch;
            break;
        case SM_Switch:
            SM_STATE = SM_WaitFall;
            break;
        case SM_WaitFall:
            if ((PINA & 0x01) == 0x00) SM_STATE = SM_WaitRise;
            break;
    }
    
    switch (SM_STATE) {
        case SM_Switch:
            currLightState = currLightState >> 1;
            if ((currLightState & 0x03) == 0x00) currLightState += 0x02;
            PORTB = currLightState;
            break;
        default:
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00, PORTA = 0x00;
    DDRB = 0x03, PORTB = 0x01;

    /* Insert your solution below */
    while (1) {
        TickFct_Switch();
    }
    return 1;
}
