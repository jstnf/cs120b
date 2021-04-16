/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #3 Exercise #3
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
    DDRC = 0xFF; PORTC = 0x00;

    /* Insert your solution below */
    unsigned char fuel = 0x00;
    unsigned char result = 0x00;
    while (1) {
        result = 0x00;
        fuel = PINA & 0x0F;
        if (fuel > 0x00) {
            result = result | 0x20;
            if (fuel > 0x02) {
                result = result | 0x10;
                if (fuel > 0x04) {
                    result = result | 0x08;
                    result = (fuel > 0x06) ? result | 0x04 : result;
                    result = (fuel > 0x09) ? result | 0x02 : result;
                    result = (fuel > 0x0C) ? result | 0x01 : result;
                } else {
                    result = result | 0x40;
                }
            } else {
                result = result | 0x40;
            }
        } else {
            result = 0x40;
        }
        
        result = ((PINA & 0x30) == 0x30) ? result | 0x80 : result;
        PORTC = result;
    }
    return 1;
}
