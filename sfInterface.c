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

// TODO Add moves to make from position
//
void SF_setPosition( char *fen)
{
   // Verify pipe first
   if(sfPipe == NULL)
   {
      DPRINT("setPosition called with uninitialized stockfish pipe\n");
   }

   else if(fen == NULL)
   {
      DPRINT("Setting board to initial position\n");
      fprintf(sfPipe,"position startpos\n");
   }
   else
   {
      DPRINT("Setting board to %s\n", fen);
      fprintf(sfPipe,"position fen %s\n", fen);
      // TODO grab results.
   }
}

void SF_findMoveFixedDepth( int d )
{
   if(sfPipe == NULL)
   {
      DPRINT("findMoveFixedDepth called with uninitialized stockfish pipe\n");
      return;
   }

   DPRINT("Computer beginning fixed-depth search of %d ply\n", d);
   fprintf(sfPipe,"go depth %d\n", d);
}

void SF_findMoveFixedTime( uint32_t t )
{
   if(sfPipe == NULL)
   {
      DPRINT("findMoveFixedTime called with uninitialized stockfish pipe\n");
      return;
   }

   DPRINT("Computer beginning fixed-time search of %dms\n", t);
   fprintf(sfPipe,"go movetime %d\n", t);
}

// TODO return move if done.

// TODO... Make this into a task that waits for available data on
//   a file descriptor


static void *enginePollTask ( void *arg )
{
   #define MAX_LINE_LEN 100

   int  ret;
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
