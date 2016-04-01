#include "board.h"
#include "bitboard.h"
#include "constants.h"
#include "debug.h"
#include "zobrist.h"
// #include "eval.h"

#include <stdlib.h> // abs function
#include <string.h>	// memcpy function
#include <ctype.h>

// Local functions to help with move function
static void movePiece  (board_t *b, U8 fromSq, U8 toSq, piece_t p, color_t c);

const char *startString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

U64 positionHistory[POSITION_HISTORY_SIZE];
int positionIndex = 0;

// Initialize board with given string
//   NOTE:  Last two fields of FEN (half-move count and move number) are optional.  This allows
//   Function to work with a string consisting of the first four fields of an EPD record
fenErr_t setBoard(board_t *brd, const char *FEN)
{


	 // Local copy of a board upon which to do all operations...
	 board_t b;

	 // Index into the 64 squares of the board...
	 int index = 0;

	 // Initialize the board settings that are not directly set below...
	 b.materialCount[WHITE] = 0;
	 b.materialCount[BLACK] = 0;
	 b.colors[WHITE] = 0;
	 b.colors[BLACK] = 0;
	 b.pieces[PAWN] = 0;
	 b.pieces[KNIGHT] = 0;
	 b.pieces[BISHOP] = 0;
	 b.pieces[ROOK] = 0;
	 b.pieces[QUEEN] = 0;
	 b.pieces[KING] = 0;
	 b.hash = 0;
	 b.castleBits = 0;

    if (FEN == NULL) FEN = startString;
    
	 // parse piece placement field...
	 while(index < 64)
	 {
	 	switch(*FEN)
	 	{
	 		case '1':
	 		case '2':
	 		case '3':
	 		case '4':
	 		case '5':
	 		case '6':
	 		case '7':
	 		case '8':
	 			// Skip over the number of squares
	 			index += (*FEN - '0'); break;
	 		// For each possible piece, add it at the given offset...
	 		case 'B': addPiece(&b, index++, BISHOP, WHITE); break;
	 		case 'K': addPiece(&b, index++, KING,   WHITE); break;
	 		case 'N': addPiece(&b, index++, KNIGHT, WHITE); break;
	 		case 'P': addPiece(&b, index++, PAWN,   WHITE); break;
	 		case 'Q': addPiece(&b, index++, QUEEN,  WHITE); break;
	 		case 'R': addPiece(&b, index++, ROOK,   WHITE); break;
	 		case 'b': addPiece(&b, index++, BISHOP, BLACK); break;
	 		case 'k': addPiece(&b, index++, KING,   BLACK); break;
	 		case 'n': addPiece(&b, index++, KNIGHT, BLACK); break;
	 		case 'p': addPiece(&b, index++, PAWN,   BLACK); break;
	 		case 'q': addPiece(&b, index++, QUEEN,  BLACK); break;
	 		case 'r': addPiece(&b, index++, ROOK,   BLACK); break;

			// This is just to make the string more readable... just ignore.
	 		case '/': break;

			// Any other characters denote an error
	 		default: return FEN_ILLEGAL_CHARACTER_FIELD_1; break;

	 	}
	 	FEN++; // move to next character.
	 }

	 // Make sure we counted 64 positions
	 if(index != 64) return FEN_PARSE_ERR_FIELD_1;

	 // Make sure we have no more characters in FIELD 1
	 if(*FEN != ' ') return FEN_PARSE_ERR_FIELD_1;

	 // Move pointer to start of field 2
	 FEN++;

	 // Should be either w or b
	 if(*FEN == 'w')
	 {
	 	// Update the position hash
	 	b.toMove = WHITE;
	 	b.hash ^= Z_WHITE_TURN_KEY;
	 }
	 else if (*FEN == 'b')
	 {
	 	b.toMove = BLACK;
	 }
	 else if (*FEN == ' ')
	 {
	 	// too many spaces between....
	 	return FEN_BAD_SEPARATOR;
	 }
	 else
	 {
	 	// found an unexpected character
	 	return FEN_INVALID_FIELD_2;
	 }

	 // Move to separator between field 2 and 3
	 FEN++;

	 // If not a space, we have an error because field 2 is longer than 1 byte
	 if(*FEN != ' ') return FEN_INVALID_FIELD_2;

	 // Move to start of field 3 (castling squares)
	 FEN++;

	 // check for too many spaces....
	 if(*FEN == ' ')
	 {
	 	return FEN_BAD_SEPARATOR;
	 }

	 // if a dash, all castle bits should be zero
	 if(*FEN == '-')
	 {
	 	FEN++;
	 	// make sure we have a proper separator.
	 	if (*FEN != ' ')
	 	{
	 		return FEN_INVALID_FIELD_3;
	 	}
	 }
	 // otherwise, check for valid castle bits
	 //  NOTE: the following allows bits to occur in any order.  May not be
	 //  strictly allowable by FEN specification.
    // TODO CHESS960
    else
	 {
		 while(*FEN != ' ')
		 {
			if(*FEN == 'K' && !(b.castleBits & WHITE_CASTLE_SHORT) )
			{
				b.castleBits |= WHITE_CASTLE_SHORT;
				b.hash ^= Z_WHITE_SHORT_KEY;
			}
			else if(*FEN == 'Q' && !(b.castleBits & WHITE_CASTLE_LONG) )
			{
				b.castleBits |= WHITE_CASTLE_LONG;
				b.hash ^= Z_WHITE_LONG_KEY;
			}
			else if(*FEN == 'k' && !(b.castleBits & BLACK_CASTLE_SHORT) )
			{
				b.castleBits |= BLACK_CASTLE_SHORT;
				b.hash ^= Z_BLACK_SHORT_KEY;
			}
			else if(*FEN == 'q' && !(b.castleBits & BLACK_CASTLE_LONG) )
			{
				b.castleBits |= BLACK_CASTLE_LONG;
				b.hash ^= Z_BLACK_LONG_KEY;
			}
			else
			{
				return FEN_INVALID_FIELD_3;
			}
			FEN++;
		 }
	 }

	 // Separator already confirmed at this point
	 FEN++;

	 // Check for no enPassant...
	 if(*FEN == '-')
	 {
	 	b.enPassantCol = 8;
	 	b.zobristEnPassantCol = 8;
	 	FEN++;
	 }

	 // else should be a-f
	 else if(*FEN >='a' && *FEN <= 'h')
	 {
	    int targetSquare;

	 	b.enPassantCol = *FEN - 'a';

    	targetSquare = b.enPassantCol + (b.toMove == WHITE ? 40 : 16);

		if( (shiftE(squareMask[targetSquare]) & b.pieces[PAWN] & b.colors[b.toMove == WHITE ? BLACK : WHITE]) ||
		    (shiftW(squareMask[targetSquare]) & b.pieces[PAWN] & b.colors[b.toMove == WHITE ? BLACK : WHITE]))
		{
		    b.zobristEnPassantCol = *FEN - 'a';
	 	    b.hash ^= Z_ENPASSANT_COL_KEY(b.enPassantCol);
	 	}


	 	FEN++;
	 	if( !(*FEN == '3' && b.toMove == BLACK) && !(*FEN == '6' && b.toMove == WHITE) )
	 	{
	 		return FEN_INVALID_FIELD_4;
	 	}
	 	else
	 	{
	 		FEN++;
	 	}
	 }
	 else if (*FEN == ' ')
	 {
	 	return FEN_BAD_SEPARATOR;
	 }
	 else
	 {
	 	return FEN_INVALID_FIELD_4;
	 }

	 // At this point, and EPD record will end.  What follows then is optional....

	 // Defaults...
	 b.halfMoves = 0;
	 b.moveNumber = 1;

	 // If string ends here, leave defaults...
	 if(*FEN != '\0')
	 {

         b.moveNumber = 0; // Since string didn't end, assume moveNumber will be included.
                           // Set to zero so logic below will work correctly...

         // Move to next character
         FEN++;

         // Should be a separator
         if(*FEN == ' ')
         {
            return FEN_BAD_SEPARATOR;
         }

         // This is the half-move counter... should be a digit...
         if(*FEN <'0' && *FEN >'9')
         {
            return FEN_INVALID_FIELD_5;
         }

         while(1)
         {
            b.halfMoves *= 10;
            b.halfMoves += *FEN - '0';
            FEN++;

            if(*FEN == ' ') break;

            if(*FEN <'0' || *FEN >'9')
            {
                return FEN_INVALID_FIELD_5;
            }
         }

         // We hit a separator, so move to next character
         FEN++;

         // This is the first digit of the move number... should be a non-zero digit...
         if(*FEN <'1' || *FEN >'9')
         {
            return FEN_INVALID_FIELD_6;
         }

         while(1)
         {
            b.moveNumber *= 10;
            b.moveNumber += *FEN - '0';

            FEN++;

            if(*FEN == '\0') break;

            if(*FEN <'0' || *FEN >'9')
            {
                return FEN_INVALID_FIELD_6;
            }
         }
    }
	memcpy(brd, &b, sizeof(board_t));

	positionHistory[0] = brd->hash;
    positionIndex = 1;

 	return FEN_OK;
}

