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

// Set to arbitrary pattern
void LED_SetGridState ( uint64_t bits )
{
   int i;

   bits = reverseBitOrder64(bits);

   pthread_mutex_lock(&LED_dataMutex);

   for(i=0;i<8;i++)
   {
      uint64_t mask = (bits >> i*8) & 0x00000000000000FF;

      // Turn on LEDs that are not already in a blinking state
      ledRowData[i].ledState |= (mask & ~ledRowData[i].ledBlink);

      // Shut off LEDs that are not already in a blinking state
      ledRowData[i].ledState &= ~(~mask & ~ledRowData[i].ledBlink);
      
      ledRowData[i].rowDirty = TRUE;
   }

   pthread_mutex_unlock(&LED_dataMutex);

   LED_Flush();
}

// Set to arbitrary pattern
void LED_FlashGridState ( uint64_t bits )
{
   int i;


   // LED_AllOff();

   DPRINT("Flashing LED grid state to %016llX\n", bits);

   bits = reverseBitOrder64(bits);

   pthread_mutex_lock(&LED_dataMutex);

   for(i=0;i<8;i++)
   {
      uint64_t mask = (bits >> i*8) & 0x00000000000000FF;

      // Turn on all LEDs that are part of the mask
      ledRowData[i].ledState |= mask;

      // Turn off any LEDs that are currently blinking, but not part of new mask
      ledRowData[i].ledState &= ~(ledRowData[i].ledBlink & ~mask);

      ledRowData[i].ledBlink  = mask;
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

void LED_SetBrightness( unsigned char level)
{

   unsigned char command[2];

   command[0] = INTENSITY_COMMAND;
	command[1] = level;
	bcm2835_spi_writenb((char *)command, 2);

}
