#include "hsm.h"
#include "hsmDefs.h"
#include "st_exitingGame.h"
#include "timer.h"
#include "display.h"

void exitingGameEntry( event_t ev )
{
   timerKill(TMR_GAME_CLOCK_TIC);

   displayClear();
   displayWriteLine(0, "Game Over", TRUE);
   switch(ev.data)
   {
      case GAME_END_CHECKMATE:
         displayWriteLine(0, "Checkmate", TRUE);
         break;

      case GAME_END_STALEMATE:
         displayWriteLine(0, "Stalemate", TRUE);
         break;

      case GAME_END_ABORT:
         displayWriteLine(0, "Aborted", TRUE);
         break;
   }
   displayWriteLine(2, "Press any button to", TRUE);
   displayWriteLine(3, "return to main menu", TRUE);

}

void exitingGameExit( event_t ev )
{
   displayClear();
}
