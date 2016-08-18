//http://www.parallax.com/product/28024
/*
 * ccifu002_lab7_part1.c
 *
 * Created: 2/23/2015 10:19:32 PM
 *  Author: Christian
 */ 
#define F_CPU 500000UL
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "C:\Users\Christian\Desktop\LCD_library\LCD.c"

#include "C:\Users\Christian\Documents\Atmel Studio\6.1\include\io.h"
#include "C:\Users\Christian\Documents\Atmel Studio\6.1\include\io.c"
#include <avr/interrupt.h>

#include <C:\Users\Christian\Documents\Atmel Studio\6.1\include\timer.h>
#include <stdio.h>

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

int rand(void);

unsigned char phrase[] = {'P','R','E','S','S',' ','D',' ','T','O',' ','S','T','A','R','T'}; //Press # to Start;
unsigned char score[] = {'*','H','I','G','H','*','*','*','C','U','R','R','E','N','T','*'}; //*High***Current*
double scale[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};	
unsigned char music[] = {'C', 'c', 'G', 'D', 'A', 'F', 'A', 'B','B','G', 'F','E', 'c', 'B','A','G','A','A','A','E','F'};
unsigned char line1[20] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};//set to blank
unsigned char line2[20] = {'>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};//set to blank
unsigned char line3[20] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};//set to blank
unsigned char line4[20] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};//set to blank


// Returns '\0' if no key pressed,
// Else returns char '1', '2', ... '9', 'A', ...
// If multiple keys pressed, returns leftmost-topmost one
// Keypad must be connected to port C
/* Keypad arrangement:

 PC4 PC5 PC6 PC7
 col 1 2 3 4
row
PC0 1 1 | 2 | 3 | A
PC1 2 4 | 5 | 6 | B
PC2 3 7 | 8 | 9 | C
PC3 4 * | 0 | # | D
*/
unsigned char GetKeypadKey() {
// Check keys in col 1
// Enable col 4 with 0, disable others with 1’s
// The delay allows PORTC to stabilize before checking
	PORTC = 0xEF;
	asm("nop");
	if (GetBit(PINC,0)==0) { return('1'); }
	if (GetBit(PINC,1)==0) { return('4'); }
	if (GetBit(PINC,2)==0) { return('7'); }
	if (GetBit(PINC,3)==0) { return('*'); }
		
	// Check keys in col 2
	// Enable col 5 with 0, disable others with 1’s
	// The delay allows PORTC to stabilize before checking
	PORTC = 0xDF;
	asm("nop");
	if (GetBit(PINC,0)==0) { return('2'); }
	if (GetBit(PINC,1)==0) { return('5'); }
	if (GetBit(PINC,2)==0) { return('8'); }
	if (GetBit(PINC,3)==0) { return('0'); }
	// ... *****FINISH*****
	
	PORTC = 0xBF;
	asm("nop");
	if (GetBit(PINC,0)==0) { return('3'); }
	if (GetBit(PINC,1)==0) { return('6'); }
	if (GetBit(PINC,2)==0) { return('9'); }
	if (GetBit(PINC,3)==0) { return('#'); }
	// ... *****FINISH*****
	// Check keys in col 3
	// Enable col 6 with 0, disable others with 1’s
	// The delay allows PORTC to stabilize before checking
	PORTC = 0x7F;
	asm("nop");
	if (GetBit(PINC,0)==0) { return('A'); }
	if (GetBit(PINC,1)==0) { return('B'); }
	if (GetBit(PINC,2)==0) { return('C'); }
	if (GetBit(PINC,3)==0) { return('D'); }
	// ... *****FINISH*****
	// Check keys in col 4
	// ... *****FINISH*****
	return('\0'); // default value
}
//--------Find GCD function -------------------------------
unsigned long int findGCD (unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}
//--------End find GCD function ---------------------------
//--------Task scheduler data structure--------------------
// Struct for Tasks represent a running process in our
// simple real-time operating system.
/*Tasks should have members that include: state, period, a
measurement of elapsed time, and a function pointer.*/
typedef struct _task {
	//Task's current state, period, and the time elapsed
	// since the last tick
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	//Task tick function
	int (*TickFct)(int);
} task;

