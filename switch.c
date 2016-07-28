#include "types.h"
#include "switch.h"
#include "diag.h"
#include "util.h"
#include "i2c.h"
#include "gpio.h"
#include "event.h"
#include "hsm.h"
#include "bcm2835.h"
#include "options.h"
#include "hsm.h"
#include "hsmDefs.h"

#include <string.h>
#include <pthread.h>
#include <unistd.h>

//   BIT NUMBER FOR EACH SQUARE
//
//   +--+--+--+--+--+--+--+--+
// 8 |00|01|02|03|04|05|06|07|  PORTA |
//   +--+--+--+--+--+--+--+--+         > --- GPIO_EXPANDER_87
// 7 |08|09|10|11|12|13|14|15|  PORTB |
//   +--+--+--+--+--+--+--+--+
// 6 |16|17|18|19|20|21|22|23|  PORTA |
//   +--+--+--+--+--+--+--+--+         > --- GPIO_EXPANDER_65
// 5 |24|25|26|27|28|29|30|31|  PORTB |
//   +--+--+--+--+--+--+--+--+
// 4 |32|33|34|35|36|37|38|39|  PORTA |
//   +--+--+--+--+--+--+--+--+         > --- GPIO_EXPANDER_43
// 3 |40|41|42|43|44|45|46|47|  PORTB |
//   +--+--+--+--+--+--+--+--+
// 2 |48|49|50|51|52|53|54|55|  PORTA |
//   +--+--+--+--+--+--+--+--+         > --- GPIO_EXPANDER_21
// 1 |56|57|58|59|60|61|62|63|  PORTB |
//   +--+--+--+--+--+--+--+--+
//    A  B  C  D  E  F  G  H
//    1  2  3  4  5  6  7  8

// For diagnostics...
static const buttonPos_t switchStateTable[16] =
{
   POS_CENTER,    // 0000
   POS_RIGHT,     // 0001
   POS_LEFT,      // 0010
   POS_ERROR,     // 0011
   POS_DOWN,      // 0100
   POS_DOWN_RIGHT,// 0101
   POS_DOWN_LEFT, // 0110
   POS_ERROR,     // 0111
   POS_UP,        // 1000
   POS_UP_RIGHT,  // 1001
   POS_UP_LEFT,   // 1010
   POS_ERROR,     // 1011
   POS_ERROR,     // 1100
   POS_ERROR,     // 1101
   POS_ERROR,     // 1110
   POS_ERROR      // 1111
};

// The state of the most recent reed switch readings
static uint64_t sampleState = 0;

// Amount of time to delay before starting to repeat the button position
//   0 = don't repeat
static uint8_t repeatDelay = 0;

// Interval at which to repeat a button position
static uint8_t repeatInterval = 0;

// Down-Counter to track repeating button positions:
static uint8_t repeatCounter = 0;


// The debounced state of each sample
static uint64_t debouncedState = 0xFFFFFFFFFFFFFFFF;

// debounce counter for each reed switch
static uint8_t  debounceCounters[64];

// The current and previous 5-way button reading
//   This is a simplified button debouncing scheme.  Two consecutive matching samples
//   will change the state.
static uint8_t bSampleState = 0;
static uint8_t bLastSampleState = 0;

// TBD do we need this?
// Used to turn polling on/off
static bool_t pollingOn = FALSE;

static pthread_mutex_t Switch_dataMutex;

// Function to get the reed switch states
static uint64_t getSwitchStates( void );

// Called when a reed switch changes state
static void switchChanged(int sq, bool_t state);

// Checks a given row on the chess board.
static uint8_t checkRow(uint8_t row, uint8_t intPin, uint8_t portAddress, uint8_t slaveAddress);

// debounce the state of the 5-way button
static void buttonDebounce (uint8_t switchData);



// reed switch states.  Used to avoid i2c read of data when interrupt line not asserted.
static uint64_t lastBitBoard = 0;
static uint64_t bitBoard = 0;

// Initialize switch stuff
void switchInit( void )
{
   // Create a mutex to block data access from multiple threads.
   pthread_mutex_init(&Switch_dataMutex, NULL);

   // Zero all switch counters
   memset(debounceCounters, 0x00, sizeof(debounceCounters));
}

// TBD should we just always enable this at init?
// Begin polling
void StartSwitchPoll( void )
{
    pthread_mutex_lock(&Switch_dataMutex);

    DPRINT("Starting switch polling\n");

    // Zero all switch counters
    memset(debounceCounters, 0x00, sizeof(debounceCounters));

    // all bits to zero
    debouncedState = 0xFFFFFFFFFFFFFFFF;

    // Enable polling
    pollingOn = TRUE;

    pthread_mutex_unlock(&Switch_dataMutex);
}

// TBD do we ever need this?
void StopSwitchPoll( void )
{
    DPRINT("Stopping switch polling\n");
    pollingOn = FALSE;
}

