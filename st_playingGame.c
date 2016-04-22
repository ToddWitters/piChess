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

   // Record the selected move
   game.posHistory[game.playedMoves].move = mv;

   // Make the move on the board and store reverse information
   game.posHistory[game.playedMoves].revMove = move(&game.brd, mv);

   // Update the pointer
   if(++game.playedMoves >= MAX_MOVES_IN_GAME)
   {
      // Set back in range...
      game.playedMoves = MAX_MOVES_IN_GAME - 1;

      DPRINT("ERROR: Game exceeded %d moves... undo no longer possible", MAX_MOVES_IN_GAME);
   }

   game.posHistory[game.playedMoves].posHash = game.brd.hash;

   // Store the current clock values in case we revert back later
   game.posHistory[game.playedMoves].clocks[WHITE] = game.wtime;
   game.posHistory[game.playedMoves].clocks[BLACK] = game.btime;

   // If black just moved...
   if(game.brd.toMove == WHITE)
   {
      // And black has non-zero time with an increment specified...
      if(game.btime != 0 && options.game.blackTimeInc != 0)
      {
         // Update his time and display the clocks...
         game.btime += options.game.blackTimeInc;
         inGame_udpateClocks();
      }
   }
   else
   {
      if(game.wtime != 0 && options.game.whiteTimeInc != 0)
      {
         game.wtime += options.game.whiteTimeInc;
         inGame_udpateClocks();
      }
   }

   // These first two are not optional and have no associated options with them.
   // TODO test for 75-move rule
   // if(game.brd.halfMoves >= 150)

   // TODO test for 5-fold repetition rule
   // Current position repeats 4 other times: 4, 8, 12, and 16 half-moves back

   // if(options.game.autoDrawOnInsufficient)
   // TODO test for insufficient material
   //   K v K
   //   K v K and bishop(s) all on same color
   //   K v KN
   //   KB v KB multiple bishops on either size, all on same color

   // if(options.game.autoDrawOnThreefold)
   // TODO test for three-fold rep
   // Current position repeats two other places in game history

   // if(options.game.autoDrawOnFiftyMove)
   // TODO test for 50-move rule
   // if(game.brd.halfMoves >= 100)

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
