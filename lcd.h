/*
;File: LCD.h
;Description: A TI-provided header file to assist with LCD usage. Modified for the purposes of this
project.
;Input: ~
;Output: ~
;Author: Ben Bruzewski / TI
;Lab Section: Section 01
;Date: 11/19/2021
;--------------------------------------------------------
*/
#ifndef BOARD_LCD_H
#define BOARD_LCD_H
#include "Board.h"
// LCD Constants
#define LCD_NUM_DIGITS 7 // Number of digits on LCD
#define LCD_MEM_OFFSET 2 // Offset from LCDMEM[0]
#define LCD_MEM_LOC 11 // Num of LCDMEM[] locations used
// LCD Segments
#define LCD_A BIT0
#define LCD_B BIT1
#define LCD_C BIT2
#define LCD_D BIT3
#define LCD_E BIT6
#define LCD_F BIT4
#define LCD_G BIT5
#define LCD_H BIT7
//
// Initialize LCD
//
extern void initLCD_A(void);
//
// Clear LCD display
//
extern void clrLCD(void);
//
// Display character on LCD
//
// pos - character position on LCD
// index - index into LCD_Char_Map[] array
//
extern void dispChar(UInt8 pos, UInt8 index);
extern const UInt8 LCD_MAX_CHARS;
#endif // BOARD_LCD_H
