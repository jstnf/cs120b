/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #9 Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: <>
 */
#include <avr/io.h>
#include "timer.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum SM1_STATES { SM1_SMStart, SM1_Light1, SM1_Light2, SM1_Light3 } SM1_STATE;
enum SM2_STATES { SM2_SMStart, SM2_On, SM2_Off } SM2_STATE;
enum SM3_STATES { SM3_SMStart, SM3_Combine } SM3_STATE;

unsigned int timer1 = 0;
unsigned char threeLEDs = 0x00;

void TickFct_ThreeLEDs() {
    switch (SM1_STATE) {
        case SM1_SMStart:
            SM1_STATE = SM1_Light1;
            break;
        case SM1_Light1:
            if (timer1 >= 1000) {
                timer1 = 0;
                SM1_STATE = SM1_Light2;
            }
            break;
        case SM1_Light2:
            if (timer1 >= 1000) {
                timer1 = 0;
                SM1_STATE = SM1_Light3;
            }
            break;
        case SM1_Light3:
            if (timer1 >= 1000) {
                timer1 = 0;
                SM1_STATE = SM1_Light1;
            }
            break;
    }
    
    switch (SM1_STATE) {
        case SM1_Light1:
            threeLEDs = 0x01;
            timer++;
            break;
        case SM1_Light2:
            threeLEDs = 0x02;
            timer++;
            break;
        case SM1_Light3:
            threeLEDs = 0x04;
            timer++;
            break;
        default:
            break;
    }
}

unsigned int timer2 = 0;
unsigned char blinkingLED = 0x00;

void TickFct_Blinking() {
    switch (SM2_STATE) {
        case SM2_SMStart:
            SM2_STATE = SM2_On;
            break;
        case SM2_On:
            if (timer2 >= 1000) {
                timer = 0;
                SM2_STATE = SM2_Off;
            }
            break;
        case SM2_Off:
            if (timer2 >= 1000) {
                timer = 0;
                SM2_STATE = SM2_On;
            }
            break;
    }
    
    switch (SM2_STATE) {
        case SM2_On:
            blinkingLED = 0x08;
            timer++;
            break;
        case SM2_Off:
            blinkingLED = 0x01;
            timer++;
            break;
    }
}

void TickFct_Combine() {
    switch (SM3_STATE) {
        case SM3_SMStart:
        case SM3_Combine:
            SM3_STATE = SM3_Combine;
            break;
    }
    
    switch (SM3_STATE) {
        case SM3_Combine:
            PORTB = blinkingLED | threeLEDs;
            break;
        default:
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0x0F; PORTB = 0x00;
    
    TimerSet(1);
    TimerOn();
    
    SM1_STATE = SM1_SMStart;
    SM2_STATE = SM2_SMStart;
    SM3_STATE = SM3_SMStart;

    /* Insert your solution below */
    while (1) {
        TickFct_ThreeLEDs();
        TickFct_Blinking();
        TickFct_Combine();
        
        while (!TimerFlag) { }
        TimerFlag = 0;
    }
    return 1;
}
