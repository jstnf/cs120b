/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #4 Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum SM_STATES { SM_SMStart, SM_WaitRise, SM_Increment, SM_WaitIncrementFall, SM_Decrement, SM_WaitDecrementFall, SM_Reset, SM_WaitResetFall } SM_STATE;

unsigned char currAmount = 0x07;

void TickFct_Counter() {
    switch (SM_STATE) {
        case SM_SMStart:
            SM_STATE = SM_WaitRise;
            break;
        case SM_WaitRise:
            if ((PINA & 0x03) == 0x03) SM_STATE = SM_Reset;
            else if ((PINA & 0x03) == 0x01) SM_STATE = SM_Increment;
            else if ((PINA & 0x03) == 0x02) SM_STATE = SM_Decrement;
            break;
        case SM_Increment:
            if ((PINA & 0x03) == 0x03) SM_STATE = SM_Reset;
            else SM_STATE = SM_WaitIncrementFall;
            break;
        case SM_WaitIncrementFall:
            if ((PINA & 0x03) == 0x03) SM_STATE = SM_Reset;
            else if ((PINA & 0x03) == 0x00) SM_STATE = SM_WaitRise;
            break;
        case SM_Decrement:
            if ((PINA & 0x03) == 0x03) SM_STATE = SM_Reset;
            else SM_STATE = SM_WaitDecrementFall;
            break;
        case SM_WaitDecrementFall:
            if ((PINA & 0x03) == 0x03) SM_STATE = SM_Reset;
            else if ((PINA & 0x03) == 0x00) SM_STATE = SM_WaitRise;
            break;
        case SM_Reset:
            SM_STATE = SM_WaitResetFall;
            break;
        case SM_WaitResetFall:
            if ((PINA & 0x03) == 0x00) SM_STATE = SM_WaitRise;
            break;
    }
    
    switch (SM_STATE) {
        case SM_Increment:
            if (currAmount != 0x09) currAmount++;
            PORTC = currAmount;
            break;
        case SM_Decrement:
            if (currAmount != 0x00) currAmount--;
            PORTC = currAmount;
            break;
        case SM_Reset:
            currAmount = 0x00;
            PORTC = currAmount;
            break;
        default:
            break;
    }
}

int main() {
    DDRA = 0x00, PORTA = 0x00;
    DDRB = 0x0F, PORTC = 0x07;

    while (1) {
        TickFct_Counter();
    }
}