// TBD do we ever need this?
void ResumeSwitchPoll( void )
{

    pthread_mutex_lock(&Switch_dataMutex);

    DPRINT("Resuming switch polling\n");
    
    // reset counters since we dont know how long we have been shut off
    memset(debounceCounters, 0x00, sizeof(debounceCounters));
    pollingOn = TRUE;

    pthread_mutex_unlock(&Switch_dataMutex);
}

// Retrieve all 64 reed switch states
uint64_t GetSwitchStates ( void )
{
    uint64_t retValue;

    pthread_mutex_lock(&Switch_dataMutex);

    retValue = ~debouncedState;

    pthread_mutex_unlock(&Switch_dataMutex);

    return reverseBitOrder64(retValue);
}

// Called periodically from timer task
void switchPoll ( void )
{
   uint64_t diff;
   uint64_t mask;
   int i;

   // Lock access to these while we modify them...
   pthread_mutex_lock(&Switch_dataMutex);

   if(pollingOn)
   {

      // REED SWITCHES..

      // Get the current switch positions and check for any differences.
      diff = debouncedState ^ (sampleState = getSwitchStates());

      // Walk through 1 bit at a time...
      mask =0x0000000000000001;

      for(i=0;i<64;i++)
      {
         // Is the sample different than the debounced value?
         if(mask & diff)
         {
            // Count it
            debounceCounters[i]++;

            // Is this square marked as occupied?
            if(mask & debouncedState)
            {
               // Has it been occupied long enough?
               if(debounceCounters[i] >= options.board.pieceDropDebounce)
               {
                  switchChanged(i, FALSE);
                  debouncedState &= ~mask;
                  debounceCounters[i] = 0;
               }
            }
            else // Square is currently occupied
            {
               if(debounceCounters[i] >= options.board.pieceLiftDebounce)
               {
                  switchChanged(i, TRUE);
                  debouncedState |= mask;
                  debounceCounters[i] = 0;
               }
            }
         }
         else
         {
            debounceCounters[i] = 0;
         }

         mask <<= 1;
      }

      // 5-WAY BUTTON

      // Has the state changed?
      if( bcm2835_gpio_lev(BUTTON_SWITCH_INT_PIN) == 0)
      {
         uint8_t command[2];
         uint8_t junk;

         // Read the current state of the pins.
         // NOTE, since the compare value hasn't changed yet, the interrupt will still
         // be triggered... we'll handle that below...
         command[0] = BUTTON_PORT;
         i2cSendReceive(GPIO_EXPANDER_UI_ADDR, command, 1, &bSampleState, 1);

         // Mask off the 5 button bits
         bSampleState &= B_MASK;

         // Use the new value as the comparison value
         command[0] = BUTTON_PORT + (DEFVALA_ADDR - GPIOA_ADDR);
         command[1] = bSampleState;
         i2cSendCommand(GPIO_EXPANDER_UI_ADDR, command, 2);

         // Read the current state of the pins again to clear the interrupt, now that
         //  the new comparison value is written.
         command[0] = BUTTON_PORT;
         i2cSendReceive(GPIO_EXPANDER_UI_ADDR, command, 1, &junk, 1);

         // Invert the logic for these bits (i.e. 0 = actively pressed)
         bSampleState ^= B_MASK;
      }
      else
      {
         // No change ... just use last state
         bSampleState = bLastSampleState;
      }

      // Since these aren't exclusive bits, handle the deboncing a little differently...
      buttonDebounce(bSampleState);

      // Keep track of last state
      bLastSampleState = bSampleState;

   }
    pthread_mutex_unlock(&Switch_dataMutex);
}


// Reed switch polling
static uint64_t getSwitchStates( void )
{

   // Check switches, one row at a time
   ((uint8_t*)(&bitBoard))[0] = checkRow(8, ROW_8_SWITCH_INT_PIN, GPIOA_ADDR, GPIO_EXPANDER_87_ADDR);
   ((uint8_t*)(&bitBoard))[1] = checkRow(7, ROW_7_SWITCH_INT_PIN, GPIOB_ADDR, GPIO_EXPANDER_87_ADDR);
   ((uint8_t*)(&bitBoard))[2] = checkRow(6, ROW_6_SWITCH_INT_PIN, GPIOA_ADDR, GPIO_EXPANDER_65_ADDR);
   ((uint8_t*)(&bitBoard))[3] = checkRow(5, ROW_5_SWITCH_INT_PIN, GPIOB_ADDR, GPIO_EXPANDER_65_ADDR);
   ((uint8_t*)(&bitBoard))[4] = checkRow(4, ROW_4_SWITCH_INT_PIN, GPIOA_ADDR, GPIO_EXPANDER_43_ADDR);
   ((uint8_t*)(&bitBoard))[5] = checkRow(3, ROW_3_SWITCH_INT_PIN, GPIOB_ADDR, GPIO_EXPANDER_43_ADDR);
   ((uint8_t*)(&bitBoard))[6] = checkRow(2, ROW_2_SWITCH_INT_PIN, GPIOA_ADDR, GPIO_EXPANDER_21_ADDR);
   ((uint8_t*)(&bitBoard))[7] = checkRow(1, ROW_1_SWITCH_INT_PIN, GPIOB_ADDR, GPIO_EXPANDER_21_ADDR);

   // Keep track for next time...
   lastBitBoard = bitBoard;

   // Invert the logic, since GND = pressed
   bitBoard = ~bitBoard;

   return bitBoard;
}


