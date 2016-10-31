#include "hsm.h"
#include "hsmDefs.h"
#include "st_inGame.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "board.h"
#include "moves.h"
#include "diag.h"
#include "options.h"
#include "timer.h"
#include "sfInterface.h"
#include "menu.h"
#include "specChars.h"
#include "display.h"
#include "led.h"
#include "book.h"

game_t game;

extern menu_t *inGameMenu;
extern bool_t computerMovePending;

static char *convertTimeToString (uint32_t tenths );

void inGameEntry( event_t ev )
{
   DPRINT("inGameEntry\n");

   game.chess960 = options.game.chess960;

   game.graceTime = 0;
   game.playedMoves = 0;
   game.moveRecord[0] = '\0';

   memset(&game.posHistory, 0x00, sizeof(game.posHistory));

   switch(options.game.timeControl.type)
   {
      case TIME_EQUAL:
         game.wtime      = options.game.timeControl.timeSettings[0].totalTime * 10;
         game.wIncrement = options.game.timeControl.timeSettings[0].increment * 10;
         game.btime = game.wtime;
         game.bIncrement = game.wIncrement;
         break;
      case TIME_ODDS:
         game.wtime =      options.game.timeControl.timeSettings[WHITE].totalTime * 10;
         game.wIncrement = options.game.timeControl.timeSettings[WHITE].increment * 10;
         game.btime =      options.game.timeControl.timeSettings[BLACK].totalTime * 10;
         game.bIncrement = options.game.timeControl.timeSettings[BLACK].increment * 10;

         break;
      case TIME_NONE:
         game.wtime = game.btime = 0;
         game.wIncrement = game.bIncrement = 0;
         break;
   }

   game.posHistory[0].clocks[WHITE] = game.wtime;
   game.posHistory[0].clocks[BLACK] = game.btime;

   game.posHistory[0].posHash = game.brd.hash;

   if(game.wtime !=0 || game.btime != 0)
      timerStart(TMR_GAME_CLOCK_TIC, 100, 100, EV_MOVE_CLOCK_TIC);

   // If either or both player is the computer, set up stockfish Engine and opening book
   if( (options.game.white == PLAYER_COMPUTER) || (options.game.black == PLAYER_COMPUTER) )
   {
      DPRINT("Starting Chess Engine\n");
      SF_initEngine();
      if(options.game.useOpeningBook == TRUE)
      {
         openBook("Stockfish_1.6_Book.bin");
      }
   }
}

void inGameExit( event_t ev )
{
   SF_closeEngine();
   computerMovePending = FALSE;
   LED_AllOff();
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

   if(options.game.timeControl.type == TIME_NONE ) return;

   // Don't move computer clock (or grace clock) when both players are computer 
   //    and human is making the move for the computer
   if(options.game.black == PLAYER_COMPUTER &&
      options.game.white == PLAYER_COMPUTER &&
      computerMovePending) return;

   // bail if game is not playable
   if(game.disposition != GAME_PLAYABLE) return;

   // bail if first move hasn't been made yet...
   if(game.playedMoves == 0) return;

   // If human is moving for computer, update grace time
   if(game.graceTime != 0)
   {
      game.graceTime--;
      inGame_udpateClocks();
   }

   else if(game.brd.toMove == WHITE)
   {
      if (game.wtime)
      {

         --game.wtime;

         if(game.wtime < 600 || (game.wtime % 10) == 9)
         {
            inGame_udpateClocks();
         }
      }

   }
   else
   {
      if (game.btime)
      {

         --game.btime;
         if(game.btime < 600 || (game.btime % 10) == 9)
         {
            inGame_udpateClocks();
         }
      }
   }
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


void inGame_udpateClocks( void )
{


   const char *timeString;
   char fullString[10];

   if(options.game.timeControl.type == TIME_NONE ) return;

   if(inGameMenu != NULL) return;

   memset(fullString, ' ', 10);
   timeString = convertTimeToString(game.wtime);
   memcpy(fullString, timeString, strlen(timeString));
   if(game.brd.toMove == WHITE  && game.graceTime == 0)
   {
      defineCharacter(0, charLeftFilledArrow);
      fullString[strlen(timeString) + 1] = 0;
   }
   displayWriteChars(3, 0, 10, fullString);

   memset(fullString, ' ', 10);
   timeString = convertTimeToString(game.btime);
   memcpy(&fullString[10-strlen(timeString)], timeString, strlen(timeString));
   if(game.brd.toMove == BLACK && game.graceTime == 0)
   {
      defineCharacter(0, charRightFilledArrow);
      fullString[10 - strlen(timeString) - 2] = 0;
   }
   displayWriteChars(3, 10, 10, fullString);


   memset(fullString, ' ', 6);
   if(game.graceTime != 0)
   {
      timeString = convertTimeToString(game.graceTime);
      memcpy(&fullString[6-strlen(timeString)], timeString, strlen(timeString));
      defineCharacter(0, charRightFilledArrow);
      fullString[6 - strlen(timeString) - 2] = 0;
   }
   displayWriteChars(1, 14, 6, fullString);


}

static char *convertTimeToString (uint32_t tenths )
{
   // Time  <  1 minute  shown as      SS.S
   // Times  <  1 hour   shown as   MM:SS
   // Times  >= 1 hour   shows at H:MM:SS

   static char str[9];

   if( tenths < 600 )
   {
      sprintf(str,"0:%02d.%1d", tenths / 10, tenths % 10 );
   }
   else if (tenths < 36000)
   {
      sprintf(str, "%d:%02d",      tenths / 600,  (tenths % 600) / 10);
   }
   else
   {
      sprintf(str, "%d:%02d:%02d", tenths / 36000, (tenths % 36000) / 600, (tenths % 600) / 10);
   }

   return str;
}
