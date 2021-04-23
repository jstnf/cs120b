/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #4 Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum SM_STATES { SM_SMStart, SM_Wait, SM_Wait1Rise, SM_Wait1Fall, SM_Unlock, SM_UnlockFall, SM_Lock } SM_STATE;

unsigned char currState = 0x00;

void TickFct_LockSystem() {
    switch (SM_STATE) {
        case SM_SMStart:
            SM_STATE = SM_Wait;
            break;
        case SM_Wait:
            if ((PINA & 0xFF) == 0x00) SM_STATE = SM_Wait; // We have done nothing, just sit and wait
            if ((PINA & 0xFF) == 0x04) SM_STATE = SM_Wait1Rise; // Start initiating right sequence
            else SM_STATE = SM_Lock; // Wrong combination or locked door
            break;
        case SM_Wait1Rise:
            if ((PINA & 0xFF) == 0x04) SM_STATE = SM_Wait1Rise; // Have not released the button yet
            else if ((PINA & 0xFF) == 0x00) SM_STATE = SM_Wait1Fall; // We released the button and have not pressed anything else
            else SM_STATE = SM_Lock; // Wrong combination or locked door
            break;
        case SM_Wait1Fall:
            if ((PINA & 0xFF) == 0x00) SM_STATE = SM_Wait1Fall; // Have not pressed anything else yet
            else if ((PINA & 0xFF) == 0x02) SM_STATE = SM_Unlock; // Right combination!
            else SM_STATE = SM_Lock; // Wrong combination or locked door
            break;
        case SM_Unlock:
            SM_STATE = SM_UnlockFall; // Now we wait
            break;
        case SM_UnlockFall:
            if ((PINA & 0x80) == 0x80) SM_STATE = SM_Lock; // Lock button takes all precedence
            else if ((PINA & 0xFF) == 0x02) SM_STATE = SM_UnlockFall; // We still have not released the button
            else if ((PINA & 0xFF) != 0x00) SM_STATE = SM_Lock; // We did the wrong combination
            else SM_STATE = SM_Wait; // Released all buttons, now we wait
            break;
        case SM_Lock:
            SM_STATE = SM_Wait; // Now we wait
            break;
    }
    
    switch (SM_STATE) {
        case SM_Wait1Rise:
        case SM_Wait1Fall:
        case SM_Lock:
            currState = 0x00;
            PORTB = currState;
            break;
        case SM_Unlock:
            currState = 0x01;
            PORTB = currState;
            break;
        default:
            break;
    }
}

int main() {
    DDRA = 0x00, PORTA = 0x00;
    DDRB = 0x01, PORTB = 0x00;

    while (1) {
        TickFct_LockSystem();
    }
}
