#include <pthread.h>
#include <unistd.h>

#include "led.h"
#include "diag.h"
#include "bcm2835.h"
#include "util.h"
#include "constants.h"
#include "options.h"

// The following shows the LED numbers relative to the chessboard
//   with a1 in the lower left.  Also, shown are the segments (connected
//    to columns of LED anodes) and the Digits (connected to rows LED cathodes)

//             +----+----+----+----+----+----+----+----+
// 8 = DIG0 -> | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 |
//             +----+----+----+----+----+----+----+----+
// 7 = DIG1 -> | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 |
//             +----+----+----+----+----+----+----+----+
// 6 = DIG2 -> | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |
//             +----+----+----+----+----+----+----+----+
// 5 = DIG3 -> | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |
//             +----+----+----+----+----+----+----+----+
// 4 = DIG4 -> | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |
//             +----+----+----+----+----+----+----+----+
// 3 = DIG5 -> | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 |
//             +----+----+----+----+----+----+----+----+
// 2 = DIG6 -> | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 |
//             +----+----+----+----+----+----+----+----+
// 1 = DIG7 -> | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |
//             +----+----+----+----+----+----+----+----+
//              SEGG|SEGF|SEGE|SEGD|SEGC|SEGB|SEGA| DP
//                A    B    C    D   E     F    G    H

// NOTE:  This table must correlate with
// typedef in header
const unsigned long long pattern[] =
{
   0x0000000000000000, // All Off
   0xFFFFFFFFFFFFFFFF, // All On

   b8|d8|f8|h8|
	a7|c7|e7|g7|
	b6|d6|f6|h6|
	a5|c5|e5|g5|
	b4|d4|f4|h4|
	a3|c3|e3|g3|
	b2|d2|f2|h2|
   a1|c1|e1|g1,       // Light Squares


	b8|d8|f8|h8|
	a7|c7|e7|g7|
	b6|d6|f6|h6|
	a5|c5|e5|g5|
	b4|d4|f4|h4|
	a3|c3|e3|g3|
	b2|d2|f2|h2|
   a1|c1|e1|g1,        // Dark  Squares

};

// This is a software correction for a hardware wiring issues.  LED bit mapping was wired
//   in the opposite bit order.  Since a SW change is much easier, we'll fix that here...
static const unsigned char BitReverseTable256[] =
{
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

static unsigned long long reverseBitOrder64( unsigned long long input);

typedef struct led_row_t
{
   unsigned char ledState;
   unsigned char ledBlink;
   unsigned char rowDirty;
}led_row_t;

// Local data
led_row_t ledRowData[8];

static pthread_t flashThread;
static pthread_mutex_t LED_dataMutex;

// Local functions
static void *LED_FlashToggle ( void *arg );

// Initialize MAX chip, set all LEDs off and non-flashing
void LED_Init( void )
{
	int i;

	unsigned char command[2];

	DPRINT("Initializing LED driver\n");

   // Initialize broadcom spi driver
	bcm2835_spi_begin();

   // Through experimentation, this is the fastest we can go...
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);

   // Set to scan ALL LEDs
	command[0] = SCAN_LIMIT_COMMAND;
	command[1] = SCAN_ALL;
	bcm2835_spi_writenb((char *)command, 2);

   // Set intensity
   LED_SetBrightness(options.board.LED_Brightness);

   // Make sure display test mode is OFF
	command[0] = DISPLAY_TEST_COMMAND;
	command[1] = TEST_OFF;
	bcm2835_spi_writenb((char *)command, 2);

   // Don't try to interpret data as a digit - its binary with each bit controlling one of 8 LEDs
	command[0] = DECODE_MODE_COMMAND;
	command[1] = NO_DECODE;
	bcm2835_spi_writenb((char *)command, 2);

   // Set all LEDs to off and no blinking
	for(i=0;i<8;i++)
	{
		ledRowData[i].ledState = 0;
		ledRowData[i].ledBlink = 0;
		ledRowData[i].rowDirty = TRUE;
	}

   // Flush all data out to the LED chip
	LED_Flush( );

   command[0] = SHUTDOWN_COMMAND;
	command[1] = NORMAL_MODE;
	bcm2835_spi_writenb((char *)command, 2);


   // TODO should we do something here to ensure we don't re-create a 2nd process
   //   if init is called again?
   //
   // Create a thread for the flash operation
   pthread_create(&flashThread, NULL, LED_FlashToggle, NULL);

   // Create a mutex to block data access from multiple threads.
   pthread_mutex_init(&LED_dataMutex, NULL);

}

// Turn requested LED on and optionally flush
void LED_On (int led, bool_t flush)
{

   DPRINT("Turning LED %s on\n", convertSqNumToCoord(led));

   if(led > 63) return;

   int row = 7 - (led / 8);
   int colmask = 0x01 << ( 7 - (led % 8));

   pthread_mutex_lock(&LED_dataMutex);

   ledRowData[row].ledState |= colmask;
   ledRowData[row].ledBlink &= ~colmask;
   ledRowData[row].rowDirty = 1;

   pthread_mutex_unlock(&LED_dataMutex);

   if(flush) LED_Flush();

}

// Turn requested LED off and optionally flush
void LED_Off (int led, bool_t flush)
{

   DPRINT("Turning LED %s off\n", convertSqNumToCoord(led));

   if(led > 63) return;

   int row = 7 - (led / 8);
   int colmask = 0x01 << ( 7 - (led % 8));

   pthread_mutex_lock(&LED_dataMutex);

   ledRowData[row].ledState &= ~colmask;
   ledRowData[row].ledBlink &= ~colmask;
   ledRowData[row].rowDirty = 1;

   pthread_mutex_unlock(&LED_dataMutex);

   if(flush) LED_Flush();

}

