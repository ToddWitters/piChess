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

#if 0
   // TESTING
   ev.ev = EV_PIECE_DROP;

   // Fill board with initial position
   ev.param = A1;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = B1;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = C1;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = D1;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = E1;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = F1;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = G1;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = H1;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   sleep(1);

   ev.param = A2;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = B2;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = C2;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = D2;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = E2;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = F2;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = G2;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = H2;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   sleep(1);

   ev.param = A7;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = B7;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = C7;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = D7;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = E7;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = F7;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = G7;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = H7;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   sleep(1);

   ev.param = A8;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = B8;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = C8;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = D8;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = E8;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = F8;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = G8;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   ev.param = H8;
   putEvent(EVQ_EVENT_MANAGER, &ev);
   sleep(1);
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
