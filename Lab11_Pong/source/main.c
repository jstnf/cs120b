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

unsigned char paddles_pos = 0x33; // [7:4] is P1 (left) pos; [3:0] is P2 (right) pos - 2 is top position, 3 is middle position, 4 is bottom position

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

// Use the current ball_vel and ball_pos to get the next position of the ball
unsigned char getNextPos() {
    unsigned char result = 0x00;
    
    // Horizontal movement
    unsigned char x = ball_pos >> 4;
    if (ball_vel & 0x40) {  // Move right
        x++;
    } else {                // Move left
        x--;
    }
    result = result | (x << 4);
    
    // Vertical movement
    switch (ball_vel & 0x0F) {
        case 0x00:          // Move up
            result = result | ((ball_pos & 0x0F) - 1);
            break;
        case 0x01:          // Move straight
            result = result | (ball_pos & 0x0F);
            break;
        case 0x02:          // Move down
            result = result | ((ball_pos & 0x0F) + 1);
            break;
    }
    
    return result;
}

// Return 0 if there is a win, 1 if the ball should head upward, 2 if the ball should head downward, 3 if the ball should head straight
unsigned char checkWinPosition(unsigned char yPos, unsigned char paddlePos) {
    // Check if the ball's y pos and the center pos match - if so we can just return 3 (moves straight back)
    if (yPos == paddlePos) return 0x03;
    
    // Now check for win conditions
    // If there is a difference of at least 2 between the paddlePos and yPos, we have a win
    unsigned char diff;
    if (paddlePos > yPos) {
        diff = paddlePos - yPos;
        if (diff == 0x01) return 0x01; // Move upward
        else return 0x00; // Win condition
    } else {
        diff = yPos - paddlePos;
        if (diff == 0x01) return 0x02; // Move downward
        else return 0x00; // Win condition
    }
}

void PongTick() {
    // The first tick will reset the game to starting state, but maintain score
    if (pongGameTick == 1) {
        roundResetGame();
    }
    
    // Get unchecked next position
    unsigned char nextPos = getNextPos();
    
    // First check if the ball is going out of bounds - if so, we need to reverse its velocity
    if ((nextPos & 0x0F) == 0x00) { // The ball is heading upward, needs to be sent downward
        ball_vel = (ball_vel & 0xF0) | 0x02;
    } else if ((nextPos & 0x0F) == 0x06) { // The ball is heading downward, needs to be sent upward
        ball_vel = (ball_vel & 0xF0);
    }
    
    // With new velocity, recheck next position
    nextPos = getNextPos();
    
    // Check if the ball is entering either paddle zone - we need to check if that space is occupied by a paddle if so
    // We convert the ball's position and use the position of a specific paddle to check for a win condition
    unsigned char isolatedYPos = nextPos & 0x0F;
    unsigned char winCondition = 0x00; // Keep this at 0x00 if no one wins, 0x01 if player 1 (left) wins, 0x02 if player 2 (right) wins
    if ((nextPos & 0xF0) == 0x00) { // The ball has reached the left side, isolate the left paddle position
        switch (checkWinPosition(isolatedYPos, paddles_pos >> 4)) {
            case 0x00: // We have a winner
                winCondition = 0x02;
                break;
            
            // No one wins, update the x and y velocity and calculate new position of ball (now moving right)
            
            case 0x01: // Ball now moves up left
                ball_vel = 0x11;
                break;
            case 0x02: // Ball now moves down left
                ball_vel = 0x12;
                break;
            case 0x03: // Ball now moves straight left
                ball_vel = 0x10;
                break;
        }
    } else if ((nextPos & 0xF0) == 0x70) { // The ball has reached the right side, isolate the right paddle position
        switch (checkWinPosition(isolatedYPos, paddles_pos & 0x0F)) {
            case 0x00: // We have a winner
                winCondition = 0x02;
                break;
            
            // No one wins, update the x and y velocity and calculate new position of ball (now moving right)
            
            case 0x01: // Ball now moves up right
                ball_vel = 0x01;
                break;
            case 0x02: // Ball now moves down right
                ball_vel = 0x02;
                break;
            case 0x03: // Ball now moves straight right
                ball_vel = 0x00;
                break;
        }
    }
    
    switch (winCondition) {
        default:
            // Recheck y velocity since we updated it before using only paddle position information
            // This ensures the ball stays inbounds
            // We can just use our old code
            nextPos = getNextPos();
            if ((nextPos & 0x0F) == 0x00) { // The ball is heading upward, needs to be sent downward
                ball_vel = (ball_vel & 0xF0) | 0x02;
            } else if ((nextPos & 0x0F) == 0x06) { // The ball is heading downward, needs to be sent upward
                ball_vel = (ball_vel & 0xF0);
            }
            
            // Finally, update ball position!
            ball_pos = getNextPos();
            break;
        case 0x01:
        case 0x02:
            // TODO code win condition
            pongGameTick = 0;
            break;
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

unsigned char active = 0x00; // 0x00 is inactive, 0x01 is active - this determines if the game should tick or not

// SM for the game tick
enum SM3_STATES { SM3_NotPlaying, SM3_Playing };
int TickFct_PongTick(int state) {
    switch (state) {
        default:
            if (active) state = SM3_Playing;
            else state = SM3_NotPlaying;
            break;
    }
    
    switch (state) {
        default:
            break;
        case SM3_Playing:
            pongGameTick++;
            PongTick();
            break;
    }
    
    return state;
}

// SM to control game states
enum SM2_STATES { SM2_MenuRise, SM2_MenuFall, SM2_PlayingRise, SM2_PlayingFall };
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
            active = 0x00;
            break;
        case SM2_PlayingFall:
            PongDraw();
            active = 0x01;
            break;
        default:
            drawStartScreen();
            active = 0x00;
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
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    /* Insert your solution below */
    static task task0, task1, task2, task3;
    task *tasks[] = { &task3, &task2, &task1, &task0 }; // Task execution order
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
    
    task3.state = start;
    task3.period = 333;
    task3.elapsedTime = task3.period;
    task3.TickFct = &TickFct_PongTick;
    
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