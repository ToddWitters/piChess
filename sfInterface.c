#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "diag.h"
#include "sfInterface.h"
#include "util.h"
#include "options.h"
#include "st_computerMove.h"
#include "hsmDefs.h"
#include "event.h"

#include <pthread.h>

#include <stropts.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>


// #define SF_EXE      "/home/pi/chess/stockfish > sfOutput.txt"
#define SF_EXE      "/home/pi/chess/stockfish > /dev/null"

FILE *sfPipe = NULL;
struct pollfd fds[1];

static pthread_t enginePollThread;

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

   sprintf(skillLevelText, "%ld", getOptionVal("engineStrength"));
   SF_setOption("Skill Level", skillLevelText);
}

void SF_closeEngine( void )
{
   pthread_cancel(enginePollThread);
   if(sfPipe != NULL)
   {
      fprintf(sfPipe, "quit\n");
      pclose(sfPipe);
      sfPipe = NULL;

   }
}

void SF_setOption( char *name, char *value)
{
   fprintf(sfPipe,"setoption name %s value %s\n", name, value);
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
         DPRINT("Setting move list to %s\n", moveList);
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
         DPRINT("Setting move list to %s\n", moveList);
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

void SF_go( void )
{
   if(sfPipe == NULL)
   {
      DPRINT("SF_go called with uninitialized stockfish pipe\n");
      return;
   }
   DPRINT("Starting untimed computer analysis");
   fprintf(sfPipe,"go infinite\n");
}


// Another possibility...
// http://www.tldp.org/LDP/lpg/node15.html#SECTION00730000000000000000

extern bool_t computerMovePending;

static void *enginePollTask ( void *arg )
{
   while(1)
   {
      usleep(50000);

      if( computerMovePending == false && access( OUTPUT_FILE, R_OK ) != -1 )
      {
         event_t ev = {EV_PROCESS_COMPUTER_MOVE, 0};
         usleep(100000);
         putEvent(EVQ_EVENT_MANAGER, &ev);
         usleep(100000);
      }
   }

   return NULL;
}
