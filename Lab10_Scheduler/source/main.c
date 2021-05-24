/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #10 Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char GetKeypadKey() {
    PORTC = 0xEF;
    asm("nop");
    if (GetBit(PINC,0)==0) return('1');
    if (GetBit(PINC,1)==0) return('4');
    if (GetBit(PINC,2)==0) return('7');
    if (GetBit(PINC,3)==0) return('*');
    
    PORTC = 0xDF;
    asm("nop");
    if (GetBit(PINC,0)==0) return('2');
    if (GetBit(PINC,1)==0) return('5');
    if (GetBit(PINC,2)==0) return('8');
    if (GetBit(PINC,3)==0) return('0');
    
    PORTC = 0xBF;
    asm("nop");
    if (GetBit(PINC,0)==0) return('3');
    if (GetBit(PINC,1)==0) return('6');
    if (GetBit(PINC,2)==0) return('9');
    if (GetBit(PINC,3)==0) return('#');
    
    PORTC = 0x7F;
    asm("nop");
    if (GetBit(PINC,0)==0) return('A');
    if (GetBit(PINC,1)==0) return('B');
    if (GetBit(PINC,2)==0) return('C');
    if (GetBit(PINC,3)==0) return('D');
    
    return('\0');
}

enum SM_STATES { SM_SMStart, SM_Wait, SM_Press } SM_STATE;

void TickFct_KeyPad() {
    switch (SM_STATE) {
        case SM_SMStart:
            SM_STATE = SM_Wait;
            break;
        case SM_Wait:
            break;
        case SM_Press:
            break;
    }
    
    switch (SM_STATE) {
        case SM_Press:
            break;
        default:
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xF0; PORTC = 0x0F;

    /* Insert your solution below */
    unsigned char x;
    while (1) {
        x = GetKeypadKey();
        switch (x) {
            case '\0':
                PORTB = 0x00;
                break;
            default:
                PORTB = 0xFF;
                break;
        }
    }
    return 1;
}