// Move assumed valid for this board.
// Add/remove functions will ASSERT adds are to empty squares and removes have expected piece there...
// Castling moves assumed legal (if to/from square and piece moved match a castle for king), rook is also moved with king
revMove_t move(board_t *b, const move_t m)
{

	unsigned char to = m.to;
	unsigned char from = m.from;

	int offset = 0;

	revMove_t retValue;

	// A place holder for the side to move (saves repeated pointer access)
	color_t toMove = b->toMove;

	// A place to hold the "opposite color" calculation
	color_t oppositeColor = (toMove == WHITE ? BLACK : WHITE);

	// promotion piece (if any)
	piece_t promote = (piece_t)(m.promote);

	// handy marker for piece moved
	piece_t pieceMoved = ( (b->pieces[PAWN]   & squareMask[from]) ? PAWN : (
						   (b->pieces[KNIGHT] & squareMask[from]) ? KNIGHT : (
						   (b->pieces[BISHOP] & squareMask[from]) ? BISHOP : (
						   (b->pieces[ROOK]   & squareMask[from]) ? ROOK : (
						   (b->pieces[QUEEN]  & squareMask[from]) ? QUEEN : KING)))));



	// Test that side to move has proper value
	ASSERT( (toMove == WHITE) || (toMove == BLACK) );

	// Check that a piece of the side to move exists at the origin
	ASSERT( (b->colors[toMove] & squareMask[from]) != 0);




	// Prepare the return value...
	retValue.move.from    = from;
	retValue.move.to      = to;
	retValue.move.promote = promote;
	retValue.priorCastleBits = b->castleBits;
	retValue.priorEnPassant  = b->enPassantCol;
	retValue.priorHalfMoveCnt = b->halfMoves;
	retValue.priorZobristEnPassantCol = b->zobristEnPassantCol;

	retValue.captured = b->colors[oppositeColor] & squareMask[to] ?
							( (b->pieces[PAWN]   & squareMask[to]) ? PAWN : (
						     (b->pieces[KNIGHT] & squareMask[to]) ? KNIGHT : (
						     (b->pieces[BISHOP] & squareMask[to]) ? BISHOP : (
						     (b->pieces[ROOK]   & squareMask[to]) ? ROOK : QUEEN)))) : PIECE_NONE;

	// Check for capture enPassant...
	if( (pieceMoved == PAWN)       &&  // moved a pawn
	    ( (to % 8) != (from % 8) ) &&  // to and from files are different
	    ( (squareMask[to] & b->colors[oppositeColor]) == 0) // target square doesn't have enemy piece
	  )
	{
		offset = (toMove == WHITE ? 8 : -8);
		retValue.captured = PAWN;
	}

	// First remove any captured piece from the board
	if(retValue.captured != PIECE_NONE)
	{
		removePiece(b, to + offset, retValue.captured, oppositeColor );
	}

	if(promote == PIECE_NONE)
	{
		// Simply move the piece back
		movePiece(b, from, to, pieceMoved, toMove);
	}
	else
	{
		ASSERT( ( ( (to < 8) && toMove) == WHITE) || ( (to > 55) && toMove == BLACK ) );

		ASSERT( pieceMoved == PAWN);

		// Remove pawn
		removePiece(b, from, pieceMoved, toMove);

		// Add promoted piece
		addPiece(b, to, promote, toMove);

	}

	// If this was a castling move, move the rook too...
	if(pieceMoved == KING)
	{
		if(from == E1)
		{
			if(to == C1)
			{
				movePiece(b, A1, D1, ROOK, toMove);
			}
			else if(to == G1)
			{
				movePiece(b, H1, F1, ROOK, toMove);
			}
		}
		else if(from == E8)
		{
			if(to == C8)
			{
				movePiece(b, A8, D8, ROOK, toMove);
			}
			else if(to == G8)
			{
				movePiece(b, H8, F8, ROOK, toMove);
			}
		}
	}

   // TODO CHESS960
	// Adjust Castling possibilities if needed
	if( pieceMoved == KING )
	{
		if( toMove == WHITE)
		{
			if(b->castleBits & WHITE_CASTLE_LONG)
			{
				b->hash ^= Z_WHITE_LONG_KEY;
			}
			if(b->castleBits & WHITE_CASTLE_SHORT)
			{
				b->hash ^= Z_WHITE_SHORT_KEY;
			}
			b->castleBits &= ~(WHITE_CASTLE_LONG | WHITE_CASTLE_SHORT);
		}
		else
		{
			if(b->castleBits & BLACK_CASTLE_LONG)
			{
				b->hash ^= Z_BLACK_LONG_KEY;
			}
			if(b->castleBits & BLACK_CASTLE_SHORT)
			{
				b->hash ^= Z_BLACK_SHORT_KEY;
			}
			b->castleBits &= ~(BLACK_CASTLE_LONG | BLACK_CASTLE_SHORT);
		}
	}

	if( pieceMoved == ROOK )
	{
		if(from == A1 && b->castleBits & WHITE_CASTLE_LONG)
		{
			b->hash ^= Z_WHITE_LONG_KEY;
			b->castleBits &= ~WHITE_CASTLE_LONG;
		}
		else if (from == H1 && b->castleBits & WHITE_CASTLE_SHORT)
		{
			b->hash ^= Z_WHITE_SHORT_KEY;
			b->castleBits &= ~WHITE_CASTLE_SHORT;
		}
		else if(from == A8 && b->castleBits & BLACK_CASTLE_LONG)
		{
			b->hash ^= Z_BLACK_LONG_KEY;
			b->castleBits &= ~BLACK_CASTLE_LONG;
		}
		else if (from == H8 && b->castleBits & BLACK_CASTLE_SHORT)
		{
			b->hash ^= Z_BLACK_SHORT_KEY;
			b->castleBits &= ~BLACK_CASTLE_SHORT;
		}
	}

	// If a rook was captured, that affects castling too...
	if( retValue.captured == ROOK)
	{
		if( (to == A1) &&  (b->castleBits & WHITE_CASTLE_LONG) )
		{
			b->hash ^= Z_WHITE_LONG_KEY;
			b->castleBits &= ~WHITE_CASTLE_LONG;
		}
		else if( (to == H1) &&  (b->castleBits & WHITE_CASTLE_SHORT) )
		{
			b->hash ^= Z_WHITE_SHORT_KEY;
			b->castleBits &= ~WHITE_CASTLE_SHORT;
		}
		else if( (to == A8) &&  (b->castleBits & BLACK_CASTLE_LONG) )
		{
			b->hash ^= Z_BLACK_LONG_KEY;
			b->castleBits &= ~BLACK_CASTLE_LONG;
		}
		else if( (to == H8) &&  (b->castleBits & BLACK_CASTLE_SHORT) )
		{
			b->hash ^= Z_BLACK_SHORT_KEY;
			b->castleBits &= ~BLACK_CASTLE_SHORT;
		}
	}

	// Adjust halfmove count
	if( (pieceMoved == PAWN) || (retValue.captured != PIECE_NONE) )
	{
		b->halfMoves = 0;
	}
	else
	{
		b->halfMoves++;
	}

	// Adjust move number if black just moved
	if(toMove == BLACK)
	{
		b->moveNumber++;
	}

	// Adjust side to move
	b->toMove = oppositeColor;
	b->hash ^= Z_WHITE_TURN_KEY; // Either side to move will trigger this toggle.

	// Adjust enPassant col if we moved a pawn forward two

	if ( (pieceMoved == PAWN) && (abs(from - to) == 16) )
	{
		b->enPassantCol = to & 0x07;
		if( (shiftE(squareMask[to]) & b->pieces[PAWN] & b->colors[oppositeColor]) ||
		    (shiftW(squareMask[to]) & b->pieces[PAWN] & b->colors[oppositeColor]))
		{
		    b->zobristEnPassantCol = to & 0x07;
		}
		else
		{
		    b->zobristEnPassantCol = 8;
		}
	}
	else
	{
		b->enPassantCol = 8;
	    b->zobristEnPassantCol = 8;
	}

    if(retValue.priorZobristEnPassantCol != b->zobristEnPassantCol)
    {

        if(retValue.priorZobristEnPassantCol != 8)
	    {
	        b->hash  ^= Z_ENPASSANT_COL_KEY(retValue.priorZobristEnPassantCol);
	    }

        if( b->zobristEnPassantCol != 8)
        {
	        b->hash  ^= Z_ENPASSANT_COL_KEY(b->zobristEnPassantCol);
        }

    }

    ASSERT(positionIndex < POSITION_HISTORY_SIZE);
    ASSERT(positionIndex >= 0);


	positionHistory[positionIndex++] = b->hash;

	if(positionIndex == POSITION_HISTORY_SIZE)
	{
	    positionIndex = 0;
	}

	return retValue;
}

