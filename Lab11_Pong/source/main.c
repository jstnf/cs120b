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

// "2D Matrix" contains patterns for each row
unsigned char row1 = 0x00;
unsigned char row2 = 0x00;
unsigned char row3 = 0x00;
unsigned char row4 = 0x00;
unsigned char row5 = 0x00;

// Pong implementation
unsigned char scoreP1 = 0x00;
unsigned char scoreP2 = 0x00;
unsigned int pongGameTick = 0;

unsigned char ball_vel = 0x11; // [4] is X vel - 0 moves left, 1 moves right; [1:0] is Y vel - 0 ball moves up, 1 ball moves straight, 2 ball moves down
unsigned char ball_pos = 0x33; // [7:4] is X pos; [3:0] is Y pos - 0 is top border, game area 1-5, 6 is bottom border

unsigned char paddles_pos = 0x33; // [7:4] is P1 pos; [3:0] is P2 pos - 2 is top position, 3 is middle position, 4 is bottom position

// GAME LOGIC
void fullResetGame() {
    scoreP1 = 0x00;
    scoreP2 = 0x00;
    pongGameTick = 0;
}

void roundResetGame() {
    ball_vel = 0x11;
    ball_pos = 0x33;
    paddles_pos = 0x33;
}

void PongTick(int tickNum) {
    // The first tick will reset the game to starting state, but maintain score
    if (tick == 1) {
        roundResetGame();
    }
}

// LED LOGIC
void clearCanvas() {
    row1 = 0x00;
    row2 = 0x00;
    row3 = 0x00;
    row4 = 0x00;
    row5 = 0x00;
}

// Clear the canvas before you call this method!
void drawPaddles() {
    // In all situations we draw the middle dot on both sides
    row3 = row3 | 0x81; // O******O
    
    // Left paddle
    switch (paddles_pos & 0xF0) {
        case 0x20:
            row1 = row1 | 0x80;
            row2 = row2 | 0x80;
            break;
        case 0x30:
            row2 = row2 | 0x80;
            row4 = row4 | 0x80;
            break;
        case 0x40:
            row4 = row4 | 0x80;
            row5 = row5 | 0x80;
            break;
    }
    
    // Right paddle
    switch (paddles_pos & 0x0F) {
        case 0x02:
            row1 = row1 | 0x01;
            row2 = row2 | 0x01;
            break;
        case 0x03:
            row2 = row2 | 0x01;
            row4 = row4 | 0x01;
            break;
        case 0x04:
            row4 = row4 | 0x01;
            row5 = row5 | 0x01;
            break;
    }
}

// Clear the canvas before you call this method!
void drawBall() {
    unsigned char ballPattern = 0x80;
    unsigned char xPos = ball_pos >> 4;
    unsigned char i;
    for (i = 0; i < xPos; i++) {
        ballPattern = ballPattern >> 1;
    }
    
    switch (ball_pos & 0x0F) { // Determine which row to draw to
        case 0x01:
            row1 = row1 | ballPattern;
            break;
        case 0x02:
            row2 = row2 | ballPattern;
            break;
        case 0x03:
            row3 = row3 | ballPattern;
            break;
        case 0x04:
            row4 = row4 | ballPattern;
            break;
        case 0x05:
            row5 = row5 | ballPattern;
            break;
        default:
            break;
    }
}

void PongDraw() {
    clearCanvas();
    drawBall();
    drawPaddles();
}

void drawStartScreen() {
    row1 = 0x00; // ********
    row2 = 0x24; // **O**O**
    row3 = 0x00; // ********
    row4 = 0x42; // *O****O*
    row5 = 0x3C; // **OOOO**
}

// SM to control game states
enum SM2_STATES { SM2_MenuRise, SM2_MenuFall, SM2_PlayingRise, SM2_PlayingFall }
int TickFct_GameState(int state) {
    switch (state) {
        default:
            if ((~PINA & 0x80) == 0x00) state = SM2_MenuFall;
            else state = SM2_MenuRise;
            break;
        case SM2_MenuFall:
            if (~PINA & 0x80) {
                state = SM2_PlayingRise;
                
                // Only want to reset these values on transition
                fullResetGame();
            }
            break;
        case SM2_PlayingRise:
            if ((~PINA & 0x80) == 0x00) state = SM2_PlayingFall;
            break;
        case SM2_PlayingFall:
            if (~PINA & 0x80) state = SM2_MenuRise;
            break;
    }
    
    switch (state) {
        case SM2_PlayingRise:
            PongDraw();
            break;
        case SM2_PlayingFall:
            pongGameTick++;
            PongTick(pongGameTick);
            PongDraw();
            break;
        default:
            drawStartScreen();
            break;
    }
    
    return state;
}

// Global variables controlling to which row and what pattern to draw
unsigned char row = 0x01;
unsigned char pattern = 0x00;

// SM to alternate row and pattern drawn to board, sets up variables for task0 to output
enum SM1_STATES { SM1_SMStart, SM1_R1, SM1_R2, SM1_R3, SM1_R4, SM1_R5 };
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
    DDRA = 0x00; PORTA = 0x11;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    /* Insert your solution below */
    static task task0, task1, task2;
    task *tasks[] = { &task2, &task1, &task0 }; // Task execution order
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
    
    task2.state = start;
    task2.period = 1;
    task2.elapsedTime = task2.period;
    task2.TickFct = &TickFct_GameState;
    
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