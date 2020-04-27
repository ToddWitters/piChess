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
#include "switch.h"

extern bool_t computerMovePending;
extern game_t game;
bool_t waitingForButton = FALSE;

static void computerMove_engineSelection( move_t mv, move_t ponder );


void computerMoveEntry( event_t ev )
{

   DPRINT("ComputerMoveEntry\n");
   computerMovePending = FALSE;
   move_t m;
   move_t nullMv = {0,0,PIECE_NONE};

   if( (options.game.useOpeningBook == TRUE) &&
       (getRandMove( &game.brd, &m ) == BOOK_NO_ERROR) )
   {
      listBookMoves(&game.brd);
      DPRINT("Move selected from Book\n");
      computerMove_engineSelection( m, nullMv );
   }
   else
   {

      SF_setPosition(game.startPos, game.moveRecord);

      if(isOptionStr("timeControl","untimed"))
      {
         if(isOptionStr("computerStrategy", "fixedTime"))
         {
            SF_findMoveFixedTime(options.game.timeControl.compStrategySetting.timeInMs);
         }
         else if(isOptionStr("computerStrategy", "fixedDepth"))
         {
            SF_findMoveFixedDepth((int)getOptionVal("searchDepth"));
         }
         else if(isOptionStr("computerStrategy", "tillButton"))
         {
            waitingForButton = true;
            SF_go();
         }
         else
         {
            DPRINT("Unexpected Value [%s] for computerStrategy.  Setting to fixedDepth\n", getOptionStr("computerStrategy"));
            setOptionStr("computerStrategy", "fixedDepth");
         }
      }
      else
      {
         SF_findMove( game.wtime * 100, game.btime * 100, game.wIncrement * 100, game.bIncrement * 100 );
      }


      // timerStart(TMR_COMPUTER_POLL, 100, 100, EV_CHECK_COMPUTER_DONE);

      // If there is only one computer player...
      if(strcmp(getOptionStr("whitePlayer"),getOptionStr("blackPlayer")))
      {
         displayWriteLine(0, "Computer thinking...", true);
      }

      else if(game.brd.toMove == WHITE)
      {
         displayWriteLine(0, "White thinking...", true);
      }
      else
      {
         displayWriteLine(0, "Black thinking...", true);
      }
   }

}

bool computerMoveWaitingButton( event_t ev )
{

   return ( waitingForButton);
}

void computerMoveButtonStop( event_t ev )
{
   SF_stop();

   // This will request the engine to stop, and ultimately trigger a decision...
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
      fclose(tmpFile);
      remove(OUTPUT_FILE);

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
            DPRINT("Error unexpected contents in %s\n", OUTPUT_FILE);
         }
      }
      else
      {
         DPRINT("Error unexpected contents in %s\n", OUTPUT_FILE);
      }
   }
   else
   {
      DPRINT("Error opening %s\n", OUTPUT_FILE);
   }
}


extern uint64_t mustMove;
static void computerMove_engineSelection( move_t mv, move_t ponder )
{
   if(computerMovePending == false)
   {
      DPRINT("setting pending move true\n");
      computerMovePending = true;

      playingGame_processSelectedMove(mv);
   }
}
