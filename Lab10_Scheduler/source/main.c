/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #10 Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "timer.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef struct _task {
    signed char state;
    unsigned long int period;
    unsigned long int elapsedTime;
    int (*TickFct)(int);
} task;

unsigned char x = '\0';
void validateOneButton(unsigned char input) {
    if (x == '\0') x = input;
    else x = '-'; // - character means two buttons are being pressed at once
}

void GetKeypadKey() {
    x = '\0';
    
    PORTC = 0xEF;
    asm("nop");
    if ((PINC & 0x01) == 0x00) validateOneButton('1');
    if ((PINC & 0x02) == 0x00) validateOneButton('4');
    if ((PINC & 0x04) == 0x00) validateOneButton('7');
    if ((PINC & 0x08) == 0x00) validateOneButton('*');
    
    PORTC = 0xDF;
    asm("nop");
    if ((PINC & 0x01) == 0x00) validateOneButton('2');
    if ((PINC & 0x02) == 0x00) validateOneButton('5');
    if ((PINC & 0x04) == 0x00) validateOneButton('8');
    if ((PINC & 0x08) == 0x00) validateOneButton('0');
    
    PORTC = 0xBF;
    asm("nop");
    if ((PINC & 0x01) == 0x00) validateOneButton('3');
    if ((PINC & 0x02) == 0x00) validateOneButton('6');
    if ((PINC & 0x04) == 0x00) validateOneButton('9');
    if ((PINC & 0x08) == 0x00) validateOneButton('#');
    
    PORTC = 0x7F;
    asm("nop");
    if ((PINC & 0x01) == 0x00) validateOneButton('A');
    if ((PINC & 0x02) == 0x00) validateOneButton('B');
    if ((PINC & 0x04) == 0x00) validateOneButton('C');
    if ((PINC & 0x08) == 0x00) validateOneButton('D');
}

enum SM1_STATES { SM1_SMStart, SM1_Wrong, SM1_Wait, SM1_Right, SM1_R1, SM1_F1, SM1_R2, SM1_F2, SM1_R3, SM1_F3, SM1_R4, SM1_F4, SM1_R5, SM1_F5 };
unsigned char feedback = 0x00;
int TickFct_KeyPad(int state) {
    GetKeypadKey();
    
    switch (state) {
        case SM1_Wrong:
            if (x == '#') state = SM1_Wait;
            else state = SM1_Wrong;
            break;
        case SM1_Wait:
            if (x == '\0') state = SM1_R1;
            else if (x == '#') state = SM1_Wait;
            else state = SM1_Wrong;
            break;
        case SM1_Right:
            if (x == '\0') state = SM1_Right;
            else state = SM1_Wrong;
            break;
        case SM1_R1:
            if (x == '\0') state = SM1_R1;
            else if (x == '1') state = SM1_F1;
            else state = SM1_Wrong;
            break;
        case SM1_F1:
            if (x == '1') state = SM1_F1;
            else if (x == '\0') state = SM1_R2;
            else state = SM1_Wrong;
            break;
        case SM1_R2:
            if (x == '\0') state = SM1_R2;
            else if (x == '2') state = SM1_F2;
            else state = SM1_Wrong;
            break;
        case SM1_F2:
            if (x == '2') state = SM1_F2;
            else if (x == '\0') state = SM1_R3;
            else state = SM1_Wrong;
            break;
        case SM1_R3:
            if (x == '\0') state = SM1_R3;
            else if (x == '3') state = SM1_F3;
            else state = SM1_Wrong;
            break;
        case SM1_F3:
            if (x == '3') state = SM1_F3;
            else if (x == '\0') state = SM1_R4;
            else state = SM1_Wrong;
            break;
        case SM1_R4:
            if (x == '\0') state = SM1_R4;
            else if (x == '4') state = SM1_F4;
            else state = SM1_Wrong;
            break;
        case SM1_F4:
            if (x == '4') state = SM1_F4;
            else if (x == '\0') state = SM1_R5;
            else state = SM1_Wrong;
            break;
        case SM1_R5:
            if (x == '\0') state = SM1_R5;
            else if (x == '5') state = SM1_F5;
            else state = SM1_Wrong;
            break;
        case SM1_F5:
            if (x == '5') state = SM1_F5;
            else if (x == '\0') state = SM1_Right;
            else state = SM1_Wrong;
            break;
        default:
            state = SM1_Wrong;
            break;
    }
    
    switch (state) {
        case SM1_Right:
            feedback = 0x01;
            break;
        case SM1_Wrong:
            feedback = 0x02;
            break;
        case SM1_R1:
        case SM1_F1:
            feedback = 0x04;
            break;
        case SM1_R2:
        case SM1_F2:
            feedback = 0x08;
            break;
        case SM1_R3:
        case SM1_F3:
            feedback = 0x10;
            break;
        case SM1_R4:
        case SM1_F4:
            feedback = 0x20;
            break;
        case SM1_R5:
        case SM1_F5:
            feedback = 0x40;
            break;
        default:
            feedback = 0x00;
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
            PORTB = feedback;
            break;
        default:
            break;
    }
    
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xF0; PORTC = 0x0F;
    
    static task task1, task0;
    task *tasks[] = { &task1, &task0 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    const char start = -1;
    task1.state = start;
    task1.period = 1;
    task1.elapsedTime = task1.period;
    task1.TickFct = &TickFct_KeyPad;
    
    task0.state = start;
    task0.period = 1;
    task0.elapsedTime = task0.period;
    task0.TickFct = &TickFct_Combine;
    
    TimerSet(1);
    TimerOn();

    /* Insert your solution below */
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
