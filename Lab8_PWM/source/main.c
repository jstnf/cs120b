/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #8 Exercise #3
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

#define ds4 311.13
#define e4 329.63
#define fs4 369.99
#define gs4 415.30
#define a4 440
#define b4 493.88
#define cs5 554.37
#define d5 587.33

void set_PWM(double frequency) {
    static double current_frequency;
    if (frequency != current_frequency) {
        if (!frequency) { TCCR3B &= 0x08; }
        else { TCCR3B |= 0x03; }
        
        if (frequency < 0.954) { OCR3A = 0xFFFF; }
        else if (frequency > 31250) { OCR3A = 0x0000; }
        else { OCR3A = (short) (8000000 / (128 * frequency)) - 1; }
        
        TCNT3 = 0;
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

double notes[75] = { 
    e4, a4, a4, cs5, cs5, 
    a4, e4, e4, e4, e4, 
    b4, a4, gs4, fs4, e4, 
    e4, a4, a4, cs5, cs5, 
    a4, e4, a4, gs4, fs4, 
    gs4, a4, ds4, e4, e4, 
    gs4, gs4, a4, gs4, fs4, 
    gs4, a4, e4, a4, gs4, 
    gs4, gs4, d5, b4, gs4, 
    a4, a4, fs4, fs4, fs4, 
    a4, a4, e4, e4, e4, 
    e4, b4, gs4, a4, a4, 
    gs4, fs4, fs4, fs4, a4, 
    gs4, b4, a4, e4, e4, 
    e4, e4, b4, gs4, a4 };
    
unsigned char times[75] = { 
    4, 4, 4, 4, 4, 
    8, 4, 4, 6, 2, 
    2, 2, 2, 2, 12, 
    4, 4, 4, 4, 4, 
    8, 4, 4, 4, 2, 
    2, 4, 4, 12, 4, 
    4, 4, 2, 2, 2, 
    2, 8, 4, 4, 4, 
    4, 2, 2, 2, 2, 
    12, 4, 4, 4, 4, 
    4, 8, 4, 4, 6, 
    2, 4, 4, 12, 4, 
    2, 2, 4, 2, 2, 
    2, 2, 8, 4, 4, 
    6, 2, 4, 4, 12 };

unsigned char playing = 0x00;
unsigned char index = 0x00;
unsigned char length = 75;
    
enum SM1_STATES { SM1_SMStart, SM1_WaitFall, SM1_WaitRise, SM1_Playing } SM1_STATE;
enum SM2_STATES { SM2_SMStart, SM2_Waiting, SM2_Playing, SM2_Transition, SM2_Done } SM2_STATE;

void TickFct_SM1() {
    switch (SM1_STATE) {
        case SM1_SMStart:
            SM1_STATE = SM1_WaitFall;
            break;
        case SM1_WaitFall:
            if (~PINA & 0x01) SM1_STATE = SM1_Playing;
            break;
        case SM1_WaitRise:
            if ((~PINA & 0x01) == 0x00) SM1_STATE = SM1_WaitFall;
            break;
        case SM1_Playing:
            index++;
            if (index >= length) {
                SM1_STATE = SM1_WaitRise;
            }
            break;
    }
    
    switch (SM1_STATE) {
        case SM1_Playing:
            playing = 0x01;
            break;
        default:
            set_PWM(0);
            break;
    }
}

unsigned char currTime = 0x00;

void TickFct_SM2() {
    switch (SM2_STATE) {
        case SM2_Done:
            index = 0;
        case SM2_SMStart:
            SM2_STATE = SM2_Waiting;
            break;
        case SM2_Waiting:
            if (playing) {
                playing = 0x00;
                SM2_STATE = SM2_Playing;
            }
            break;
        case SM2_Playing:
            currTime += 1;
            if (currTime >= times[index]) {
                currTime = 0;
                SM2_STATE = SM2_Transition;
            }
            break;
        case SM2_Transition:
            if (index < length) {
                SM2_STATE = SM2_Playing;
            } else {
                SM2_STATE = SM2_Done;
            }
            break;
    }
    
    switch (SM2_STATE) {
        case SM2_Playing:
            set_PWM(notes[index]);
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;

    /* Insert your solution below */
	TimerSet(150);
	TimerOn();
    SM1_STATE = SM1_SMStart;
    SM2_STATE = SM2_SMStart;
    PWM_on();
    while (1) {
        TickFct_SM1();
        TickFct_SM2();
    }
    return 1;
}
