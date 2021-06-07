/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #3 Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
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
    DDRB = 0x00; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;

    /* Insert your solution below */
    unsigned char cnt = 0x00;
    unsigned char tmpA = 0x00;
    unsigned char tmpB = 0x00;
    while (1) {
        int counter;
        cnt = 0x00;
        tmpA = PINA;
        tmpB = PINB;
        for (counter = 0; counter < 16; counter++) {
            if ((tmpA & 0x01) == 0x01) cnt++;
            if ((tmpB & 0x01) == 0x01) cnt++;
            tmpA = tmpA >> 1;
            tmpB = tmpB >> 1;
        }
        PORTC = cnt;
    }
    return 1;
}
