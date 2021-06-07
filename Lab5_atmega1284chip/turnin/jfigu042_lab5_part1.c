/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #5 Exercise #1
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
    DDRC = 0x3F; PORTC = 0x00;

    /* Insert your solution below */
    unsigned char tmpA = 0x00;
    unsigned char result = 0x00;
    while (1) {
        result = 0x00;
        tmpA = ~((PINA & 0x0F) + 0xF0); // Apply bitwise NOT and add 0xF0 to achieve results similar to before in pull-up mode
        if (tmpA > 0x00) { // > 0
            result = result | 0x20;
            if (tmpA > 0x02) { // > 2
                result = result | 0x10;
                if (tmpA > 0x04) { // > 4
                    result = result | 0x08;
                    result = (tmpA > 0x06) ? result | 0x04 : result; // > 6
                    result = (tmpA > 0x09) ? result | 0x02 : result; // > 9
                    result = (tmpA > 0x0C) ? result | 0x01 : result; // > 12
                } else {
                    result = result | 0x40;
                }
            } else {
                result = result | 0x40;
            }
        } else {
            result = 0x40;
        }
        PORTC = result;
    }
    return 1;
}
