#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "diag.h"
#include "sfInterface.h"
#include "util.h"
#include "options.h"

FILE *sfPipe = NULL;

void SF_initEngine( void )
{

   char skillLevelText[3];

   // Start Stockfish, pipe output to sfOutput.txt
   sfPipe = popen("Stockfish6/stockfish > sfOutput.txt", "w");

   // Verify pipe was successfull.
   if(sfPipe == NULL)
   {
      DPRINT("Failed to open pipe for stockfish engine\n");
      return;
   }

   // Remove buffering so sprintf will send commands immediately.
   setbuf(sfPipe, NULL);

   // Set up our default parameters to Stockfish
   SF_setOption("Threads",     "4");

   sprintf(skillLevelText, "%d\n", options.engine.strength);
   SF_setOption("Skill Level", skillLevelText);
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
move_t SF_checkDone( move_t *ponder )
{

   FILE *tmpFile;
   static move_t retValue;
   char lastLine[MAX_LINE_LEN];

   retValue.from    = 0;
   retValue.to      = 0;
   retValue.promote = (unsigned short)PIECE_NONE;

   // Grab last line of output, save to temporary file
   system("tail --lines=1 sfOutput.txt > examine.txt");

   // Open it to see what we got..
   tmpFile = fopen("examine.txt", "r");

   if(tmpFile != NULL)
   {
      fgets(lastLine, MAX_LINE_LEN, tmpFile);

      if(!strncmp(lastLine, "bestmove", 8))
      {
         retValue = convertCoordMove(&lastLine[9]);

         if(ponder != NULL)
         {
            move_t p;
            if(retValue.promote != PIECE_NONE)
            {
               p = convertCoordMove(&lastLine[22]);
            }
            else
            {
               p = convertCoordMove(&lastLine[21]);
            }
            memcpy(ponder,&p,sizeof(move_t));

         }
      }
      fclose(tmpFile);
   }

   return retValue;

   // Loop until we see "bestmove" on last line
}
