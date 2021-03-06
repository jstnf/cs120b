/*	Author: jfigu042
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Lab #11 Exercise #1
 *	Exercise Description: Class Pong Game Implementation
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Basic Req: https://youtu.be/Ra6260dWJCc
 *  Advancements: https://youtu.be/PbXVueId6jc
 */
#include <avr/io.h>
#include "pwm.h"
#include "timer.h"
#include "scheduler.h"
#include "stdint.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#define HITNOTE 196.00
#define WINNOTE 293.66

// Sound system
double currentNote = 0;
unsigned char playTicks = 0x01;

enum SM6_STATES { SM6_Wait, SM6_Playing };
int TickFct_SoundPlayer(int state) {
    switch (state) {
        default:
            if (playTicks > 0x01) state = SM6_Playing;
            else state = SM6_Wait;
            break;
        case SM6_Playing:
            playTicks = playTicks - 1 == 0x00 ? 0x01 : playTicks - 1;
            if (playTicks == 0x01) {
                state = SM6_Wait;
            }
            break;
    }
    
    switch (state) {
        default:
            set_PWM(0);
            break;
        case SM6_Playing:
            set_PWM(currentNote);
            break;
    }
    
    return state;
}

void hitSound() {
    currentNote = HITNOTE;
    playTicks = 0x03;
}

void winSound() {
    currentNote = WINNOTE;
    playTicks = 0x0D;
}

// RNG (from C standard, see https://stackoverflow.com/questions/4768180/rand-implementation)
static unsigned long int next = 1;

int rand(void) { // RAND_MAX assumed to be 32767
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
    next = seed;
}

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

unsigned char ball_vel = 0x01; // [4] is X vel - 0 moves left, 1 moves right; [1:0] is Y vel - 0 ball moves up, 1 ball moves straight, 2 ball moves down
unsigned char ball_pos = 0x33; // [7:4] is X pos; [3:0] is Y pos - 0 is top border, game area 1-5, 6 is bottom border
unsigned char ball_speed = 0x04; // Can range between 0 and 4, simply change how often the game ticks
unsigned char paddles_pos = 0x33; // [7:4] is P1 (left) pos; [3:0] is P2 (right) pos - 2 is top position, 3 is middle position, 4 is bottom position

unsigned char spin = 0x11; // This variable is changed when a paddle moves - [7:4] for P1; [3:0] for P2; 0 - spin up, 1 - no spin, 2 - spin down

// GAME LOGIC
void roundResetGame() {
    ball_vel = 0x01;
    ball_pos = 0x33;
    ball_speed = 0x04;
    paddles_pos = 0x33;
}

void fullResetGame() {
    scoreP1 = 0x00;
    scoreP2 = 0x00;
    pongGameTick = 0;
    roundResetGame();
}

