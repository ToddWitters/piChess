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

#include <string.h>
#include <stdlib.h>

bool boardPrecleared = false;
color_t currentColor;
piece_t currentPiece;
uint64_t boardState;
board_t brd;

static void	arbPosShowSelectedPiece( void );
static piece_t getPieceAtSquare( uint8_t sq );
static color_t getColorAtSquare( uint8_t sq );
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

	displayWriteLine(3, "Press btn to exit", true);


}
void arbPosSetupExit( event_t ev)
{
   LED_AllOff();

}


void arbPosSetupHandleBtnPos( event_t ev)
{

	if(boardPrecleared == false) return;

	switch(ev.data)
	{
		case POS_LEFT:
		case POS_RIGHT:
			if(currentColor == WHITE) currentColor = BLACK;
			else                      currentColor = WHITE;
			break; 
		case POS_UP:
			if(++currentPiece == PIECE_NONE) currentPiece = PAWN;
			break;
		case POS_DOWN:
			if(currentPiece == PAWN) currentPiece = KING;
			else                           --currentPiece;
			break;
	}
	arbPosShowSelectedPiece();
	showCurrentSelectionLocations();
}

void arbPosSetup_boardChange( event_t ev)
{
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
			color_t thisColor = getColorAtSquare(ev.data);
			piece_t thisPiece = getPieceAtSquare(ev.data);
			if( (thisColor != COLOR_NONE) && (thisPiece != PIECE_NONE))
			{
				removePiece(&brd, location, thisPiece, thisColor);
				currentColor = thisColor;
				currentPiece = thisPiece;
				arbPosShowSelectedPiece();
			}
			else
			{
				DPRINT("Unknown piece at location");
			}
		}
		else if(ev.ev == EV_PIECE_DROP)
		{
			addPiece(&brd, location, currentPiece, currentColor);
			if(currentPiece == KING)
			{
				brd.toMove = currentColor;
				displayWriteChars(0,3,strlen(colorNames[brd.toMove]),(char *)colorNames[brd.toMove]);
			}
		}
	}

	showCurrentSelectionLocations();

	if(testValidBoard(&brd) != BRD_NO_ERROR)
	{
		displayWriteLine(2, "Illegal position", true);
		displayWriteLine(3, "Press btn to exit", true);
	}
	else
	{
		displayClearLine(2);
		displayWriteLine(3, "Press btn to accept", true);
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

	sprintf(lineContents,"%s %s", colorNames[currentColor], pieceNames[currentPiece]);
	displayWriteLine(1, lineContents, true);
}	

static piece_t getPieceAtSquare( uint8_t sq )
{
	uint64_t mask = squareMask[sq];

	if(brd.pieces[PAWN] & mask) return PAWN;
	if(brd.pieces[KNIGHT] & mask) return KNIGHT;
	if(brd.pieces[BISHOP] & mask) return BISHOP;
	if(brd.pieces[ROOK] & mask) return ROOK;
	if(brd.pieces[QUEEN] & mask) return QUEEN;
	if(brd.pieces[KING] & mask) return KING;
	return PIECE_NONE;
}


static color_t getColorAtSquare( uint8_t sq )
{
	uint64_t mask = squareMask[sq];

	if(brd.colors[BLACK] & mask) return BLACK;
	if(brd.colors[WHITE] & mask) return WHITE;
	return COLOR_NONE;
}

static void showCurrentSelectionLocations( void )
{
	LED_SetGridState(brd.colors[currentColor] & brd.pieces[currentPiece]);	
}
