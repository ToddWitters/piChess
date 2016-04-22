#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "diag.h"
#include "sfInterface.h"
#include "util.h"
#include "options.h"
#include "st_computerMove.h"

#include <pthread.h>

#include <stropts.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>


#define OUTPUT_FILE "/home/pi/chess/result.txt"
#define SF_EXE      "/home/pi/chess/stockfish > sfOutput.txt"

FILE *sfPipe = NULL;
struct pollfd fds[1];

static pthread_t enginePollThread;

static move_t selectedMove = {0,0,PIECE_NONE};
static move_t ponderMove   = {0,0,PIECE_NONE};

static void *enginePollTask ( void *arg );


void SF_initEngine( void )
{

   char skillLevelText[3];

   remove(OUTPUT_FILE);

   // Spin up a task here...
   pthread_create(&enginePollThread, NULL, enginePollTask , NULL);

   // Start Stockfish
   sfPipe = popen(SF_EXE, "w");

   // Verify pipe was successfull.
   if(sfPipe == NULL)
   {
      DPRINT("Failed to open pipe for stockfish engine\n");
      return;
   }

   // Remove buffering so sprintf will send commands immediately.
   setbuf(sfPipe, NULL);

   // Set up our default parameters to Stockfish
   SF_setOption("Threads", "4");

   sprintf(skillLevelText, "%d\n", options.engine.strength);
   SF_setOption("Skill Level", skillLevelText);
}

void SF_closeEngine( void )
{
   if(sfPipe != NULL)
   {
      fprintf(sfPipe, "quit\n");
      pclose(sfPipe);
      sfPipe = NULL;
   }
}

void SF_setOption( char *name, char *value)
{
   fprintf(sfPipe,"setoption name %s value %s", name, value);
}

void SF_setPosition( char *fen, char *moveList)
{
   // Verify pipe first
   if(sfPipe == NULL)
   {
      DPRINT("setPosition called with uninitialized stockfish pipe\n");
   }

   // Is this the start position?
   else if(fen == NULL)
   {
      DPRINT("Setting board to initial position\n");

      // Should the engine apply a move list?
      if(moveList == NULL)
      {
         fprintf(sfPipe,"position startpos\n");
      }
      else
      {
         fprintf(sfPipe,"position startpos moves %s\n", moveList);
      }
   }
   // Not starting position...
   else
   {
      DPRINT("Setting board to %s\n", fen);

      // Should the engine apply a move list?
      if(moveList == NULL)
      {
         fprintf(sfPipe,"position fen %s\n", fen);
      }
      else
      {
         fprintf(sfPipe,"position fen %s moves %s\n", fen, moveList);
      }
      // TODO grab results.
   }
}

void SF_findMove( uint32_t wt, uint32_t bt, uint32_t wi, uint32_t bi)
{

   if(sfPipe == NULL)
   {
      DPRINT("SF_findMove called with uninitialized stockfish pipe\n");
      return;
   }

   DPRINT("Computer beginning time-budgeted search\n");

   fprintf(sfPipe, "go wtime %d btime %d winc %d binc%d\n", wt, bt, wi, bi );
}

void SF_findMoveFixedDepth( int d )
{
   if(sfPipe == NULL)
   {
      DPRINT("SF_findMoveFixedDepth called with uninitialized stockfish pipe\n");
      return;
   }

   DPRINT("Computer beginning fixed-depth search of %d ply\n", d);
   fprintf(sfPipe,"go depth %d\n", d);
}

void SF_findMoveFixedTime( uint32_t t )
{
   if(sfPipe == NULL)
   {
      DPRINT("SF_findMoveFixedTime called with uninitialized stockfish pipe\n");
      return;
   }

   DPRINT("Computer beginning fixed-time search of %dms\n", t);
   fprintf(sfPipe,"go movetime %d\n", t);
}

void SF_stop( void )
{
   if(sfPipe == NULL)
   {
      DPRINT("SF_stop called with uninitialized stockfish pipe\n");
      return;
   }
   fprintf(sfPipe,"stop\n");
}

static void *enginePollTask ( void *arg )
{
   #define MAX_LINE_LEN 100

   char engineResultLine[MAX_LINE_LEN];

   FILE *tmpFile;

   while(1)
   {
      usleep(250000);

      tmpFile = fopen(OUTPUT_FILE, "r+");

      if(tmpFile != NULL)
      {
         fgets(engineResultLine, MAX_LINE_LEN, tmpFile);

         if(!strncmp(engineResultLine, "bestmove", 8))
         {
            selectedMove = convertCoordMove(&engineResultLine[9]);

            if(selectedMove.promote != PIECE_NONE)
            {
               ponderMove = convertCoordMove(&engineResultLine[22]);
            }
            else
            {
               ponderMove = convertCoordMove(&engineResultLine[21]);
            }
            computerMove_engineSelection(selectedMove, ponderMove);

         }
         fclose(tmpFile);
         remove(OUTPUT_FILE);
      }
   }

   return NULL;
}
