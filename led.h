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


typedef enum LED_Pattern_e
{
   LED_ALL_OFF,
   LED_ALL_ON,
   LED_LIGHT_SQUARES,
   LED_DARK_SQUARES,

   LED_A_COL,
   LED_B_COL,
   LED_C_COL,
   LED_D_COL,
   LED_E_COL,
   LED_F_COL,
   LED_G_COL,
   LED_H_COL,

   LED_1_ROW,
   LED_2_ROW,
   LED_3_ROW,
   LED_4_ROW,
   LED_5_ROW,
   LED_6_ROW,
   LED_7_ROW,
   LED_8_ROW,

}LED_Pattern_t;

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

// Set a given test pattern
void LED_TestPattern( LED_Pattern_t patternNum );

// Set an arbitrary solid pattern
void LED_SetGridState ( unsigned long long bits );

// Set an arbitrary flashing pattern
void LED_FlashGridState ( unsigned long long bits );


#endif
