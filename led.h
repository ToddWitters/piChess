#ifndef LED_H
#define LED_H

#include "types.h"


#define DECODE_MODE_COMMAND  0x09
	#define NO_DECODE         0x00

#define INTENSITY_COMMAND    0x0A
   #define INTENSITY_MIN     0x00
	#define INTENSITY_MAX     0x0F

#define SCAN_LIMIT_COMMAND   0x0B
	#define SCAN_ALL          0x07


#define SHUTDOWN_COMMAND     0x0C
   #define SHUTDOWN_MODE      0x00
   #define NORMAL_MODE        0x01

#define DISPLAY_TEST_COMMAND 0x0F
   #define TEST_OFF          0x00


// Initialize
void LED_Init( void );

// Turn a single LED on
void LED_On ( int led, bool_t flush);

// Turn a single LED off
void LED_Off( int led, bool_t flush);

// All LEDs off
void LED_AllOff( void );

// Flash a given LED
void LED_Flash( int led );

// Flush any changes
void LED_Flush( void );

// Set an arbitrary solid pattern
void LED_SetGridState ( unsigned long long bits );

// Set an arbitrary flashing pattern
void LED_FlashGridState ( unsigned long long bits );

void LED_SetBrightness( unsigned char level);

void LED_flipBoard( void );

#endif
