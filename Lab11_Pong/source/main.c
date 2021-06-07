/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #11 Exercise #1
 *	Exercise Description: Class Pong Game Implementation
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: <>
 */
#include <avr/io.h>
#include "pwm.h"
#include "timer.h"
#include "scheduler.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char row1 = 0x11;
unsigned char row2 = 0x22;
unsigned char row3 = 0x43;
unsigned char row4 = 0x84;
unsigned char row5 = 0x15;

unsigned char row = 0x01;
unsigned char pattern = 0x00;

enum SM1_LED { SM1_SMStart, SM1_R1, SM1_R2, SM1_R3, SM1_R4, SM1_R5 };
int TickFct_DrawLED(int state) {
    switch (state) {
        default:
            state = SM1_R1;
            break;
        case SM1_R1:
            state = SM1_R2;
            break;
        case SM1_R2:
            state = SM1_R3;
            break;
        case SM1_R3:
            state = SM1_R4;
            break;
        case SM1_R4:
            state = SM1_R5;
            break;
    }
    
    switch (state) {
        default:
            row = 0xFE;
            pattern = row1;
            break;
        case SM1_R2:
            row = 0xFD;
            pattern = row2;
            break;
        case SM1_R3:
            row = 0xFB;
            pattern = row3;
            break;
        case SM1_R4:
            row = 0xF7;
            pattern = row4;
            break;
        case SM1_R5:
            row = 0xEF;
            pattern = row5;
            break;
    }
    
    return state;
}

// SM to create outputs (req. since lab 9)
enum SM0_STATES { SM0_SMStart, SM0_Combine };
int TickFct_Combine(int state) {
    switch (state) {
        default:
            state = SM0_Combine;
            break;
    }
    
    switch (state) {
        case SM0_Combine:
            PORTC = pattern;
            PORTD = row;
            break;
        default:
            break;
    }
    
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    /* Insert your solution below */
    static task task0, task1;
    task *tasks[] = { &task0, &task1 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    const char start = 0;
    task0.state = start;
    task0.period = 1;
    task0.elapsedTime = task0.period;
    task0.TickFct = &TickFct_Combine;
    
    task1.state = start;
    task1.period = 1;
    task1.elapsedTime = task1.period;
    task1.TickFct = &TickFct_DrawLED;
    
    TimerSet(1);
    TimerOn();
    PWM_on();
    
    unsigned short i;
    while (1) {
        for (i = 0; i < numTasks; i++) {
            if (tasks[i]->elapsedTime == tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += 1;
        }
        
        while (!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}