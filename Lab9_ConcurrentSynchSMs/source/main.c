/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #9 Exercise #4
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
enum SM4_STATES { SM4_SMStart, SM4_PulseOn, SM4_PulseOff } SM4_STATE;
enum SM5_STATES { SM5_SMStart, SM5_Wait, SM5_IncrementRise, SM5_IncrementFall } SM5_STATE;
enum SM6_STATES { SM6_SMStart, SM6_Wait, SM6_DecrementRise, SM6_DecrementFall } SM6_STATE;

unsigned int timer1 = 0;
unsigned char threeLEDs = 0x00;

void TickFct_ThreeLEDs() {
    switch (SM1_STATE) {
        case SM1_SMStart:
            SM1_STATE = SM1_Light1;
            break;
        case SM1_Light1:
            if (timer1 >= 300) {
                timer1 = 0;
                SM1_STATE = SM1_Light2;
            }
            break;
        case SM1_Light2:
            if (timer1 >= 300) {
                timer1 = 0;
                SM1_STATE = SM1_Light3;
            }
            break;
        case SM1_Light3:
            if (timer1 >= 300) {
                timer1 = 0;
                SM1_STATE = SM1_Light1;
            }
            break;
    }
    
    switch (SM1_STATE) {
        case SM1_Light1:
            threeLEDs = 0x01;
            timer1++;
            break;
        case SM1_Light2:
            threeLEDs = 0x02;
            timer1++;
            break;
        case SM1_Light3:
            threeLEDs = 0x04;
            timer1++;
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
                timer2 = 0;
                SM2_STATE = SM2_Off;
            }
            break;
        case SM2_Off:
            if (timer2 >= 1000) {
                timer2 = 0;
                SM2_STATE = SM2_On;
            }
            break;
    }
    
    switch (SM2_STATE) {
        case SM2_On:
            blinkingLED = 0x08;
            timer2++;
            break;
        case SM2_Off:
            blinkingLED = 0x00;
            timer2++;
            break;
        default:
            break;
    }
}

unsigned int timer4 = 0;
unsigned int freq = 2;
unsigned char soundBit = 0x00;

void TickFct_Sound() {
    switch (SM4_STATE) {
        case SM4_SMStart:
            SM4_STATE = SM4_PulseOn;
            break;
        case SM4_PulseOn:
            if (timer4 >= freq) {
                timer4 = 0;
                SM4_STATE = SM4_PulseOff;
            }
            break;
        case SM4_PulseOff:
            if (timer4 >= freq) {
                timer4 = 0;
                SM4_STATE = SM4_PulseOn;
            }
            break;
    }
    
    switch (SM4_STATE) {
        case SM4_PulseOn:
            if (~PINA & 0x04) {
                soundBit = 0x40;
            } else {
                soundBit = 0x00;
            }
            timer4++;
            break;
        case SM4_PulseOff:
            soundBit = 0x00;
            timer4++;
            break;
        default:
            break;
    }
}

void TickFct_Incrementer() {
    switch (SM5_STATE) {
        case SM5_SMStart:
            SM5_STATE = SM5_Wait;
            break;
        case SM5_Wait:
            if (~PINA & 0x01) {
                SM5_STATE = SM5_IncrementRise;
            }
            break;
        case SM5_IncrementRise:
            SM5_STATE = SM5_IncrementFall;
            break;
        case SM5_IncrementFall:
            if (!(~PINA & 0x01)) {
                SM5_STATE = SM5_Wait;
            }
            break;
    }
    
    switch (SM5_STATE) {
        case SM5_IncrementRise:
            if (freq != 1) {
                freq--;
            }
            break;
        default:
            break;
    }
}

void TickFct_Decrementer() {
    switch (SM6_STATE) {
        case SM6_SMStart:
            SM6_STATE = SM6_Wait;
            break;
        case SM5_Wait:
            if (~PINA & 0x02) {
                SM6_STATE = SM6_DecrementRise;
            }
            break;
        case SM6_DecrementRise:
            SM6_STATE = SM6_DecrementFall;
            break;
        case SM6_DecrementFall:
            if (!(~PINA & 0x02)) {
                SM6_STATE = SM6_Wait;
            }
            break;
    }
    
    switch (SM6_STATE) {
        case SM6_DecrementRise:
            freq++;
            break;
        default:
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
            PORTB = blinkingLED | threeLEDs | soundBit;
            break;
        default:
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0x4F; PORTB = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    
    TimerSet(1);
    TimerOn();
    
    SM1_STATE = SM1_SMStart;
    SM2_STATE = SM2_SMStart;
    SM3_STATE = SM3_SMStart;
    SM4_STATE = SM4_SMStart;
    SM5_STATE = SM5_SMStart;
    SM6_STATE = SM6_SMStart;

    /* Insert your solution below */
    while (1) {
        TickFct_ThreeLEDs();
        TickFct_Blinking();
        TickFct_Sound();
        TickFct_Incrementer();
        TickFct_Decrementer();
        TickFct_Combine();
        
        while (!TimerFlag) { }
        TimerFlag = 0;
    }
    return 1;
}