// Undo a previously made move.
void unmove(board_t *b, const revMove_t m)
{

	piece_t pieceMoved;

	int to = m.move.to;
	int from = m.move.from;

	int priorCastleBits = m.priorCastleBits;
	int priorEnPassant = m.priorEnPassant;
	int priorZobristEnPassant = m.priorZobristEnPassantCol;
	int priorHalfMoveCount = m.priorHalfMoveCnt;

	piece_t promote = (piece_t)(m.move.promote);
	piece_t captured = (piece_t)(m.captured);

	color_t oppositeColor = (b->toMove == WHITE ? BLACK : WHITE);



	// STEP 1: Remove the piece at the destination square, return to source square...

	// SPECIAL CASE: A pawn was promoted.  Remove the promoted piece....
	if(promote != PIECE_NONE)
	{
		removePiece(b, to, promote, oppositeColor);
		addPiece(b, from, PAWN, oppositeColor);
		pieceMoved = PAWN;
	}
	else
	{
		// Determine which piece is there....
		pieceMoved = ( (b->pieces[PAWN]   & squareMask[to]) ? PAWN : (
					   (b->pieces[KNIGHT] & squareMask[to]) ? KNIGHT : (
					   (b->pieces[BISHOP] & squareMask[to]) ? BISHOP : (
					   (b->pieces[ROOK]   & squareMask[to]) ? ROOK : (
					   (b->pieces[QUEEN]  & squareMask[to]) ? QUEEN : KING)))));
		movePiece(b, to, from, pieceMoved, oppositeColor);
	}

	// STEP 2:  Return captured piece if any

	// If a piece was captured last turn
	if(captured != PIECE_NONE)
	{
		int offset = 0;

		// If it was captured on the enPassant square, assess an offset to where the piece will be returned.
		if( (to % 8 == priorEnPassant) && ( (to / 8) == (b->toMove == BLACK ? 2 : 5) ) )
		{
			offset = (oppositeColor == BLACK ? -8 : 8);
		}
		addPiece(b, to + offset, captured, b->toMove);
	}

	// STEP 3: Undo rook moves from any castles...
   // TODO CHESS960

	if(pieceMoved == KING)
	{
		if(abs(to-from) == 2)
		{
			if(to == C1)
			{
				movePiece(b, D1, A1, ROOK, WHITE);
			}
			else if(to == G1)
			{
				movePiece(b, F1, H1, ROOK, WHITE);
			}
			else if(to == C8)
			{
				movePiece(b, D8, A8, ROOK, BLACK);
			}
			else if(to == G8)
			{
				movePiece(b, F8, H8, ROOK, BLACK);
			}
			else
			{
				ASSERT(0);
			}
		}
	}

  // Revert castling status if necessary

	if( (priorCastleBits & WHITE_CASTLE_SHORT) != (b->castleBits & WHITE_CASTLE_SHORT) )
	{
		b->hash ^= Z_WHITE_SHORT_KEY;
	}
	if( (priorCastleBits & WHITE_CASTLE_LONG) != (b->castleBits & WHITE_CASTLE_LONG) )
	{
		b->hash ^= Z_WHITE_LONG_KEY;
	}
	if( (priorCastleBits & BLACK_CASTLE_SHORT) != (b->castleBits & BLACK_CASTLE_SHORT) )
	{
		b->hash ^= Z_BLACK_SHORT_KEY;
	}
	if( (priorCastleBits & BLACK_CASTLE_LONG) != (b->castleBits & BLACK_CASTLE_LONG) )
	{
		b->hash ^= Z_BLACK_LONG_KEY;
	}

	b->castleBits = priorCastleBits;

  // Revert enPassant row if necessary

	// If current doesn't match prior
    if(priorZobristEnPassant != b->zobristEnPassantCol)
    {
        if(priorZobristEnPassant != 8)
	    {
	        b->hash  ^= Z_ENPASSANT_COL_KEY(priorZobristEnPassant);
	    }

        if( b->zobristEnPassantCol != 8)
        {
	        b->hash  ^= Z_ENPASSANT_COL_KEY(b->zobristEnPassantCol);
        }
    }

	b->zobristEnPassantCol = priorZobristEnPassant;
	b->enPassantCol = priorEnPassant;

	if(b->toMove == WHITE)
	{
		b->toMove = BLACK;
		b->moveNumber--;
	}
	else
	{
		b->toMove = WHITE;
	}
	b->hash ^= Z_WHITE_TURN_KEY; // Either side to move will trigger this toggle.

	b->halfMoves = priorHalfMoveCount;

    positionIndex--;

    if(positionIndex < 0)
    {
        positionIndex = POSITION_HISTORY_SIZE - 1;
    }

}

