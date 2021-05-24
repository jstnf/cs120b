/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #10 Exercise #1
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

unsigned char GetKeypadKey() {
    PORTC = 0xEF;
    asm("nop");
    if ((PINC & 0x01) == 0x00) return('1');
    if ((PINC & 0x02) == 0x00) return('4');
    if ((PINC & 0x04) == 0x00) return('7');
    if ((PINC & 0x08) == 0x00) return('*');
    
    PORTC = 0xDF;
    asm("nop");
    if ((PINC & 0x01) == 0x00) return('2');
    if ((PINC & 0x02) == 0x00) return('5');
    if ((PINC & 0x04) == 0x00) return('8');
    if ((PINC & 0x08) == 0x00) return('0');
    
    PORTC = 0xBF;
    asm("nop");
    if ((PINC & 0x01) == 0x00) return('3');
    if ((PINC & 0x02) == 0x00) return('6');
    if ((PINC & 0x04) == 0x00) return('9');
    if ((PINC & 0x08) == 0x00) return('#');
    
    PORTC = 0x7F;
    asm("nop");
    if ((PINC & 0x01) == 0x00) return('A');
    if ((PINC & 0x02) == 0x00) return('B');
    if ((PINC & 0x04) == 0x00) return('C');
    if ((PINC & 0x08) == 0x00) return('D');
    
    return('\0');
}

enum SM1_STATES { SM1_SMStart, SM1_Wait, SM1_Press };
unsigned char x;
unsigned char feedback = 0x00;
int TickFct_KeyPad(int state) {
    x = GetKeypadKey();
    
    switch (state) {
        case SM1_Wait:
            if (x != '\0') state = SM1_Press;
            break;
        case SM1_Press:
            if (x == '\0') state = SM1_Wait;
            break;
        default:
            state = SM1_Wait;
            break;
    }
    
    switch (state) {
        case SM1_Press:
            feedback = 0x80;
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
        x = GetKeypadKey();
        
        switch (x) {
            case '\0':
                PORTB = 0x00;
                break;
            case '1':
                PORTB = 0x01;
                break;
            case '2':
                PORTB = 0x02;
                break;
            case '3':
                PORTB = 0x03;
                break;
            case 'A':
                PORTB = 0x04;
                break;
            case '4':
                PORTB = 0x05;
                break;
            case '5':
                PORTB = 0x06;
                break;
            case '6':
                PORTB = 0x07;
                break;
            case 'B':
                PORTB = 0x08;
                break;
            case '7':
                PORTB = 0x09;
                break;
            case '8':
                PORTB = 0x0A;
                break;
            case '9':
                PORTB = 0x0B;
                break;
            case 'C':
                PORTB = 0x0C;
                break;
            case '*':
                PORTB = 0x0D;
                break;
            case '0':
                PORTB = 0x0E;
                break;
            case '#':
                PORTB = 0x0F;
                break;
            case 'D':
                PORTB = 0x10;
                break;
            default:
                PORTB = 0xF0;
                break;
        }
    }
    return 1;
}