void set_PWM(double frequency) {
	
	if (!frequency) TCCR3B &= 0x08; //stops timer/counter
	else TCCR3B |= 0x03; // resumes/continues timer/counter
	
	// 0.954 is smallest frequency that will not result in overflow
	if (frequency < 0.954) OCR3A = 0xFFFF;
	
	// prevents OCR3A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
	else if (frequency > 31250) OCR3A = 0x0000;
	
	// set OCR3A based on desired frequency
	else OCR3A = (short)(8000000 / (128 * frequency)) - 1;

	TCNT3 = 0; // resets counter
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}
//--------Shared Variables---------------------------------
unsigned char key;
unsigned char GameBegun = 0;
unsigned char enemyC = 0;
unsigned char CurrentEnemeyCount = 0;
unsigned char RNG = 0;
unsigned char row = 2;
//--------End Shared Variables-----------------------------

//--------User defined FSMs--------------------------------
enum SM1_States { SM1_wait, SM1_Print, SM1_GS,SM1_GS_set, SM1_END };
	// Displays info about the game. 
int SMTick1(int state) {
	// Local Variables
	unsigned char screensize = 16;
	static unsigned char position = 0;
	//State machine transitions
	switch (state) {
		// Wait for button press
		case SM1_wait:
			state = SM1_Print;
			break;
		// Button remains pressed
		case SM1_Print:
			if(key == 'D')
			{	
				key = '\0';
				state = SM1_GS_set;
				GameBegun = 1;
			}
			else 
				state = SM1_Print;
			break;
		case SM1_GS_set:
			state = SM1_GS;
			break;
		case SM1_GS:
			state = SM1_GS;
			break;
		// default: Initial state
		default:
			state = SM1_wait;
			break;
	}
	//State machine actions
	switch(state) {
		case SM1_Print:
				for(unsigned char i = 0; i < screensize;)
				{
					unsigned char c = phrase[i];
					LCD_Cursor(i+1);
					LCD_WriteData(c);
					++position;
					++i;
				}
			break;
			case SM1_GS_set:
				for(unsigned char i = 0; i < screensize;)
				{
					unsigned char c = score[i];
					LCD_Cursor(i+1);
					LCD_WriteData(c);
					++position;
					++i;
				}
				LCD_Cursor(18);
				LCD_WriteData('1');
				LCD_Cursor(19);
				LCD_WriteData('0');
				LCD_Cursor(20);
				LCD_WriteData('0');
				LCD_Cursor(25);
				LCD_WriteData('0');
				break;
			case SM1_GS:
			//CUrrent
			LCD_Cursor(25);
			
			break;
			
		default: break;
	}
	return state;
}

//gets key presses.			
enum SM2_States{Start, GETK};

int SMTick2(int state)
{
	unsigned char temp;
	switch(state)
	{
		case Start:
			state = GETK;
			break;
		case GETK:
			state = GETK;
			temp = GetKeypadKey();
			if(temp != '\0')
				key = temp;
			break;
		default:
		state = Start;
		break;
	}
	return state;
}


//handles music
enum SM3_States{Begin, Play};

int SMTick3(int state)
{
	static unsigned char y = 0;// where we where in relation to music;
	unsigned char len = 21;
	switch (state)
	{
		case Begin:
		state = Play;
		break;
		
		case Play:
			state = Play;
				if(y >=len)
					y = 0;
				switch(music[y])
				{
					case 'C':
					set_PWM(scale[0]);
					break;
					
					case 'D':
					set_PWM(scale[1]);
					break;
					
					case 'E':
					set_PWM(scale[2]);
					break;
					
					case 'F':
					set_PWM(scale[3]);
					break;
					
					case 'G':
					set_PWM(scale[4]);
					break;
					
					case 'A':
					set_PWM(scale[5]);
					break;
					
					case 'B':
					set_PWM(scale[6]);
					break;
					
					case 'c':
					set_PWM(scale[7]);
					break;
				}
				++y;
			break;
			
		default:
		state = Begin;
		break;
	}
	return state;
}

//handles enemy generation. 
enum SM4_States{SM4_start, SM4_wait, SM4_generate};