static uint8_t checkRow(uint8_t row, uint8_t intPin, uint8_t portAddress, uint8_t slaveAddress)
{
   uint8_t retValue;
   uint8_t command[2];

   // If interrupt pin is not asserted, just return last sample
   if( bcm2835_gpio_lev(intPin) == 1)
   {
      // No changes... return last value
       retValue = ((uint8_t *)(&lastBitBoard))[8-row];
   }
   else
   {

      uint8_t junk;

      // read the contents of this port
      command[0] = portAddress;
      i2cSendReceive(slaveAddress, command, 1, &retValue, 1);

      // Use the new value as the comparison value
      command[0] = portAddress + (DEFVALA_ADDR - GPIOA_ADDR);
      command[1] = retValue;
      i2cSendCommand(slaveAddress, command, 2);

      // read the contents of this port again to clear the interrupt pin
      command[0] = portAddress;
      i2cSendReceive(slaveAddress, command, 1, &junk, 1);

      retValue ^= 0xFF;

   }

   return retValue;

}

// If a reed switch has changed states, handle it here...
static void switchChanged(int sq, bool_t state)
{
   event_t evnt;

   if(state == FALSE)
   {
      evnt.ev = EV_PIECE_DROP;
   }
   else
   {
      evnt.ev = EV_PIECE_LIFT;
   }

   evnt.data = sq;
   putEvent(EVQ_EVENT_MANAGER, &evnt);

}


// This is a simplified debounce routine that just looks for
// two consecutive samples to change state..
static void buttonDebounce (uint8_t switchData)
{
   // Used below when something changes.
   event_t evnt;

   static buttonPos_t posLastState      = POS_CENTER;
   static buttonPos_t posDebouncedState = POS_CENTER;
   buttonPos_t        posSampledState;

   static buttonPress_t bLastState      = B_RELEASED;
   static buttonPress_t bDebouncedState = B_RELEASED;
   buttonPress_t        bSampledState;

   // Lower four bits represent position.  Table contains
   // state based on the possible 16 values (some of which are errors)
   posSampledState = switchStateTable[switchData & 0x0F];

   // If this position is differnt from the debounced one...
   if(posSampledState != posDebouncedState)
   {
      // ... and it matches the previous one...
      if(posSampledState == posLastState)
      {
         // Update the debounced state
         posDebouncedState = posSampledState;
         
         if(posDebouncedState == POS_CENTER)
         {
            repeatCounter = 0;
         }
         else
         {
            // New button state...
            repeatCounter = repeatDelay;
         }
         // Populate the event data
         evnt.ev    = EV_BUTTON_POS;
         evnt.data = posDebouncedState;

         // Show the new position
         // DPRINT("Nav switch %s\n", switchStateText[switchData & 0x0F]);

         putEvent(EVQ_EVENT_MANAGER,   &evnt);
      }
   }
   else
   {
      if(repeatCounter)
      {
         if(--repeatCounter == 0)
         {
            repeatCounter = repeatInterval;

            // Populate the event data
            evnt.ev    = EV_BUTTON_POS;
            evnt.data = posDebouncedState;

            putEvent(EVQ_EVENT_MANAGER,   &evnt);
            
         }
      }
   }

   posLastState = posSampledState;

   // Now do the same with the press/no press state...
   bSampledState = ( ( (switchData & B_PRESS_MASK) == 0) ? B_RELEASED : B_PRESSED);

   if(bSampledState != bDebouncedState)
   {
      // button not at debounced state AND consistent with last sample
      if(bSampledState == bLastState)
      {
         bDebouncedState = bSampledState;

         // Populate the event data
         evnt.ev    = EV_BUTTON_STATE;
         evnt.data = bDebouncedState;

         // DPRINT("Nav switch %s\n", ( (bSampledState == B_RELEASED) ? "RELEASED" : "PRESSED") );

         putEvent(EVQ_EVENT_MANAGER,   &evnt);
      }

   }
   bLastState = bSampledState;

}

void setButtonRepeat(uint8_t delay, uint8_t interval)
{
   repeatDelay    = delay;   
   repeatInterval = interval;
}