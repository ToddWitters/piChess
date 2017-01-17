#include "st_checkBoard.h"
#include "switch.h"
#include "st_fixBoard.h"
#include "event.h"
#include "display.h"
#include "stdio.h"
#include "constants.h"
#include "led.h"

#include "hsm.h"
#include "hsmDefs.h"

extern game_t game;

color_t currentColor;
piece_t currentPiece;

static void showCurrentPiece( color_t color, piece_t piece);

void checkBoardEntry( event_t ev)
{
   currentColor = WHITE;
   currentPiece = KING;

   displayWriteLine(0, "Verifying Board...", true);
   showCurrentPiece( currentColor, currentPiece );
   displayWriteLine(3, "Press btn when done", true);

}

void checkBoardExit( event_t ev)
{
   LED_AllOff();
   displayClear();
}

void checkBoard_handleButtonPosChange (event_t ev)
{
   int dir;

   if(ev.ev != EV_BUTTON_POS) return;

   switch(ev.data)
   {
      case POS_UP:
      case POS_LEFT:
         dir = 1;
         break;

      case POS_DOWN:
      case POS_RIGHT:
         dir = -1;
         break;

      default:
         return; // This covers a return to center....
   }

   do
   {
      bool wrap = false;
      if(dir == 1)
      {
         if(currentPiece == KING)
         {
            currentPiece = PAWN;
            wrap = true;
         }
         else currentPiece++;
      }
      else // dir = -1
      {
         if(currentPiece == PAWN)
         {
            currentPiece = KING;
            wrap = true;
         }
         else currentPiece--;
      }

      if(wrap)
      {
         if(currentColor == WHITE) currentColor = BLACK; else currentColor = WHITE;
      }

   }while( !(game.brd.colors[currentColor] & game.brd.pieces[currentPiece]));

   showCurrentPiece(currentColor, currentPiece);
}

static void showCurrentPiece( color_t color, piece_t piece)
{
   char tempStr[6];

   sprintf(tempStr, "%s", colorNames[color]);
   displayWriteLine(1, tempStr, true);

   sprintf(tempStr, "%s", pieceNames[piece]);
   displayWriteLine(2, tempStr, true);

   LED_SetGridState(game.brd.colors[color] & game.brd.pieces[piece]);
}