// For diagnostics... Shows ASCII representation of board

char *getFEN(const board_t *b)
{
    int offset = 0;
    int totalEmptySquares = 0;

    static char fenString[100];
    char tmpstr[10];


    fenString[0] = '\0';

    while(offset < 64)
    {
        if(squareMask[offset] & (b->colors[WHITE] | b->colors[BLACK]))
        {
            if(totalEmptySquares)
            {
                printf("%d", totalEmptySquares);
                totalEmptySquares = 0;
            }

            if     (squareMask[offset] & b->colors[WHITE] & b->pieces[PAWN])   strcat(fenString,"P");
            else if(squareMask[offset] & b->colors[WHITE] & b->pieces[KNIGHT]) strcat(fenString,"N");
            else if(squareMask[offset] & b->colors[WHITE] & b->pieces[BISHOP]) strcat(fenString,"B");
            else if(squareMask[offset] & b->colors[WHITE] & b->pieces[ROOK])   strcat(fenString,"R");
            else if(squareMask[offset] & b->colors[WHITE] & b->pieces[QUEEN])  strcat(fenString,"Q");
            else if(squareMask[offset] & b->colors[WHITE] & b->pieces[KING])   strcat(fenString,"K");
            else if(squareMask[offset] & b->colors[BLACK] & b->pieces[PAWN])   strcat(fenString,"p");
            else if(squareMask[offset] & b->colors[BLACK] & b->pieces[KNIGHT]) strcat(fenString,"n");
            else if(squareMask[offset] & b->colors[BLACK] & b->pieces[BISHOP]) strcat(fenString,"b");
            else if(squareMask[offset] & b->colors[BLACK] & b->pieces[ROOK])   strcat(fenString,"r");
            else if(squareMask[offset] & b->colors[BLACK] & b->pieces[QUEEN])  strcat(fenString,"q");
            else if(squareMask[offset] & b->colors[BLACK] & b->pieces[KING])   strcat(fenString,"k");
        }
        else
        {
            ++totalEmptySquares;
        }

        if((++offset % 8) == 0)
        {
            if(totalEmptySquares)
            {
                sprintf(tmpstr,"%d", totalEmptySquares);
                strcat(fenString, tmpstr);
                totalEmptySquares = 0;
            }
            if(offset < 64) strcat(fenString,"/");
            totalEmptySquares = 0;
        }
    }

    strcat(fenString," ");

    if(b->toMove == WHITE) strcat(fenString,"w "); else strcat(fenString,"b ");

    if(b->castleBits == 0)
    {
        strcat(fenString,"-");
    }
    else
    {
        // TODO CHESS960
        if(b->castleBits & WHITE_CASTLE_SHORT) strcat(fenString,"K");
        if(b->castleBits & WHITE_CASTLE_LONG)  strcat(fenString,"Q");
        if(b->castleBits & BLACK_CASTLE_SHORT) strcat(fenString,"k");
        if(b->castleBits & BLACK_CASTLE_LONG)  strcat(fenString,"q");
    }

    strcat(fenString," ");

    if(b->enPassantCol == 8)
    {
        strcat(fenString,"-");
    }
    else
    {
        sprintf(tmpstr,"%c", 'a' + b->enPassantCol);
        strcat(fenString, tmpstr);
        if(b->toMove == WHITE) strcat(fenString,"6"); else strcat(fenString,"3");
    }

    sprintf(tmpstr," %d %d", b->halfMoves, b->moveNumber);
    strcat(fenString, tmpstr);

    return fenString;

}