int SMTick4(int state)
{
	switch (state)
	{
		case SM4_start:
		state = SM4_wait;
		break;
		case SM4_wait:
			if(GameBegun != 1)
				state = SM4_wait;
			else
				state = SM4_generate;
			break;
		case SM4_generate:
			if(GameBegun == 1)
				{	
					state = SM4_generate;
					if(rand()% 7 == 0)
						enemyC = 1;
				}
			else
				state = SM4_wait;
			
			break;
		default:
			state = SM4_start;
			break;
		
	}
	
	return state;
}

//handles enemy movement
enum SM5_states{SM5_start, SM5_wait, SM5_move};
	
int SMTick5(int state)
{
	char row;
	switch (state)
	{
		case SM5_start:
			state = SM5_wait;
			break;
		case SM5_wait:
			if(GameBegun == 0)
				state = SM5_wait;
			else 
				state = SM5_move;
			break;
		case SM5_move:
			if(GameBegun == 0)
				state = SM5_wait;
			else
				state = SM5_move;
				{
					if(enemyC == 1 && CurrentEnemeyCount < 25)
					{
						row = (rand()+7)%4;
						++CurrentEnemeyCount;
						if(row == 0)
						{
							line1[18] = 'I';
						}
						if(row == 1)
						{
							line2[18] = 'I';
						}
						if(row == 2)
						{
							line3[18] = 'I';
						}
						if(row == 3)
						{
							line4[18] = 'I';
						}
						enemyC = 0;
					}
			}
		
		default:
		state = SM5_start;
		break;
	}
	return state;
}

enum SM6_states{SM6_start, SM6_wait, SM6_generate};

int SMTick6(int state)// works the second screen
{
	char len = 20;
	switch(state)
	{
		case SM6_start:
			lcd_clrscr();
			state = SM6_wait;
			break;
		case SM6_wait:
			if(GameBegun != 1)
				state = SM6_wait;
			else 
				state = SM6_generate;
			break;
		case SM6_generate:
		if(GameBegun)	
		{
			state = SM6_generate;
				
				lcd_gotoxy(0,0);
				for(int i = 0; i < len; ++i)
					lcd_putc(line1[i]);
				lcd_gotoxy(0,1);
				for(int i = 0; i < len; ++i)
					lcd_putc(line2[i]);
				lcd_gotoxy(0,2);
				for(int i = 0; i < len; ++i)
					lcd_putc(line3[i]);
				lcd_gotoxy(0,3);
				for(int i = 0; i < len; ++i)
					lcd_putc(line4[i]);
		}
		else
		{
			lcd_clrscr();
			state = SM6_wait;
		}	
		break;
		default:
			state = SM6_start;
			break;
	}
	return state;
}


enum SM7_states{SM7_start, SM7_wait, SM7_move};
	
//move the values in the arrays	
int SMTick7(int state)
{
	switch(state)
	{
		case SM7_start:
			state = SM7_wait;
			break;
		
		case SM7_wait:
			if(!GameBegun)
				state = SM7_wait;
			else
				state = SM7_move;
			break;
		case SM7_move:
			if(GameBegun)
			{
				state = SM7_start;
				for(int i = 2;i < 20; ++i)
				{
					if(line1[i] == 'I')
					{
							line1[i-1] = line1[i];
							line1[i] = ' ';
					}
				}
				for(int i =2;i < 20; ++i)
				{
					if(line2[i] == 'I')
					{
							line2[i-1] = line2[i];
							line2[i] = ' ';
					}
				}
				for(int i = 2;i < 20; ++i)
				{
						if(line3[i] == 'I')
						{
							line3[i-1] = line3[i];
							line3[i] = ' ';
						}
				}
				for(int i = 2;i < 20; ++i)
				{
						if(line4[i] == 'I')
						{
							line4[i-1] = line4[i];
							line4[i] = ' ';
						}
				}				
				if(line1[1] == 'I')
				{
					--CurrentEnemeyCount;
					line1[1] = ' ';
				}
				if(line2[1] == 'I')
				{
					--CurrentEnemeyCount;
					line2[1] = ' ';
				}
				if(line3[1] == 'I')
				{
					--CurrentEnemeyCount;
					line3[1] = ' ';
				}
				if(line4[1] == 'I')
				{
					--CurrentEnemeyCount;
					line4[1] = ' ';
				}
				
			}
			else
				state = SM7_wait;
		default:
			state = SM7_start;
			break;
	}
	
	return state;
}

enum SM8States{SM8_start, SM8_wait, SM8_move};