// Use the current ball_vel and ball_pos to get the next position of the ball
unsigned char getNextPos() {
    unsigned char result = 0x00;
    
    // Horizontal movement
    unsigned char x = ball_pos >> 4;
    if (ball_vel & 0xF0) {  // Move right
        x++;
    } else {                // Move left
        x--;
    }
    result = x << 4;
    
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

void increaseBallSpeed() {
    ball_speed = ball_speed - 1 == 0x00 ? 0x01 : ball_speed - 1;
}

void decreaseBallSpeed() {
    ball_speed = ball_speed + 1 == 0x05 ? 0x04 : ball_speed + 1;
}

// Spin logic 
void doSpinP1() {
    switch (spin & 0xF0) {
        case 0x00: // Spin up
            increaseBallSpeed();
            ball_vel = 0x10;
            break;
        case 0x20: // Spin down
            increaseBallSpeed();
            ball_vel = 0x12;
            break;
        default: // No spin
            break;
    }
}

void doSpinP2() {
    switch (spin & 0x0F) {
        case 0x00: // Spin up
            increaseBallSpeed();
            ball_vel = 0x00;
            break;
        case 0x02: // Spin down
            increaseBallSpeed();
            ball_vel = 0x02;
            break;
        default: // No spin
            break;
    }
}

// Function variable for speed
unsigned char ticksUntilNext = 0x04;

void PongTick() {
    // The first tick will reset the game to starting state, but maintain score
    if (pongGameTick == 1) {
        ticksUntilNext = 0x04;
        roundResetGame();
    }
    
    if (ticksUntilNext > 0x00) {
        ticksUntilNext--;
    }
    
    // Get unchecked next position
    unsigned char nextPos = getNextPos();
    
    // First check if the ball is going out of bounds - if so, we need to reverse its velocity
    if ((nextPos & 0x0F) == 0x00) { // The ball is heading upward, needs to be sent downward
        ball_vel = (ball_vel & 0xF0) | 0x02;
        hitSound();
    } else if ((nextPos & 0x0F) == 0x06) { // The ball is heading downward, needs to be sent upward
        ball_vel = (ball_vel & 0xF0);
        hitSound();
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
            
            case 0x01: // Ball now moves up right
                ball_vel = 0x10;
                increaseBallSpeed();
                doSpinP1();
                ticksUntilNext = ball_speed;
                hitSound();
                break;
            case 0x02: // Ball now moves down right
                ball_vel = 0x12;
                decreaseBallSpeed();
                doSpinP1();
                ticksUntilNext = ball_speed;
                hitSound();
                break;
            case 0x03: // Ball now moves straight right
                ball_vel = 0x11;
                increaseBallSpeed();
                doSpinP1();
                ticksUntilNext = ball_speed;
                hitSound();
                break;
        }
    } else if ((nextPos & 0xF0) == 0x70) { // The ball has reached the right side, isolate the right paddle position
        switch (checkWinPosition(isolatedYPos, paddles_pos & 0x0F)) {
            case 0x00: // We have a winner
                winCondition = 0x02;
                break;
            
            // No one wins, update the x and y velocity and calculate new position of ball (now moving right)
            
            case 0x01: // Ball now moves up left
                ball_vel = 0x00;
                increaseBallSpeed();
                doSpinP2();
                ticksUntilNext = ball_speed;
                hitSound();
                break;
            case 0x02: // Ball now moves down left
                ball_vel = 0x02;
                decreaseBallSpeed();
                doSpinP2();
                ticksUntilNext = ball_speed;
                hitSound();
                break;
            case 0x03: // Ball now moves straight left
                ball_vel = 0x01;
                increaseBallSpeed();
                doSpinP2();
                ticksUntilNext = ball_speed;
                hitSound();
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
                hitSound();
            } else if ((nextPos & 0x0F) == 0x06) { // The ball is heading downward, needs to be sent upward
                ball_vel = (ball_vel & 0xF0);
                hitSound();
            }
            
            // Finally, update ball position!
            ball_pos = getNextPos();
            break;
        case 0x01:
        case 0x02:
            // TODO code win condition
            roundResetGame();
            pongGameTick = 0;
            winSound();
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

void moveP1PaddleUp() {
    unsigned char isolatedPaddlePos = paddles_pos >> 4;
    isolatedPaddlePos = (isolatedPaddlePos - 1 == 0x01 ? 0x02 : isolatedPaddlePos - 1) << 4;
    paddles_pos = (paddles_pos & 0x0F) | isolatedPaddlePos;
    spin = (spin & 0x0F) | 0x00;
}

void moveP1PaddleDown() {
    unsigned char isolatedPaddlePos = paddles_pos >> 4;
    isolatedPaddlePos = (isolatedPaddlePos + 1 == 0x05 ? 0x04 : isolatedPaddlePos + 1) << 4;
    paddles_pos = (paddles_pos & 0x0F) | isolatedPaddlePos;
    spin = (spin & 0x0F) | 0x20;
}

void moveP2PaddleUp() {
    unsigned char isolatedPaddlePos = paddles_pos & 0x0F;
    isolatedPaddlePos = (isolatedPaddlePos - 1 == 0x01 ? 0x02 : isolatedPaddlePos - 1);
    paddles_pos = (paddles_pos & 0xF0) | isolatedPaddlePos;
    spin = (spin & 0xF0) | 0x00;
}

void moveP2PaddleDown() {
    unsigned char isolatedPaddlePos = paddles_pos & 0x0F;
    isolatedPaddlePos = (isolatedPaddlePos + 1 == 0x05 ? 0x04 : isolatedPaddlePos + 1);
    paddles_pos = (paddles_pos & 0xF0) | isolatedPaddlePos;
    spin = (spin & 0xF0) | 0x02;
}

unsigned char active = 0x00; // 0x00 is inactive, 0x01 is active - this determines if the game should tick or not

// SM for Player controls
unsigned char maintainSpinTicks = 0x01;

enum SM4_STATES { SM4_Wait, SM4_UpRise, SM4_UpFall, SM4_DownRise, SM4_DownFall };
int TickFct_PlayerControl(int state) {
    if (active == 0x00) return SM4_Wait; // Reset this if the game is not active
    
    switch (state) {
        default:
            if ((~PINA & 0x03) == 0x01) { // Move up
                state = SM4_UpRise;
            } else if ((~PINA & 0x03) == 0x02) { // Move down
                state = SM4_DownRise;
            }
            break;
        case SM4_UpRise:
            state = SM4_UpFall;
            break;
        case SM4_UpFall:
            if ((~PINA & 0x01) == 0x00) state = SM4_Wait;
            break;
        case SM4_DownRise:
            state = SM4_DownFall;
            break;
        case SM4_DownFall:
            if ((~PINA & 0x02) == 0x00) state = SM4_Wait;
            break;
    }
    
    maintainSpinTicks = maintainSpinTicks - 1 == 0x00 ? 0x01 : maintainSpinTicks - 1;
    
    switch (state) {
        case SM4_UpRise:
            moveP1PaddleUp();
            maintainSpinTicks = 0x0A; // Add cooldown to maintain spin
            break;
        case SM4_DownRise:
            moveP1PaddleDown();
            maintainSpinTicks = 0x0A; // Add cooldown to maintain spin
            break;
        default:
            if (maintainSpinTicks == 0x01) spin = (spin & 0x0F) | 0x10;
            break;
    }
    
    return state;
}

// SM for CPU
unsigned char cpuControl = 0x01;
enum SM5_STATES { SM5_Wait, SM5_Random, SM5_Follow };
int TickFct_CPUControl(int state) {
    if (active == 0x00) {
        cpuControl = 0x01;
        return SM5_Wait;
    }
    
    switch (state) {
        default:
            cpuControl--;
            if (cpuControl == 0x00) {
                state = rand() % 3;
                cpuControl = (char) (rand() % 4) + 1;
            }
            // Randomly choose one of the three states and how long they should stay in that state (1-4 ticks @ 200ms/tick)
            break;
    }
    
    unsigned char yPos;
    unsigned char paddlePos;
    
    switch (state) {
        case SM5_Follow:
            yPos = ball_pos & 0x0F;
            paddlePos = paddles_pos & 0x0F;
            // If there is a difference between the ball pos and p2 pos greater than 0, move in the correct direction
            if (paddlePos > yPos) {
                moveP2PaddleUp();
            } else if (paddlePos < yPos) {
                moveP2PaddleDown();
            } else {
                spin = (spin & 0xF0) | 0x01;
            }
            break;
        case SM5_Random:
            if (rand() & 2) {
                moveP2PaddleDown();
            } else {
                moveP2PaddleUp();
            }
            break;
        default:
            // Do nothing :)
            spin = (spin & 0xF0) | 0x01;
            break;
    }
    
    return state;
}

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
    // This will keep shuffling rand in order to somewhat guarantee randomness
    rand();
    
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
    DDRB = 0x40; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    /* Insert your solution below */
    static task task0, task1, task2, task3, task4, task5, task6;
    task *tasks[] = { &task4, &task5, &task3, &task2, &task1, &task0, &task6 }; // Task execution order
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
    task3.period = 100;
    task3.elapsedTime = task3.period;
    task3.TickFct = &TickFct_PongTick;
    
    task4.state = start;
    task4.period = 20;
    task4.elapsedTime = task4.period;
    task4.TickFct = &TickFct_PlayerControl;
    
    task5.state = start;
    task5.period = 200;
    task5.elapsedTime = task5.period;
    task5.TickFct = &TickFct_CPUControl;
    
    task6.state = start;
    task6.period = 15;
    task6.elapsedTime = task6.period;
    task6.TickFct = &TickFct_SoundPlayer;
    
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