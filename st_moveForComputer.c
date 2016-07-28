#include "hsm.h"
#include "hsmDefs.h"
#include "st_moveForComputer.h"

#include "st_inGame.h"
#include "st_playingGame.h"
#include "display.h"
#include "led.h"
#include "switch.h"
#include "event.h"
#include "constants.h"
#include "options.h"
#include "moves.h"

uint64_t occupiedSquares;
extern bool_t computerMovePending;
uint64_t mustMove;

void moveForComputerEntry( event_t ev )
{
   displayWriteLine(0, "Pls move for computer", TRUE);
   LED_SetGridState( (GetSwitchStates() ^ (game.brd.colors[WHITE] | game.brd.colors[BLACK])) | mustMove);
   game.graceTime = options.game.graceTimeForComputerMove;
}

void moveForComputerExit( event_t ev )
{
   displayClear();
}

void moveForComputer_boardChange( event_t ev)
{
   if( (ev.ev = EV_PIECE_LIFT) && (squareMask[ev.data] == mustMove) )
   {
      mustMove = 0;
   }

   LED_SetGridState( (GetSwitchStates() ^ (game.brd.colors[WHITE] | game.brd.colors[BLACK])) | mustMove);

   if( mustMove == 0 && (GetSwitchStates() == (game.brd.colors[WHITE] | game.brd.colors[BLACK])))
   {
      event_t event;

      int totalMovesFound = findMoves(&game.brd , NULL);

      // If there are no legal moves left...
      if( totalMovesFound <= 0 )
      {
         event.ev = EV_GAME_DONE;

         if(totalMovesFound == CHECKMATE)
         {
            game.disposition = GAME_AT_CHECKMATE;
            event.data = GAME_END_CHECKMATE;
         }
         else
         {
            game.disposition = GAME_AT_STALEMATE;
            event.data = GAME_END_STALEMATE;
         }
      }
      else
      {
         game.graceTime = 0;
         event.ev = EV_PLAYER_MOVED_FOR_COMP;
      }

      putEvent(EVQ_EVENT_MANAGER, &event);
      computerMovePending = FALSE;
   }
}
