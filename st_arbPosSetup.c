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

// [Placing WHITE KNIGHT]
// [   toMove: White    ]
// [castle: KQkq   ep:e4] OR [ ERROR MESSAGE     ]
// [     btn = XXX      ]

bool boardPrecleared = false;
color_t currentColor;
piece_t currentPiece;
uint64_t boardState;
board_t brd;
uint8_t lastLiftSquare = 64;
uint8_t lastCastleBits = 0;
uint8_t lastEnPassantCol = 8;

static void	arbPosShowSelectedPiece( void );
static void showCurrentSelectionLocations( void );
static void showCastlingEnPassant( void );


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

	if(boardPrecleared == true)
	{
		displayWriteLine(1, "   toMove: White", false);
		arbPosShowSelectedPiece();
	}
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

	// If there are no kings don't allow changing the piece...
	if( !(brd.colors[WHITE] & brd.pieces[KING])) return;
	if( !(brd.colors[BLACK] & brd.pieces[KING])) return;

	lastLiftSquare = 64;

	switch(ev.data)
	{
		case POS_LEFT:
		case POS_RIGHT:
			if(currentColor == WHITE) currentColor = BLACK;
			else                      currentColor = WHITE;
			lastEnPassantCol = brd.enPassantCol = 8;
			showCastlingEnPassant();

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

	// If we are still clearing the board...
	if(boardPrecleared == false)
	{
		// Flash currently occupied
		boardState = GetSwitchStates();
		LED_FlashGridState(boardState);

		// If we just changed to the cleared state...
		if(boardState == 0)
		{
			// Start with white to move, and white king to be placed...
			boardPrecleared = true;
			displayWriteLine(1, "   toMove: White", false);
			arbPosShowSelectedPiece();
		}

	}
	else
	{
		uint8_t location = ev.data;

		// We just removed a piece
		if(ev.ev == EV_PIECE_LIFT)
		{
			// Find the piece and color at the given square...
			color_t thisColor = getColorAtSquare(&brd, ev.data);
			piece_t thisPiece = getPieceAtSquare(&brd, ev.data);

			// Keep track of the last place we lifted a piece.
			lastLiftSquare = location;

			// Assuming we found a piece at this square...
			if( (thisColor != COLOR_NONE) && (thisPiece != PIECE_NONE))
			{
				// Remove it from the board
				removePiece(&brd, location, thisPiece, thisColor);

				// Change the current piece/color to this piece.
				currentColor = thisColor;
				currentPiece = thisPiece;

				// Display this piece
				arbPosShowSelectedPiece();
			}

            // Reset enPassant if we remove the affected pawn
            if(brd.enPassantCol != 8 && thisPiece == PAWN)
            {
               if((location % 8) == brd.enPassantCol)
               {
                  if( ( (brd.toMove == BLACK) && (squareMask[location] & rowMask[4]))
                  	  ||
                  	  ( (brd.toMove == WHITE) && (squareMask[location] & rowMask[3])))
                  {
                     lastEnPassantCol = brd.enPassantCol;
                     brd.enPassantCol = 8;
                  }
               }
            }

            // If we lifted a ROOK, grab the current state of the castle bits in
            //   case it is immediately dropped again (signaling a toggle of the caslte
            //   state), then clear the castle bits..
            if(thisPiece == ROOK)
            {
               lastCastleBits = brd.castleBits;

               if       (location == A1) brd.castleBits &= ~WHITE_CASTLE_LONG;
               else if  (location == H1) brd.castleBits &= ~WHITE_CASTLE_SHORT;
               else if  (location == A8) brd.castleBits &= ~BLACK_CASTLE_LONG;
               else if  (location == H8) brd.castleBits &= ~BLACK_CASTLE_SHORT;

               showCastlingEnPassant();
            }

            // If we lifted a KING, grab the current state of the castle bits in
            //   case it is immediately dropped again (signaling a toggle of the caslte
            //   state), then clear the castle bits..
            if(thisPiece == KING)
            {
               lastCastleBits = brd.castleBits;

               if       (location == E1) brd.castleBits &= ~(WHITE_CASTLE_LONG | WHITE_CASTLE_SHORT);
               else if  (location == E8) brd.castleBits &= ~(BLACK_CASTLE_LONG | BLACK_CASTLE_SHORT);

               lastEnPassantCol = brd.enPassantCol;
               brd.enPassantCol = 8;
               showCastlingEnPassant();
            }
		}
		else if(ev.ev == EV_PIECE_DROP)
		{
			// Add the piece to the board.
			addPiece(&brd, location, currentPiece, currentColor);

			// Check for special gestures when a piece is lifted and dropped back again to the same square.
			// (1) if piece is king, change side to move to the color of the king
			// (2) if piece is rook AND it and it's king are on the right squares, toggle the castling bits
			// (3) if piece is pawn AND it is on the right rank (and the color NOT on move) toggle enPassant

			// Are we dropping this piece back down to the square we lifted it from?
			if(lastLiftSquare == location)
			{

				// If it was a king, change side to move
				if(currentPiece == KING)
				{
					brd.castleBits = lastCastleBits;
               		showCastlingEnPassant();

               		if(brd.toMove == currentColor)
               		   brd.enPassantCol = lastEnPassantCol;

               		// Change side to move
               		brd.toMove = currentColor;
               		displayWriteChars(1,11,strlen(colorNames[brd.toMove]),(char *)colorNames[brd.toMove]);

				}

				// If it was a ROOK (and the rook/king are on the right squares...)
				else if(currentPiece == ROOK)
				{
					if(currentColor == WHITE)
					{
						if( brd.pieces[KING] & brd.colors[WHITE] & squareMask[E1])
						{
							if(location == A1)
							{
                        		// Restore, then toggle state of castling....
                        		brd.castleBits = lastCastleBits;
								brd.castleBits ^= WHITE_CASTLE_LONG;
                        		lastCastleBits = brd.castleBits;
								showCastlingEnPassant();
							}
							else if(location == H1)
							{
                        		// Restore, then toggle state of castling....
                        		brd.castleBits = lastCastleBits;
								brd.castleBits ^= WHITE_CASTLE_SHORT;
                        		lastCastleBits = brd.castleBits;
								showCastlingEnPassant();
							}
						}
					}
					else
					{
						if( brd.pieces[KING] & brd.colors[BLACK] & squareMask[E8])
						{
							if(location == A8)
							{
                        		// Restore, then toggle state of castling....
                        		brd.castleBits = lastCastleBits;
								brd.castleBits ^= BLACK_CASTLE_LONG;
                        		lastCastleBits = brd.castleBits;
								showCastlingEnPassant();
							}
							else if(location == H8)
							{
                        		// Restore, then toggle state of castling....
                        		brd.castleBits = lastCastleBits;
								brd.castleBits ^= BLACK_CASTLE_SHORT;
                        		lastCastleBits = brd.castleBits;
								showCastlingEnPassant();
							}
						}
					}
				}

				// If it was a pawn of the side not on move...
				else if( currentPiece == PAWN )
				{
					if( brd.toMove == WHITE && currentColor == BLACK )
					{
						if(squareMask[location] & rowMask[3])
						{
                     		brd.enPassantCol = lastEnPassantCol;

							// No enpassant piece currently selected OR a different file is selected
							if(brd.enPassantCol != location % 8)
								brd.enPassantCol = location % 8;
							else
								brd.enPassantCol = 8;

                     		lastEnPassantCol = brd.enPassantCol;

							showCastlingEnPassant();
						}
					}
					else if(brd.toMove == BLACK && currentColor == WHITE)
					{
						if(squareMask[location] & rowMask[4])
						{
                     		brd.enPassantCol = lastEnPassantCol;

							if(brd.enPassantCol != location % 8)
								brd.enPassantCol = location % 8;
							else
								brd.enPassantCol = 8;

                     		lastEnPassantCol = brd.enPassantCol;

							showCastlingEnPassant();
						}
					}
				}
			}

			// Whenever a king is dropped, check to see which (if any) still need to be added...
			if(currentPiece == KING)
			{
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
		showCastlingEnPassant();
		displayWriteLine(3, "Press to start game", true);
	}

}

bool arbPosSetupCheckPosition(event_t ev)
{
   bool_t result = ( (ev.data == B_PRESSED) && (testValidBoard(&brd) == BRD_NO_ERROR) );

   if(result == true)
   {

   	   // since we are going to return true for this guard position, do these things now...

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
	displayWriteLine(0, lineContents, false);
}


static void showCurrentSelectionLocations( void )
{
	LED_SetGridState(brd.colors[currentColor] & brd.pieces[currentPiece]);
}

static void showCastlingEnPassant( void )
{
	char tempStr[21] = {0};

	if(brd.castleBits)
	{
		strcat(tempStr, "castle: ");

		if(brd.castleBits & WHITE_CASTLE_SHORT)
			strcat(tempStr, "K");
		else
			strcat(tempStr, "-");

		if(brd.castleBits & WHITE_CASTLE_LONG)
			strcat(tempStr, "Q");
		else
			strcat(tempStr, "-");

		if(brd.castleBits & BLACK_CASTLE_SHORT)
			strcat(tempStr, "k");
		else
			strcat(tempStr, "-");

		if(brd.castleBits & BLACK_CASTLE_LONG)
			strcat(tempStr, "q");
		else
			strcat(tempStr, "-");
	}

	if(brd.enPassantCol != 8)
	{
		char tempStr2[3];

		tempStr2[0] = 'a' + brd.enPassantCol;
		tempStr2[1] = ( brd.toMove == WHITE ? '6' : '3' );
		tempStr2[2] = '\0';

		if(tempStr[0]) strcat(tempStr, "  ");
		strcat(tempStr, "ep:");
		strcat(tempStr, tempStr2);
	}

	displayWriteLine(2,tempStr,true);


}
