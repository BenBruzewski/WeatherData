/*
;File: main.c
;Description: Interfacing with UART, LED, LCD, and Switches to display weather data
;Input: User provided header file with weather appropriately formatted. (2 items in float, 9 in char, 6 in
int.)
;Output: LED, LCD display dependent on weather values. UART displays formatted detailed
descriptions. Switch toggles b/wn Temp and Precip % data on the board.
;Author: Ben Bruzewski
;Lab Section: Section 01
;Date: 11/19/2021
;--------------------------------------------------------
*/
#include <msp430.h>
#include <math.h>
#include <stdio.h>
#include <weather.h> // My file which uses data from visualcrossing.com
#include <LCD.h> // shortcuts to send particular values to the LCD screen
#include <Board.h> // definitions which help utilize LCD.h more effectively.
#include <string.h>
unsigned int mode = 0; // using this to swap between temp display and precip display
unsigned int sleep = 0; // sleep mode value set by WDT function.
unsigned int time = 0;
// Starting const definitions for the LCD to function
// The LCD functions as a seven-segment display setup, so LCD_A --> LCD_G function as parts of that display. So, 0 for example enables ABCDEF (outer ring) but not G (middle line).
const UInt8 LCD_Char_Map[] = // these values help make calling characters easier. 7-segment display with LCD_letter
{
LCD_A+LCD_B+LCD_C+LCD_D+LCD_E+LCD_F, // '0' or 'O'
LCD_B+LCD_C, // '1' or 'I'
LCD_A+LCD_B+LCD_D+LCD_E+LCD_G, // '2' or 'Z'
LCD_A+LCD_B+LCD_C+LCD_D+LCD_G, // '3'
LCD_B+LCD_C+LCD_F+LCD_G, // '4' or 'y'
LCD_A+LCD_C+LCD_D+LCD_F+LCD_G, // '5' or 'S'
LCD_A+LCD_C+LCD_D+LCD_E+LCD_F+LCD_G, // '6' or 'b'
LCD_A+LCD_B+LCD_C, // '7'
LCD_A+LCD_B+LCD_C+LCD_D+LCD_E+LCD_F+LCD_G, // '8' or 'B'
LCD_A+LCD_B+LCD_C+LCD_F+LCD_G, // '9' or 'g'
0, // I've added this spot (index = 10) as a placeholder. Explained in report. Was originally LCD_D for underscore, but 0 works as a spacer too!
LCD_A+LCD_B+LCD_C+LCD_E+LCD_F+LCD_G, // 'A'
LCD_A+LCD_D+LCD_E+LCD_F, // 'C'
LCD_B+LCD_C+LCD_D+LCD_E+LCD_G, // 'd'
LCD_A+LCD_D+LCD_E+LCD_F+LCD_G, // 'E'
LCD_A+LCD_E+LCD_F+LCD_G, // 'F'
LCD_B+LCD_C+LCD_E+LCD_F+LCD_G, // 'H'
LCD_B+LCD_C+LCD_D+LCD_E, // 'J'
LCD_D+LCD_E+LCD_F, // 'L'
LCD_A+LCD_B+LCD_E+LCD_F+LCD_G, // 'P'
LCD_B+LCD_C+LCD_D+LCD_E+LCD_F // 'U'
};
const UInt8 LCD_MAX_CHARS = (sizeof(LCD_Char_Map)/sizeof(UInt8)); // maximum value user can enter when trying to display a character
// Starting function defs for the LCD...
void dispChar(UInt8 pos, UInt8 index) // displaying a character to be shown on the LCD screen.
{
	LCDMEM[pos + LCD_MEM_OFFSET] &= ~LCD_Char_Map[8];
	if( pos < LCD_NUM_DIGITS ) // if the position and index are valid locations on the LCD screen and inside our predefined char map...
	{
		if( index < LCD_MAX_CHARS )
		{
			LCDMEM[pos + LCD_MEM_OFFSET] |= LCD_Char_Map[index]; // actually displaying the value onscreen
		}
	}
}
void initLCD_A(void)
{
// Clear LCD memory
clrLCD();
// Configure COM0-COM3 and R03-R33 pins
P5SEL |= (BIT4 | BIT3 | BIT2);
P5DIR |= (BIT4 | BIT3 | BIT2);
// Configure LCD_A
LCDACTL = LCDFREQ_128 | LCDMX1 | LCDMX0 | LCDSON | LCDON;
LCDAPCTL0 = LCDS4 | LCDS8 | LCDS12 | LCDS16 | LCDS20 | LCDS24;
LCDAPCTL1 = 0;
LCDAVCTL0 = LCDCPEN;
LCDAVCTL1 = VLCD_2_60;
}
void clrLCD(void)
{
	int i;
	for(i = LCD_MEM_OFFSET; i < (LCD_MEM_OFFSET+LCD_MEM_LOC); i++)
	{
		LCDMEM[i] = 0;
	}
}
// Starting UART function definitions:
void UART_setup()
{
	UCA0CTL1 |= UCSWRST; // set software reset
	P2SEL |= BIT4 + BIT5; // Ports 2.4 and 2.5 are USCI_A0 RXD and TXD now
	UCA0CTL1 |= UCSSEL_2; // using sys clk for BRCLK
	UCA0BR0 = 0x09; // 1,048,576 / 115,200 = 9.102222... so 9
	UCA0BR1 = 0x00; // nothing needed in upper byte
	UCA0MCTL = 0x02; // modulation because of our excess 0.1
	UCA0CTL1 &= ~UCSWRST; // Clear software reset bits at end of setup
	// end of required setup for baud rate of 115,200.
}
void UART_Send_Character(char my_char) // send a character named my_char
{
	while (!(IFG2 & UCA0TXIFG)); // if a character is waiting in the buffer, and our IFG2 flag is set
	UCA0TXBUF = my_char; // send our char to be delivered to the terminal
}
void UART_send_String(char* string)
{
	// send a string via UART using Send_Character (my char)
	unsigned int i = 0;
	unsigned int length = strlen(string); // length of the char array
	for (i = 0; i < length; i++)
{
UART_Send_Character(string[i]); // send character i of the string, and increment i.
}
	//UART_Send_Character('\n\r'); // Carriage Return to terminal once we're done (newline and clear)
}
// ShowData() will be called within our ISRs to re-send the data for the new mode the user has selected.
void showData()
{
P2OUT &= ~BIT2;
P5OUT &= ~BIT1;
P2OUT &= ~BIT1;
clrLCD();// turn all LEDs and the LCD off for new data.
float temp = WeatherF[0];
float precip = WeatherF[1]; // move the header values into local variables, temp and precip
unsigned int i = 0; // no matter the temp we should display it on the LCD
if (mode == 0) // display temperature data (default)
{
// display temp on LCD here...
if (temp < 40)
P2OUT |= BIT2; // green LED
else if (temp > 70)
P5OUT |= BIT1; // red LED
else
P2OUT |= BIT1; // orange LED
for (i = 0; i < 3; i++)
{
dispChar(4-i, WeatherI[i]); // start of the temp data piece by piece in the Weather[] array.
}
}
else if (mode == 1) // display precipitation data
{
// display precip % on LCD here...
if (precip < 33)
P2OUT |= BIT2; // green LED
else if (precip > 66)
P5OUT |= BIT1; // orange LED
else
P2OUT |= BIT1; // red LED
for (i = 0; i < 3; i++) // same as temp, just with the precipitation array now
{
dispChar(4-i, WeatherI[i+3]);
}
}
}
int main(void)
{
	IE1 = WDTIE;
	WDTCTL = WDT_ADLY_1000; // set WDT to 1000ms (1 second)
	//FLL_CTL0 |= XCAP18PF; // Set load cap for 32k xtal
	initLCD_A(); // Initialize LCD_A
	UART_setup();
	P2DIR |= BIT1 + BIT2; // configure LEDs for output
	P5DIR |= BIT1;
	_EINT(); // enable interrupts
	P1IE |= 0x03; // enable for P1.0 and P1.1 (011)
	P1IES |= 0x03;
	P1IFG &= ~0x03; // switch interrupts flags set to 0 by default
	char greetings[] = "Hello! Today's Weather for Huntsville Alabama is: \n\r";
	UART_send_String(greetings);
	char t_String[] = "Average temperature: ";
	UART_send_String(t_String);
	unsigned int i = 0;
	for (i = 0; i < 5; i++)
	{
		UART_Send_Character(WeatherC[i]); // send the temperature data!
	}
	UART_Send_Character('F');
	char p_String[] = "\n\rChance of precipitation (%) : ";
	UART_send_String(p_String);
	for (i = 0; i < 4; i++)
	{
		UART_Send_Character(WeatherC[5+i]);
	}
	UART_Send_Character('%');
	char goodbye[] = "\n\rIf you would like to toggle the MSP430's data to precipitation data, press
	SW1. Press again to toggle back to temperature.\n\rThe LEDs and LCD will time out after 60 seconds
	of inactivity. If you would like to reactivate them, tap Switch #2.";
	UART_send_String(goodbye);
	// end UART displaying, time for LEDs and LCD:
	showData(); // displaying the default data on LEDs / LCD, which is temperature data....
	_BIS_SR(LPM0 + GIE); // wait here until the user presses a switch to swap data types, or the WDT
	puts the display and LEDs to "sleep" (disable them).
}
#pragma vector = PORT1_VECTOR // Interrupt for the "sleep mode" wake-up and to toggle data b/wn temp and precip.
__interrupt void Port1_ISR (void)
{
if (P1IFG && BIT0) // P1IFG&BIT0 (Switch 1 ISR)
{
	P1IFG &= ~BIT0; // SW1 IFG cleared
	// toggle data to rainfall or temp values
	if (mode == 0) // if temp is being displayed..
	{
		mode = 1; // change to precip
		time = 0; // reset WDT since we got input.
		showData();
	}
	else if (mode == 1) // if precip is being displayed..
	{
		mode = 0; // change to temp
		time = 0;
		showData();
	}
}
if (P1IFG && BIT1) // check if IFG flag for P1.1 == true (Switch 2 ISR)
{
	P1IFG &= ~BIT1; // SW2 IFG cleared
	P1IE |= 0x03; // enable for P1.0 and P1.1 (011)
	sleep = 0; // marking that we're awake again
	time = 0; // resetting the WDT
	showData(); // need to display the data again since putting the MSP to sleep turns it all off.
	// otherwise they're just htiting S2 while it's awake which resets the WDT counter
	// reset WDT counter and wake up board
}
}
#pragma vector = WDT_VECTOR
__interrupt void watchdog_timer(void){
	if (sleep == 0) // if the MSP430 is awake...
	{
		time++; // increment every second
		if (time == 60) // once 60 seconds pass, go to sleep mode.
		{
			P2OUT &= ~BIT1;// turn all LEDs and the LCD off for sleep mode.
			P2OUT &= ~BIT2;
			P5OUT &= ~BIT1;
			P1IE &= ~BIT0; // disable interrupts on P1.0 (SW1) but NOT Switch2 because SW2 is how we "wake up".
			// disable LCD and LEDS
			sleep = 1; // toggle sleep mode so we don't keep counting while asleep.
			clrLCD();
		}
	}
}
