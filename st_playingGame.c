#include "hsm.h"
#include "hsmDefs.h"
#include "st_playingGame.h"

#include "types.h"
#include "options.h"
#include "st_inGame.h"
#include "timer.h"
#include "moves.h"
#include "board.h"
#include "event.h"
#include "diag.h"

#include "sfInterface.h"


#include <string.h>

bool_t computerMovePending = FALSE;

void playingGameEntry( event_t ev )
{
   DPRINT("PlayingGameEntry\n");
}

void playingGameExit( event_t ev )
{

}

// Display logic during game...

// Line 1 : Comp. thinking / White's Move / Black's Move
// Line 2 : Last Few Moves
// Line 3 : White/Black, Human/Computer, Computer/Human
// Line 4 : Clock Times / Untimed Game

// After lift of pawn that could promote
//    Line 1: Empty
//    Line 2: Press to undrpromote
//    Line 3: Empty

// After illegal move (but before recovery)
//    Line 1: Empty
//    Line 2: ----Illegal Move----
//    Line 3: Empty

// When making move for computer
//    Line 1: <Grace time>
//    Line 2: Please make move
//    Line 3: for Computer

void playingGame_processSelectedMove( move_t mv)
{
   event_t ev;
   int16_t totalMovesFound;

   DPRINT("ProcessSelectedMove()\n");

   // Make the move on the board...
   move(&game.brd, mv);

   // TODO test for 75-move rule
   // TODO test for insufficient material

   // If a computer just finished...
   if( (game.brd.toMove == WHITE && options.game.black == PLAYER_COMPUTER) ||
       (game.brd.toMove == BLACK && options.game.white == PLAYER_COMPUTER))
   {
         computerMovePending = TRUE;
         ev.ev = EV_GOTO_PLAYING_GAME;
   }
   else
   {
      totalMovesFound = findMoves(&game.brd , NULL);

      // If there are no legal moves left...
      if( totalMovesFound <= 0 )
      {
         ev.ev = EV_GAME_DONE;

         if(totalMovesFound == CHECKMATE)
         {
            DPRINT("Found a checkmate!");
            game.disposition = GAME_AT_CHECKMATE;
            ev.data = GAME_END_CHECKMATE;
         }
         else
         {
            DPRINT("Found a stalemate!");
            game.disposition = GAME_AT_STALEMATE;
            ev.data = GAME_END_STALEMATE;
         }
      }
      else
      {
         ev.ev = EV_GOTO_PLAYING_GAME;
      }
   }

   putEvent(EVQ_EVENT_MANAGER, &ev);
}

uint16_t playingGamePickSubstate( event_t ev)
{

   if(computerMovePending == TRUE)
      return ST_MOVE_FOR_COMPUTER;

   else if(game.brd.toMove == WHITE)
      if(options.game.white == PLAYER_HUMAN)
         return ST_PLAYER_MOVE;
      else
         return ST_COMPUTER_MOVE;

   else
      if(options.game.black == PLAYER_HUMAN)
         return ST_PLAYER_MOVE;
      else
         return ST_COMPUTER_MOVE;

}