// Turn all LEDs off
void LED_AllOff( void )
{
   int i;

   DPRINT("Turning All LEDs off\n");

   pthread_mutex_lock(&LED_dataMutex);

   for(i=0;i<8;i++)
   {
      ledRowData[i].ledBlink = 0;
      if(ledRowData[i].ledState != 0)
      {
         ledRowData[i].ledState = 0;
         ledRowData[i].rowDirty = TRUE;
      }
   }

   pthread_mutex_unlock(&LED_dataMutex);

   LED_Flush( );
}

// Flash the desired LED
void LED_Flash( int led )
{
   int row,colmask,i;

   DPRINT("Setting LED %s as flashing\n", convertSqNumToCoord(led));

   if(led > 63) return;

   row = 7 - (led / 8);
   colmask = 0x01 << ( 7 - (led % 8));

   pthread_mutex_lock(&LED_dataMutex);

   ledRowData[row].ledBlink |= colmask;

   // Make sure ALL LEDs that are set to blink state are synchronized with this one:
   for(i=0;i<8;i++)
   {
      ledRowData[row].ledState |= ledRowData[row].ledBlink;
   }

   pthread_mutex_unlock(&LED_dataMutex);

}

// Set to known test pattern
void LED_TestPattern( LED_Pattern_t patternNum )
{

   if(patternNum >= sizeof(pattern)/sizeof(pattern[0])) return;

   LED_SetGridState( pattern[patternNum] );

}

// Set to arbitrary pattern
void LED_SetGridState ( unsigned long long bits )
{
   int i;

   LED_AllOff();

   DPRINT("Fixing LED grid state to %016llX\n", bits);

   bits = reverseBitOrder64(bits);

   pthread_mutex_lock(&LED_dataMutex);

   for(i=0;i<8;i++)
   {
      ledRowData[i].ledState = (bits >> i*8) & 0x00000000000000FF;
      ledRowData[i].rowDirty = TRUE;
      ledRowData[i].ledBlink = 0;
   }

   pthread_mutex_unlock(&LED_dataMutex);

   LED_Flush();
}

// Set to arbitrary pattern
void LED_FlashGridState ( unsigned long long bits )
{
   int i;

   LED_AllOff();

   DPRINT("Flashing LED grid state to %016llX\n", bits);

   bits = reverseBitOrder64(bits);

   pthread_mutex_lock(&LED_dataMutex);

   for(i=0;i<8;i++)
   {
      ledRowData[i].ledBlink = ledRowData[i].ledState = (bits >> i*8) & 0x00000000000000FF;
      ledRowData[i].rowDirty = TRUE;
   }

   pthread_mutex_unlock(&LED_dataMutex);

   LED_Flush();
}


// Push LED data out to MAX chip...
void LED_Flush ( void )
{

	int i;

   pthread_mutex_lock(&LED_dataMutex);

   for(i=0;i<8;i++)
   {
      if(ledRowData[i].rowDirty == TRUE)
      {
         unsigned char command[2];

         command[0] = i+1;
         command[1] = ledRowData[i].ledState;

         bcm2835_spi_writenb((char *)command, 2);

         ledRowData[i].rowDirty = FALSE;
      }
   }

   pthread_mutex_unlock(&LED_dataMutex);

}

// LOCAL functions

// This needs to be called from the foreground...
static void *LED_FlashToggle ( void *arg )
{
   int i;

   bool_t changeMade;

	while(1)
   {
      usleep(300000);

      changeMade = FALSE;

      pthread_mutex_lock(&LED_dataMutex);

		for(i=0;i<8;i++)
		{
			if(ledRowData[i].ledBlink)
			{
            changeMade = TRUE;
				ledRowData[i].ledState ^= ledRowData[i].ledBlink;
				ledRowData[i].rowDirty = 1;
			}
		}

      pthread_mutex_unlock(&LED_dataMutex);

      if(changeMade)
      {
         LED_Flush();
      }
   }

   return NULL;
}

static unsigned long long reverseBitOrder64( unsigned long long input)
{
   unsigned long long result = 0;

   *((uint8_t *)(&result) + 0) = BitReverseTable256[*((uint8_t* )(&input) + 7)];
   *((uint8_t* )(&result) + 1) = BitReverseTable256[*((uint8_t* )(&input) + 6)];
   *((uint8_t* )(&result) + 2) = BitReverseTable256[*((uint8_t* )(&input) + 5)];
   *((uint8_t* )(&result) + 3) = BitReverseTable256[*((uint8_t* )(&input) + 4)];
   *((uint8_t* )(&result) + 4) = BitReverseTable256[*((uint8_t* )(&input) + 3)];
   *((uint8_t* )(&result) + 5) = BitReverseTable256[*((uint8_t* )(&input) + 2)];
   *((uint8_t* )(&result) + 6) = BitReverseTable256[*((uint8_t* )(&input) + 1)];
   *((uint8_t* )(&result) + 7) = BitReverseTable256[*((uint8_t* )(&input) + 0)];

   return result;
}

void LED_SetBrightness( unsigned char level)
{

   unsigned char command[2];

   command[0] = INTENSITY_COMMAND;
	command[1] = level;
	bcm2835_spi_writenb((char *)command, 2);

}