// ASSUMES piece placement routines set bitboard correctly
boardErr_t testValidBoard(board_t *b)
{

    int wKingOffset;
    int bKingOffset;
    bool_t inCheck;


  // LEVEL 0: Positions which make an illegal and unplayable board

    // Verify exactly one king per side...
    if(bitCount(b->colors[WHITE] & b->pieces[KING]) != 1) return ERR_BAD_WHITE_KING_COUNT;
    if(bitCount(b->colors[BLACK] & b->pieces[KING]) != 1) return ERR_BAD_BLACK_KING_COUNT;

    // Verify kings are not in contact with eachother
    wKingOffset = 63 - getLSBindex(b->colors[WHITE] & b->pieces[KING]);
    bKingOffset = 63 - getLSBindex(b->colors[BLACK] & b->pieces[KING]);
    if( (kingCoverage[wKingOffset] & squareMask[bKingOffset]) != 0) return ERR_OPPOSING_KINGS;

    // Verify side NOT to move is NOT check (i.e. king is not capturable)
    if(b->toMove == WHITE)
    {
        b->toMove = BLACK;
        inCheck = testInCheck(b);
        b->toMove = WHITE;
    }
    else
    {
        b->toMove = WHITE;
        inCheck = testInCheck(b);
        b->toMove = BLACK;
    }
    if(inCheck == TRUE) return ERR_OPP_ALREADY_IN_CHECK;

    // Verify no pawns on first or last rank
    if( (b->pieces[PAWN] & (rowMask[0] | rowMask[7]) ) != 0 ) return ERR_BAD_PAWN_POSITION;

  // LEVEL 1: Positions which create impossible combinations of pieces...

    // Verify max 16 pieces per side...
    if( bitCount(b->colors[WHITE]) > 16) return ERR_TOO_MANY_WHITE_PIECES;
    if( bitCount(b->colors[BLACK]) > 16) return ERR_TOO_MANY_BLACK_PIECES;

    // Verify piece mix for each side is obtainable through pawn promotions
    //      (1) Determine number of missing pawns for a given side
    //      (2) Verify "extra" pieces do not exceed (1)


  // LEVEL 2: Positions that are unreachable through normal play from starting position

    // Verify Doubled pawns accounted for by missings pieces (i.e. capture required to do this)

    return BRD_NO_ERROR;

}

