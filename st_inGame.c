#include "hsm.h"
#include "hsmDefs.h"
#include "st_inGame.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "board.h"
#include "moves.h"
#include "diag.h"
#include "options.h"
#include "timer.h"
#include "sfInterface.h"

game_t game;

void inGameEntry( event_t ev )
{
   DPRINT("inGameEntry\n");

   game.chess960 = options.game.chess960;
   game.wtime = options.game.whiteTime;
   game.btime = options.game.blackTime;
   game.graceTime = 0;

   memset(&game.moves, 0x00, sizeof(game.moves));
   memset(&game.posHash, 0x00, sizeof(game.posHash));

   if(game.wtime !=0 || game.btime != 0)
      timerStart(TMR_GAME_CLOCK_TIC, 100, 100, EV_MOVE_CLOCK_TIC);

   // If either or both player is the computer, set up pipe to stockfish Engine
   if( (options.game.white == PLAYER_COMPUTER) || (options.game.black == PLAYER_COMPUTER) )
      SF_initEngine();

}

void inGameExit( event_t ev )
{
   // Set to default for next time...
   inGame_SetPosition( NULL );
}

uint16_t inGamePickSubstate( event_t ev)
{
   if(game.disposition != GAME_PLAYABLE)
   {
      DPRINT("Returning ST_EXITING_GAME from inGamePickSubstate\n");
      return ST_EXITING_GAME;
   }
   else
   {
      DPRINT("Returning ST_PLAYING_GAME from inGamePickSubstate\n");
      return ST_PLAYING_GAME;
   }
}

void inGame_moveClockTick( event_t ev)
{

}

void inGame_SetPosition( const char *FEN)
{
   if(game.startPos != NULL)
   {
      free(game.startPos);
      game.startPos = NULL;
   }

   setBoard( &game.brd, FEN);

   if(testValidBoard(&game.brd) == BRD_NO_ERROR)
   {

      DPRINT("Board is valid\n");
      if(FEN != NULL)
      {
         game.startPos = malloc(strlen(FEN)+1);
         strcpy( game.startPos, FEN);
      }
      else
      {
         game.startPos = malloc(strlen(startString));
         strcpy( game.startPos, startString);
      }

      if( findMoves(&game.brd , NULL) == 0 )
      {
         if(testInCheck(&game.brd) == TRUE)
         {
            DPRINT("CHECKMATE\n");
            game.disposition = GAME_AT_CHECKMATE;
         }
         else
         {
            DPRINT("STALEMATE\n");
            game.disposition = GAME_AT_STALEMATE;
         }
      }
      else
      {
         DPRINT("PLAYABLE\n");
         game.disposition = GAME_PLAYABLE;
      }
   }
   else
   {
      DPRINT("Board is invalid\n");
      game.disposition = GAME_INVALID;
   }
}
