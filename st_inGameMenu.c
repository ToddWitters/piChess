#include "hsm.h"
#include "hsmDefs.h"
#include "st_inGameMenu.h"

#include <stddef.h>
#include "menu.h"
#include "display.h"
#include "board.h"
#include <string.h>
#include "constants.h"
#include "st_fixBoard.h"

#include "diag.h"
extern game_t game;

menu_t *inGameMenu = NULL;


void inGameMenuEntry( event_t ev )
{
   if(inGameMenu == NULL)
   {
      inGameMenu = createMenu("---- Game Menu -----", 0);

      //          menu      offset      text                   press                  right   picker
      menuAddItem(inGameMenu, ADD_TO_END, "Back to Game",        EV_GOTO_PLAYING_GAME,  0,      NULL);
      menuAddItem(inGameMenu, ADD_TO_END, "Take back move",      EV_TAKEBACK,           0,      NULL);
      menuAddItem(inGameMenu, ADD_TO_END, "Abort Game",          EV_GOTO_MAIN_MENU,     0,      NULL);
      menuAddItem(inGameMenu, ADD_TO_END, "Verify Board",        EV_START_BOARD_CHECK,  0,      NULL);
   }

   drawMenu(inGameMenu);

}

void inGameMenuExit( event_t ev )
{
   destroyMenu(inGameMenu);
   inGameMenu = NULL;
   displayClear();
}

void gameMenu_goBack2( event_t ev)
{
   int i;
   uint64_t dirtySquares = 0;

   if(game.playedMoves >= 2)
   {

      for(i=0;i<2;i++)
      {
         char *tempPtr;
         // Save off existing board...
         board_t prev;
         memcpy(&prev, &game.brd, sizeof(board_t));

         // Undo the last move
         revMove_t rev = game.posHistory[game.playedMoves -1].revMove;
         unmove(&game.brd, rev);

         // Mark as dirty the newly occupied square
         dirtySquares |= squareMask[rev.move.to];
         DPRINT("dirty: %016llX\n", dirtySquares );

         // Ensure Rook moves during castling are marked too...
         if(rev.priorCastleBits != game.brd.castleBits)
         {
            if(rev.move.from == E1)
            {
               if(rev.move.to == C1)
                  dirtySquares |= d1;
               else if(rev.move.to == G1)
                  dirtySquares |= f1;
            }
            else if(rev.move.from == E8)
            {
               if(rev.move.to == C8)
                  dirtySquares |= d8;
               else if(rev.move.to == G8)
                  dirtySquares |= f8;
            }
         }

         tempPtr = &game.moveRecord[strlen(game.moveRecord)];
         DPRINT("[%s]", game.moveRecord);

         while(*tempPtr != ' ' && (tempPtr != game.moveRecord))
            tempPtr--;

         *tempPtr = '\0';
         DPRINT("[%s]", game.moveRecord);

         tempPtr = &game.SANRecord[strlen(game.SANRecord)];
         DPRINT("[%s]", game.SANRecord);

         while(*tempPtr != ' ' && (tempPtr != game.SANRecord))
            tempPtr--;

         *tempPtr = '\0';
         DPRINT("[%s]", game.SANRecord);

         // TODO... update moveRecord && SANRecord

         // TODO... restore clocks

         game.playedMoves--;
      }

      // Remove those dirty squares that are not currently occupied...
      dirtySquares &= (game.brd.colors[WHITE] | game.brd.colors[BLACK]);

      DPRINT("dirty: %016llX\n", dirtySquares );


      fixBoard_setDirty(dirtySquares);
   }
   else
   {
      fixBoard_setDirty(0);
   }

}
