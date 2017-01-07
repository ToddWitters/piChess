#include "hsm.h"
#include "st_arbPosSetup.h"
#include "display.h"
#include "switch.h"
#include "led.h"
#include "types.h"
#include "stdio.h"
#include "board.h"
#include "constants.h"
#include "hsmDefs.h"
#include "diag.h"
#include "util.h"
#include "bitboard.h"
#include "diag.h"

#include <string.h>
#include <stdlib.h>

bool boardPrecleared = false;
color_t currentColor;
piece_t currentPiece;
uint64_t boardState;
board_t brd;

static void	arbPosShowSelectedPiece( void );
static void showCurrentSelectionLocations( void );

extern game_t game;

const char* pieceNames[] =
{
	"Pawn",
	"Knight",
	"Bishop",
	"Rook",
	"Queen",
	"King"
};

const char* colorNames[] =
{
	"Black",
	"White"
};

void arbPosSetupEntry( event_t ev)
{

	setBoardEmpty(&brd);

	boardState = GetSwitchStates();

	currentColor = WHITE;
	currentPiece = KING;

	if(boardState != 0) boardPrecleared = false;
	else                boardPrecleared = true;

	displayClear();

	displayWriteChars(0,3,13, "White to move");

	if(boardPrecleared == true)	
		arbPosShowSelectedPiece();	
	else
	{
		displayWriteLine(1, "Remove all pieces", true);
        LED_FlashGridState(boardState);
	}

	displayWriteLine(3, "Press btn to abort", true);


}
void arbPosSetupExit( event_t ev)
{
   LED_AllOff();

}


void arbPosSetupHandleBtnPos( event_t ev)
{

	if(boardPrecleared == false) return;

	// If king counts are wrong, don't allow changing the piece...
	if( bitCount(brd.colors[WHITE] & brd.pieces[KING]) != 1) return;
	if( bitCount(brd.colors[BLACK] & brd.pieces[KING]) != 1) return;

	switch(ev.data)
	{
		case POS_LEFT:
		case POS_RIGHT:
			if(currentColor == WHITE) currentColor = BLACK;
			else                      currentColor = WHITE;
			break; 
		case POS_UP:
			if(++currentPiece == KING) currentPiece = PAWN;
			break;
		case POS_DOWN:
			if(currentPiece == PAWN) currentPiece = QUEEN;
			else                           --currentPiece;
			break;
	}
	arbPosShowSelectedPiece();
	showCurrentSelectionLocations();
}

void arbPosSetup_boardChange( event_t ev)
{

	boardErr_t err;
			
	if(boardPrecleared == false)
	{
		boardState = GetSwitchStates();
		LED_FlashGridState(boardState);
		if(boardState == 0)
		{
			boardPrecleared = true;
			arbPosShowSelectedPiece();
		}

	}
	else
	{
		uint8_t location = ev.data;

		if(ev.ev == EV_PIECE_LIFT)
		{
			color_t thisColor = getColorAtSquare(&brd, ev.data);
			piece_t thisPiece = getPieceAtSquare(&brd, ev.data);
			if( (thisColor != COLOR_NONE) && (thisPiece != PIECE_NONE))
			{
				removePiece(&brd, location, thisPiece, thisColor);
				currentColor = thisColor;
				currentPiece = thisPiece;
				arbPosShowSelectedPiece();
			}
		}
		else if(ev.ev == EV_PIECE_DROP)
		{
			addPiece(&brd, location, currentPiece, currentColor);
			if(currentPiece == KING)
			{
				brd.toMove = currentColor;
				displayWriteChars(0,3,strlen(colorNames[brd.toMove]),(char *)colorNames[brd.toMove]);

				if( !(brd.colors[WHITE] & brd.pieces[KING]))
					currentColor = WHITE;
				else if( !(brd.colors[BLACK] & brd.pieces[KING]) )
					currentColor = BLACK;
				else
					currentPiece = PAWN;
			}
			arbPosShowSelectedPiece();

		}
	}

	showCurrentSelectionLocations();

	if( (err = testValidBoard(&brd)) != BRD_NO_ERROR)
	{
		switch(err)
		{
	       case ERR_BAD_WHITE_KING_COUNT:
           case ERR_BAD_BLACK_KING_COUNT:
	  		  displayWriteLine(2, "", true);
           	  break;

           case ERR_OPPOSING_KINGS:
	  		  displayWriteLine(2, "King's are adjacent", true);
           	  break;

           case ERR_OPP_ALREADY_IN_CHECK:
 	  		  displayWriteLine(2, "King is capturable", true);             
           	  break;

           case ERR_BAD_PAWN_POSITION:
 	  		  displayWriteLine(2, "Pawn on illegal row", true);             
           	  break;

           case ERR_TOO_MANY_WHITE_PIECES:
 	  		  displayWriteLine(2, ">16 white pieces", true);             
           	  break;

           case ERR_TOO_MANY_BLACK_PIECES:
 	  		  displayWriteLine(2, ">16 black pieces", true);             
           	  break;

           case BRD_NO_ERROR:
           default:
           	  break;	  

		}
		displayWriteLine(3, "Press btn to abort", true);
	}
	else
	{
		displayClearLine(2);
		displayWriteLine(3, "Press to start game", true);
	}

}

bool arbPosSetupCheckPosition(event_t ev)
{
   bool_t result = ( (ev.data == B_PRESSED) && (testValidBoard(&brd) == BRD_NO_ERROR) );

   if(result == true)
   {
   	  char* currentFEN = getFEN(&brd);
   	  game.startPos = malloc(strlen(currentFEN)+1);
      strcpy( game.startPos, currentFEN);

      memcpy(&game.brd, &brd, sizeof(board_t));

      game.disposition = GAME_PLAYABLE;
   }

   return result;
}

static void	arbPosShowSelectedPiece( void )
{
	char lineContents[21];

	sprintf(lineContents,"Placing %s %s", colorNames[currentColor], pieceNames[currentPiece]);
	displayWriteLine(1, lineContents, false);
}	


static void showCurrentSelectionLocations( void )
{
	LED_SetGridState(brd.colors[currentColor] & brd.pieces[currentPiece]);	
}
