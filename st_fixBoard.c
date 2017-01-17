#include <stdio.h>
#include <string.h>

#include "hsm.h"
#include "hsmDefs.h"

#include "display.h"
#include "switch.h"
#include "led.h"
#include "bitboard.h"
#include "event.h"
#include "st_fixBoard.h"
#include "options.h"
#include "board.h"

extern game_t game;

static uint64_t dirtySquares;
static piece_t currentPiece;
static color_t currentColor;

board_t targetBoard;
extern bool_t computerMovePending;

void fixBoardEntry( event_t ev)
{

	displayClear();
	LED_AllOff();

	displayWriteLine(0, "Fixing board...", true);
	displayWriteLine(3, "btn = abort game", true);

	currentPiece = PAWN;
	currentColor = WHITE;

	memcpy(&targetBoard, &game.brd, sizeof(board_t));

	// If we were in process of entering a computer move, back up to the pre-move position....
	if( computerMovePending )
		unmove(&targetBoard, game.posHistory[game.playedMoves-1].revMove);

	fixBoard_boardChange(ev);

}

static const char *pieceName[] =
{
   "pawn",
   "knight",
   "bishop",
   "rook",
   "queen",
   "king",
};

static const char *colorName[] =
{
   "black",
   "white"
};

void fixBoard_boardChange( event_t ev )
{

	char tempStr[21];

	uint64_t current = GetSwitchStates();
	uint64_t missing   = ~current &  ( targetBoard.colors[WHITE] | targetBoard.colors[BLACK] );
	uint64_t extra     =  current & ~( targetBoard.colors[WHITE] | targetBoard.colors[BLACK] );

	// Remove dirty squares that are no longer occupied by a piece
	dirtySquares &= current;

	// Add list of dirty squares to those the extras to ensure they are removed..
	extra |= dirtySquares;

	// Step 1:  Make sure any extra or dirty pieces are removed...

	if(extra != 0)
	{
		LED_SetGridState(0);
		LED_FlashGridState(extra);
		displayWriteLine(2, "Remove pieces", true);
	}

	// Step 2: Add groups of pieces

	// Are there more of the currently selected piece?
	else if (missing != 0)
	{
		while ( ! (missing & targetBoard.colors[currentColor] & targetBoard.pieces[currentPiece]) )
		{
			if(currentPiece == KING)
			{
				currentPiece = PAWN;
				if(currentColor == WHITE)
					currentColor = BLACK;
				else
					currentColor = WHITE;
			}
			else
			{
				currentPiece++;
			}
		}

		LED_FlashGridState(0);
		LED_SetGridState(missing & targetBoard.colors[currentColor] & targetBoard.pieces[currentPiece]);
		sprintf(tempStr,
			    "Place %s %s",
			    colorName[currentColor],
			    pieceName[currentPiece]
			    );

		displayWriteLine(2, tempStr, true);
	}

	else
	{
		event_t ev;

		LED_AllOff();
		ev.ev   = EV_GOTO_PLAYING_GAME;
		ev.data = 0;
		putEvent(EVQ_EVENT_MANAGER, &ev);
	}

}

void fixBoardExit( event_t ev)
{
	displayClear();
}

void fixBoard_setDirty( uint64_t d)
{
	dirtySquares = d;
}
