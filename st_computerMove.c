#include "hsm.h"
#include "hsmDefs.h"
#include "st_computerMove.h"

#include "st_inGame.h"
#include "display.h"
#include "sfInterface.h"
#include "timer.h"
#include "moves.h"
#include "board.h"
#include "diag.h"
#include "st_playingGame.h"
#include "constants.h"
#include "options.h"
#include "util.h"

extern bool_t computerMovePending;

void computerMoveEntry( event_t ev )
{

   DPRINT("ComputerMoveEntry");
   computerMovePending = FALSE;

   SF_setPosition(getFEN(&game.brd));

   SF_findMoveFixedDepth( 16 );

   // timerStart(TMR_COMPUTER_POLL, 100, 100, EV_CHECK_COMPUTER_DONE);

   if(options.game.white != options.game.black)
   {
      displayWriteLine(0, "Computer's Move", TRUE);
   }

   else if(game.brd.toMove == WHITE)
   {
      displayWriteLine(0, "White's Move", TRUE);
   }
   else
   {
      displayWriteLine(0, "Black's Move", TRUE);
   }

}

void computerMoveExit( event_t ev )
{
   // Zero out any remaining time...
   game.graceTime = 0;

}

extern uint64_t mustMove;
void computerMove_engineSelection( move_t mv, move_t ponder )
{
   computerMovePending = TRUE;

   if(mv.to & (game.brd.colors[WHITE] | game.brd.colors[BLACK]))
   {
      mustMove = squareMask[mv.to];
   }
   else
   {
      mustMove = 0;
   }

   playingGame_processSelectedMove(mv);
}
