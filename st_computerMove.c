#include "hsm.h"
#include "hsmDefs.h"
#include "st_computerMove.h"
#include <string.h>

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
#include "book.h"

extern bool_t computerMovePending;

static void computerMove_engineSelection( move_t mv, move_t ponder );


void computerMoveEntry( event_t ev )
{

   DPRINT("ComputerMoveEntry\n");
   computerMovePending = FALSE;
   move_t m;
   move_t nullMv = {0,0,PIECE_NONE};

   if( (options.engine.openingBook == TRUE) &&
       (getRandMove( &game.brd, &m ) == BOOK_NO_ERROR) )
   {
      DPRINT("Move selected from Book\n");
      computerMove_engineSelection( m, nullMv );
   }
   else
   {

      // TODO... 2nd arg should be a move list
      SF_setPosition(getFEN(&game.brd), NULL);

      // TODO... untimed games will have a different format
      SF_findMove( game.wtime * 100, game.btime * 100, options.game.whiteTimeInc * 100, options.game.blackTimeInc * 100 );

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

}

void computerMoveExit( event_t ev )
{
   // Zero out any remaining time...
   game.graceTime = 0;
}


void computerMove_computerPicked( event_t ev)
{
#define MAX_LINE_LEN 30

   FILE *tmpFile;
   char engineResultLine[MAX_LINE_LEN];
   move_t selectedMove, ponderMove;

   tmpFile = fopen(OUTPUT_FILE, "r");

   if(tmpFile != NULL)
   {
      fgets(engineResultLine, MAX_LINE_LEN, tmpFile);

      DPRINT("Found text in engine result file: [%s]\n", engineResultLine);

      if(!strncmp(engineResultLine, "bestmove", 8))
      {

         selectedMove = convertCoordMove(&engineResultLine[9]);

         if(strlen(engineResultLine) >= 22)
         {

            if(selectedMove.promote != PIECE_NONE)
            {
               ponderMove = convertCoordMove(&engineResultLine[22]);
            }
            else
            {
               ponderMove = convertCoordMove(&engineResultLine[21]);
            }

         }
         
         if( selectedMove.to != selectedMove.from )
         {
            computerMove_engineSelection(selectedMove, ponderMove);
         }
         else
         {
            DPRINT("Error unexpected contents in %s", OUTPUT_FILE);
         }
      }
      else
      {
         DPRINT("Error unexpected contents in %s", OUTPUT_FILE);
      }
      fclose(tmpFile);
      remove(OUTPUT_FILE);
   }
   else
   {
      DPRINT("Error opening %s", OUTPUT_FILE);
   }
}


extern uint64_t mustMove;
static void computerMove_engineSelection( move_t mv, move_t ponder )
{
   if(computerMovePending == FALSE)
   {
      computerMovePending = TRUE;

      if(squareMask[mv.to] & (game.brd.colors[WHITE] | game.brd.colors[BLACK]))
      {
         mustMove = squareMask[mv.to];
      }
      else
      {
         mustMove = 0;
      }

      playingGame_processSelectedMove(mv);
   }
}
