#include "hsm.h"
#include "hsmDefs.h"
#include "st_playingGame.h"

#include "stdio.h"
#include "string.h"

#include "types.h"
#include "options.h"
#include "st_inGame.h"
#include "timer.h"
#include "moves.h"
#include "board.h"
#include "event.h"
#include "diag.h"
#include "util.h"
#include "display.h"

#include "sfInterface.h"

extern game_t game;

#include <string.h>

bool_t computerMovePending = FALSE;

static void showMoveHistory(char* string);

void playingGameEntry( event_t ev )
{
   DPRINT("PlayingGameEntry\n");

   // If a human is on move and there are no clocks...
   if(
       (
       (game.brd.toMove == WHITE &&
       options.game.white == PLAYER_HUMAN)
       ||
       (game.brd.toMove == BLACK &&
       options.game.black  == PLAYER_HUMAN)
       )
       &&
       options.game.timeControl.type == TIME_NONE
      )
   {
      displayWriteLine(3, "Untimed Game", true);
   }


   // If computer is on move with no clocks, note the strategy it is using...
   else if(options.game.timeControl.type == TIME_NONE)
   {
      switch(options.game.timeControl.compStrategySetting.type)
      {
         case STRAT_FIXED_TIME:
            displayWriteLine(3, "Fixed time search", true);
            break;

         case STRAT_FIXED_DEPTH:
            displayWriteLine(3, "Fixed depth search", true);
            break;

         case STRAT_TILL_BUTTON:
            displayWriteLine(3, "Search till button", true);
            break;
      }
   }

   // Display the last few moves...
   showMoveHistory(game.SANRecord);
}

void playingGameExit( event_t ev )
{

}


void playingGame_processSelectedMove( move_t mv)
{
   event_t ev;
   int16_t totalMovesFound;

   DPRINT("ProcessSelectedMove()\n");

   // Record the selected move
   game.posHistory[game.playedMoves].move = mv;

   if(strlen(game.SANRecord)!= 0)
      strcat(game.SANRecord," ");

   if(game.brd.toMove == WHITE)
   {
      char temp[4];
      sprintf(temp, "%d.", game.brd.moveNumber);
      strcat(game.SANRecord, temp);
   }

   strcat(game.SANRecord, moveToSAN(mv, &game.brd));
   showMoveHistory(game.SANRecord);

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


   // If this is an equal time setting, it may be time to move to a new period
   if(options.game.timeControl.type == TIME_EQUAL)
   {
      uint8_t periodOneMoves;

      // If the first period has a non-zero moves indicator
      if( (periodOneMoves = options.game.timeControl.timeSettings[0].moves) != 0 )
      {
         uint8_t periodTwoMoves;

         // Are we just now entering period 2?
         if(game.playedMoves == (periodOneMoves * 2))
         {
            game.bIncrement = game.wIncrement = options.game.timeControl.timeSettings[1].increment * 10;
            game.wtime += options.game.timeControl.timeSettings[1].totalTime * 10;
            game.btime += options.game.timeControl.timeSettings[1].totalTime * 10;
         }
         else if( (periodTwoMoves = options.game.timeControl.timeSettings[1].moves) != 0 )
         {
            // Are we just now entering period 3?
            if(game.playedMoves == ((periodOneMoves + periodTwoMoves) *2))
            {
               DPRINT("Entering Period 2\n");
               game.bIncrement = game.wIncrement = options.game.timeControl.timeSettings[2].increment * 10;
               game.wtime += options.game.timeControl.timeSettings[2].totalTime * 10;
               game.btime += options.game.timeControl.timeSettings[2].totalTime * 10;
            }
         }
      }
   }



   // If white is up to move...
   if(game.brd.toMove == WHITE)
   {
      // If has non-zero time with an increment specified....
      if(game.wtime != 0 && game.wIncrement != 0)
      {
         // Update time and display the clocks...
         game.wtime += game.wIncrement;
         inGame_udpateClocks();
      }
   }
   else
   {
      if(game.btime != 0 && game.bIncrement != 0)
      {
         game.btime += game.bIncrement;
         inGame_udpateClocks();
      }
   }

   // Store the current clock values in case we revert back later
   game.posHistory[game.playedMoves].clocks[WHITE] = game.wtime;
   game.posHistory[game.playedMoves].clocks[BLACK] = game.btime;

   // pump out to move record...
   if(strlen(game.moveRecord) != 0)
      strcat(game.moveRecord," ");

   strcat(game.moveRecord, convertSqNumToCoord(mv.from));
   strcat(game.moveRecord, convertSqNumToCoord(mv.to));
   switch(mv.promote)
   {
      case QUEEN:  strcat(game.moveRecord, "q"); break;
      case ROOK:   strcat(game.moveRecord, "r"); break;
      case BISHOP: strcat(game.moveRecord, "b"); break;
      case KNIGHT: strcat(game.moveRecord, "n"); break;
   }


   // These first two are not optional and have no associated options with them.
   // TODO test for 75-move rule
   if(game.brd.halfMoves >= 150)
   {

   }

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

   // If a computer just finished,
   if( (game.brd.toMove == WHITE && options.game.black == PLAYER_COMPUTER) ||
       (game.brd.toMove == BLACK && options.game.white == PLAYER_COMPUTER))
   {
         // This will ultimately move us to the ST_MOVE_FOR_COMPUTER state
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

static void showMoveHistory(char* string)
{

   // As long as the string is too long...
   while(strlen(string) > 20)
   {
      // search for the next space character
      while(*string != ' ') string++;

      // Advance beyond it to the next character
      string++;
   }

   displayWriteLine(1, string, true);

}


