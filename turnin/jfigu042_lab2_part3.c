/*	Author: jfigu042
 *  Partner(s) Name: <none> 
 *	Lab Section: 021
 *	Assignment: Lab #2 Exercise #3
 *	Exercise Description: Light that turns on if garage door is open at night
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0x00;
    DDRC = 0x07; PORTC = 0x00;
    /* Insert your solution below */
    unsigned char tmpA = 0x00;
    unsigned char cntavail = 0x00;
    while (1) {
        cntavail = 0x00;
        tmpA = PINA;
        int counter = 0;
        for (counter = 0; counter < 4; counter++) {
            if ((tmpA & 0x01) == 0x00) {
                cntavail += 1;
            }
            tmpA = tmpA >> 1;
        }
        if (PINA == 0x0F) { cntavail = cntavail | 0x08; }
        PORTC = cntavail;
    }
    return 1;
}