//moves player ship
int SMTick8(int state)
{
	switch(state)
	{
		case SM8_start:
			state = SM8_wait;
			break;
		
		case SM8_wait:
			if(!GameBegun)
				state = SM8_wait;
			else
				state = SM8_move;
		case SM8_move:
			if(GameBegun)
			{
				state = SM8_move;
				if(key == 'D')
				{
					if(row == 1)
						line1[1] = '-';
					else if(row == 2)
						line2[1] = '-';
					else if(row == 3)
						line3[1] = '-';
					else if(row == 4)
						line4[1] = '-';
				}
				if(key == '*' && row >1)
					{
						if(row == 2)
						{
							line2[0] = ' ';
							line1[0] = '>';
							row--;
						}
						else if(row == 3)
						{
							line3[0] = ' ';
							line2[0] = '>';
							row--;
						}
						else if(row == 4)
						{
							line4[0] = ' ';
							line3[0] = '>';
							row--;
						}
						key = '\0';	
					}
				if(key == '0' && row <4)
					{
						if(row == 1)
						{
							line1[0] = ' ';
							line2[0] = '>';
							row++;
						}
						else if(row == 2)
						{
							line2[0] = ' ';
							line3[0] = '>';
							row++;
						}
						else if(row == 3)
						{
							line3[0] = ' ';
							line4[0] = '>';
							row++;
						}
					key = '\0';
					}
			}
			else
				state = SM8_wait;		
		default:
			state = SM8_start;
			break;
	}
	
	return state;
}

int main()
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	PWM_on();
	// . . . etc
	// Period for the tasks
	unsigned long int SMTick1_calc = 100;
	unsigned long int SMTick2_calc = 100;
	unsigned long int SMTick3_calc = 200;
	unsigned long int SMTick4_calc = 100;
	unsigned long int SMTick5_calc = 100;
	unsigned long int SMTick6_calc = 50;
	unsigned long int SMTick7_calc = 100;
	unsigned long int SMTick8_calc = 200;
	//Calculating GCD
	unsigned long int tmpGCD = 50;
	//Greatest common divisor for all tasks
	// or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD; 
	unsigned long int SMTick4_period = SMTick4_calc/GCD;
	unsigned long int SMTick5_period = SMTick5_calc/GCD;
	unsigned long int SMTick6_period = SMTick6_calc/GCD;
	unsigned long int SMTick7_period = SMTick7_calc/GCD;
	unsigned long int SMTick8_period = SMTick8_calc/GCD;
	//Declare an array of tasks
	static task task1, task2, task3, task4, task5, task6, task7, task8;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	// Task 1
	task1.state = -1;
	task1.period = SMTick1_period;
	task1.elapsedTime = SMTick1_period;
	task1.TickFct = &SMTick1;
	/////////////////////////
	task2.state = -1;
	task2.period = SMTick2_period;
	task2.elapsedTime = SMTick2_period;
	task2.TickFct = &SMTick2;
	// Task 2
	task3.state = -1;
	task3.period = SMTick3_period;
	task3.elapsedTime = SMTick3_period;
	task3.TickFct = &SMTick3;
	// Task 3
	
	task4.state = -1;
	task4.period = SMTick4_period;
	task4.elapsedTime = SMTick4_period;
	task4.TickFct = &SMTick4;
	// Task 4
	
	task5.state = -1;
	task5.period = SMTick5_period;
	task5.elapsedTime = SMTick5_period;
	task5.TickFct = &SMTick5;
	// Task 5
	task6.state = -1;
	task6.period = SMTick6_period;
	task6.elapsedTime = SMTick6_period;
	task6.TickFct = &SMTick6;
	// Task 6
	
	task7.state = -1;
	task7.period = SMTick7_period;
	task7.elapsedTime = SMTick7_period;
	task7.TickFct = &SMTick7;
	//task7
	
	task8.state = -1;
	task8.period = SMTick8_period;
	task8.elapsedTime = SMTick8_period;
	task8.TickFct = &SMTick8;
	////////////
	LCD_init();
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	
	TimerSet(GCD);
	TimerOn();
	
	// Scheduler for-loop iterator
	unsigned short i;
	while(1) {
			
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime ==
			tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state =
				tasks[i]->TickFct(tasks[i]->state);
				// Reset elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	// Error: Program should not exit!
}