bool_t testInCheck( board_t *b )
{
    color_t oppColor = (b->toMove == BLACK ? WHITE : BLACK);

	BB oppOrthogSliders = b->colors[oppColor]  & (b->pieces[QUEEN] | b->pieces[ROOK]  );
	BB oppDiagSliders   = b->colors[oppColor]  & (b->pieces[QUEEN] | b->pieces[BISHOP]);
	BB oppKnights       = b->colors[oppColor]  &  b->pieces[KNIGHT];
    BB oppPawns         = b->colors[oppColor]  &  b->pieces[PAWN];

    BB onMoveKing       = b->colors[b->toMove] &  b->pieces[KING];

	BB empty             = ~(b->colors[WHITE] | b->colors[BLACK]);


    BB oppOrthoAttacks = 0;
    BB oppDiagAttacks = 0;
    BB oppKnightAttacks = 0;
    BB oppPawnAttacks = 0;

	// Rooks and Queens
	if(oppOrthogSliders)
	{
		oppOrthoAttacks = Sattacks(oppOrthogSliders, empty) |
		                  Nattacks(oppOrthogSliders, empty) |
		                  Eattacks(oppOrthogSliders, empty) |
		                  Wattacks(oppOrthogSliders, empty);
	}

	// Bishops and Queens
	if(oppDiagSliders)
	{
		oppDiagAttacks  = SEattacks(oppDiagSliders, empty) |
						  SWattacks(oppDiagSliders, empty) |
						  NEattacks(oppDiagSliders, empty) |
						  NWattacks(oppDiagSliders, empty);
	}

	// Knights
	while(oppKnights)
	{
		oppKnightAttacks |= knightCoverage[63 - getLSBindex(oppKnights)];
		clearlsb(oppKnights);
	}

	// Pawns
	if(oppPawns)
	{
		if(b->toMove == WHITE)
		{
			oppPawnAttacks =  shiftSE(oppPawns) | shiftSW(oppPawns);
		}
		else
		{
			oppPawnAttacks = shiftNE(oppPawns) | shiftNW(oppPawns);
		}
	}

	// If any of these attacks reach our king, we are in check...
	if( onMoveKing & (oppOrthoAttacks | oppDiagAttacks | oppKnightAttacks | oppPawnAttacks) )
	{
		return TRUE;
	}
    else
    {
        return FALSE;
    }

}

