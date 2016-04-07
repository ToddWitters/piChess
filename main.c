#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "led.h"
#include "bcm2835.h"
#include "switch.h"
#include "util.h"
#include "diag.h"
#include "board.h"
#include "sfInterface.h"
#include "moves.h"
#include "constants.h"
#include "display.h"
#include "event.h"
#include "eventManager.h"
#include "i2c.h"
#include "gpio.h"
#include "timer.h"
#include "menu.h"
#include "options.h"

// static move_t mv, ponder;
// static revMove_t rev;
// static board_t b;

int main ( void )
{

//   menu_t *test;

   eventData_t ev;

   // Init the hardware
   bcm2835_init();


   DPRINT("Program start\n");

   // set all options
   loadOptions(&options);

   // Set up the timer tic...
   timerInit();

   i2cInit();
   gpioInit();
   displayInit();
   LED_Init();
   switchInit();

   // Start polling switches
   StartSwitchPoll();

   // Init the event handler
   initEvent();

   // Init the event Manager
   eventManagerInit();

   // TODO... should we simply call event manager here?


#endif
   while(1)
   {
      sleep(1);
   }

   return 0;
}

// DEBUG

   // EXAMPLE OF GETTING A MOVE FROM THE Engine
#if 0

   board_t b;
   move_t mv, ponder;
   revMove_t rev;

   // Initialize board
   setBoard(&b, NULL);

   // Init interface to Stockfish engine
   SF_initEngine();

   // Set the position to the current board's FEN
   // TODO need to allow for passing a move list from
   // this position.
   SF_setPosition(getFEN(&b));

   // Find the best move at a depth of 20 ply
   SF_findMoveFixedDepth( 16 );

   // Loop until computer done
   do
   {
      // wait 100ms
      usleep(100000);

      // function will return mv.to = mv.from if not finished.
      // ponder will hold move computer is expecting from human
      mv = SF_checkDone(&ponder);

   } while(mv.from == mv.to);

   DPRINT("Computer selects %s as his move\n", moveToSAN(mv, &b));

   displayWriteLine( 0, "Please make move", TRUE);
   displayWriteLine( 1, "for computer", TRUE );
   displayWriteLine( 2, "Human       Computer", TRUE);
   displayWriteLine( 3, "1:23:45  <   1:23:45", TRUE);

   rev = move(&b, mv);

   DPRINT("Computer predicts response of %s\n", moveToSAN(ponder, &b));

   unmove(&b, rev);

#endif
