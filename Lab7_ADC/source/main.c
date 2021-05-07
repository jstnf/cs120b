/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #7 Exercise #4
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: <>
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void ADC_init() {
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

unsigned short x = 0x0000;
unsigned short MAX = 0x0300;

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0x00;
    DDRB = 0x01; PORTB = 0x00;

    ADC_init();

    /* Insert your solution below */
    while (1) {
        x = ADC;
        unsigned char div = (char) (x/8);
        unsigned char i = 0x00;
        unsigned char temp = 0x00;
        for (i = 0x00; i < div; i++) {
            temp = temp << 1;
            temp += 0x01;
        }
        PORTB = temp;
    }
    return 1;
}