// Only called during board setup or to add a new piece at pawn promotion...
void addPiece(board_t *b, U8 sq, piece_t p, color_t c)
{

	BB coordMask = squareMask[sq];

	// Make sure square is empty first...
 	ASSERT ( (coordMask & (b->colors[WHITE]  |
 	                       b->colors[BLACK]  |
 	                       b->pieces[PAWN]   |
 	                       b->pieces[KNIGHT] |
 	                       b->pieces[BISHOP] |
 	                       b->pieces[ROOK]   |
 	                       b->pieces[QUEEN]  |
 	                       b->pieces[KING]) )
 	            == 0 );

    b->pieces[p] |= coordMask;
	b->colors[c] |= coordMask;

	b->hash ^= Z_PIECESQUARE_KEY(p,c,sq);

	if(p != KING)
	{
		b->materialCount[c] += pieceValue[p];
	}

}

// Relocate a piece from one square to another
static void movePiece (board_t *b, U8 fromSq, U8 toSq, piece_t p, color_t c)
{

	// NOTE this replaces a call to remove/add and eliminates adjustment of material Count calculations.


	BB coordMask = squareMask[fromSq] | squareMask[toSq];

	ASSERT( ( squareMask[fromSq] &  b->pieces[p] & b->colors[c]) != 0);

	ASSERT( (squareMask[toSq]   & (b->colors[WHITE]  |
 	                               b->colors[BLACK]  |
 	                               b->pieces[PAWN]   |
 	                               b->pieces[KNIGHT] |
 	                               b->pieces[BISHOP] |
 	                               b->pieces[ROOK]   |
 	                               b->pieces[QUEEN]  |
 	                               b->pieces[KING]) )== 0 );


	b->pieces[p] ^= coordMask;
	b->colors[c] ^= coordMask;

	b->hash ^= Z_PIECESQUARE_KEY(p,c,toSq);
	b->hash ^= Z_PIECESQUARE_KEY(p,c,fromSq);


}

// Only called during captures OR to remove a pawn that gets promoted.
void removePiece(board_t *b, U8 sq, piece_t p, color_t c)
{

	BB coordMask = ~squareMask[sq];

	ASSERT( (~coordMask & b->pieces[p] & b->colors[c]) != 0);

	b->pieces[p] &= coordMask;
	b->colors[c] &= coordMask;

	b->hash ^= Z_PIECESQUARE_KEY(p,c,sq);

	if(p != KING)
	{
		b->materialCount[c] -= pieceValue[p];
	}

}
