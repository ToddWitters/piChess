#include "moves.h"
#include "bitboard.h"
#include "constants.h"
#include "zobrist.h"
#include "debug.h"
// #include "eval.h"
#include "board.h"

#include <string.h>
#include <stdlib.h>

#define MAX_LIST_SIZE 200

static int moveIndex;
static int insertIndex;

static void addMovePromote(int from, int to, move_t *moveList);
static void addMove(int from, int to, move_t *moveList);

// create list of legal moves
// returns number of moves found OR
// unique values for CHECKMATE and STALEMATE

int findMoves(board_t *brd, move_t *moveList)
{

	int onMoveKingOffset;

	moveIndex = 0;
	insertIndex = 0;

	board_t b;

	memcpy(&b, brd, sizeof(board_t));

	// Local quick reference to color of side to move.
	color_t onMoveColor = b.toMove;

	// Local quick reference to color of the opposite side
	color_t oppColor = (onMoveColor == WHITE ? BLACK : WHITE);

 	// True if side to move is in check
	bool_t inCheck = FALSE;

	// A handy bitboard of all empty squares
	BB empty= ~(b.colors[WHITE] | b.colors[BLACK]);

	// A scratch bitboard
	BB scratch;

	// location of rooks and queens
	BB onMoveOrthogSliders = b.colors[onMoveColor] & (scratch = (b.pieces[QUEEN] | b.pieces[ROOK]));
	BB oppOrthogSliders    = b.colors[oppColor]    & scratch;

	// location of the bishops and queens
	BB onMoveDiagSliders   = b.colors[onMoveColor] & (scratch = (b.pieces[QUEEN] | b.pieces[BISHOP]));
	BB oppDiagSliders      = b.colors[oppColor]    & scratch;

	// location of knights for side on move
	BB onMoveKnights = b.colors[onMoveColor] & b.pieces[KNIGHT];
	BB oppKnights    = b.colors[oppColor]    & b.pieces[KNIGHT];

	// Location of Pawns
	BB onMovePawns = b.colors[onMoveColor] & b.pieces[PAWN];
	BB oppPawns    = b.colors[oppColor]    & b.pieces[PAWN];

	// Location of Kings
	BB onMoveKing = b.colors[onMoveColor] & b.pieces[KING];
	BB oppKing    = b.colors[oppColor]    & b.pieces[KING];

	// Attack squares for opponent knights
	BB oppKnightAttacks = 0;

	// Attack squares for the kings
	BB onMoveKingAttacks = 0;
	BB oppKingAttacks = 0;

	// Attack squares for the pawns (i.e. the diags)
	BB oppPawnAttacks = 0;

	// All orthogonal attack squares
	BB oppOrthoAttacks = 0;

	// All diagonal attack squares
	BB oppDiagAttacks = 0;

	// All attack squares
	BB oppAttacks = 0;

    // onMove pieces between attacking sliders and king
	BB pinnedPieces = 0;

	// Pieces pinned in the 8 directions from the king...
	BB pinnedN = 0;
	BB pinnedS = 0;
	BB pinnedE = 0;
	BB pinnedW = 0;
	BB pinnedNE = 0;
	BB pinnedNW = 0;
	BB pinnedSE = 0;
	BB pinnedSW = 0;

	BB slidersInPath;
	BB blockersInPath;
	int blockerOffset;
	int sliderOffset;

	// SANITY CHECKS

	{
	// Check for no overlap between WHITE/BLACK bitboards
	ASSERT( !(b.colors[WHITE] & b.colors[BLACK]) );


	// Check for no overlap between pieces bitboards
	scratch = 0;
	ASSERT( ( ( scratch |= b.pieces[PAWN] )   & b.pieces[KNIGHT] ) == 0);
	ASSERT( ( ( scratch |= b.pieces[KNIGHT] ) & b.pieces[BISHOP] ) == 0);
	ASSERT( ( ( scratch |= b.pieces[BISHOP])  & b.pieces[ROOK]   ) == 0);
	ASSERT( ( ( scratch |= b.pieces[ROOK])    & b.pieces[QUEEN]  ) == 0);
	ASSERT( ( ( scratch |= b.pieces[QUEEN])   & b.pieces[KING]   ) == 0);

	// Check for equality between color space and piece space
	ASSERT(  (b.pieces[PAWN]  |
	          b.pieces[KNIGHT] |
	          b.pieces[BISHOP] |
	          b.pieces[ROOK] |
	          b.pieces[QUEEN] |
	          b.pieces[KING]) ==
	        ( b.colors[WHITE] |
	          b.colors[BLACK] ) );

	// Test for one king of each color
	ASSERT( bitCount( b.pieces[KING] & b.colors[BLACK] ) == 1);
	ASSERT( bitCount( b.pieces[KING] & b.colors[WHITE] ) == 1);
    }


	// FIRST DETERMINE IF WE ARE IN CHECK
	// Move generation is more efficient if this can be established first...

	// Attacks from Opponent

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
	scratch = oppKnights;
	while(scratch)
	{
		oppKnightAttacks |= knightCoverage[63 - getLSBindex(scratch)];
		clearlsb(scratch);
	}

	// Pawns
	oppPawns = b.colors[oppColor] & b.pieces[PAWN];

	if(oppPawns)
	{
		if(onMoveColor == WHITE)
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
		inCheck = TRUE;
	}

	// Determine squares that opponent's king can attack (or defend)
	oppKingAttacks = kingCoverage[63-getLSBindex(oppKing)];

	// Make sure kings are not in opposition
	ASSERT((oppKingAttacks & (b.pieces[KING] & b.pieces[onMoveColor])) == 0);

	// Determine all squares which opponent can attack....
	oppAttacks = oppOrthoAttacks | oppDiagAttacks | oppKnightAttacks | oppKingAttacks | oppPawnAttacks;

	////
	// GENERATE MOVE LIST
	////

	// First, find any pieces that are pinned.  This accelerates the move generation logic...

	// find pinned pieces in all 8 directions

	// For each of the following 8 code snippets, the logic is:
	//  if(moving a given direction from king discovers a friendly piece)
	//		if(moving further in that same direction finds an attacking slider)
	//           mark the friendly piece as pinned.

	// Calculate this for later...
	onMoveKingAttacks = kingCoverage[(onMoveKingOffset = 63-getLSBindex(onMoveKing))];

	// CHECK FOR PINNED PIECES.

	// NORTH

	// Is there an enemy orthogonal slider somewhere in this direction?
	if( (slidersInPath = (ray[NORTH][onMoveKingOffset] & oppOrthogSliders) ) )
	{
		// Are there any blocking pieces somewhere in this direction (friendly OR oppononent non-orthog-sliders)
		if( (blockersInPath = (ray[NORTH][onMoveKingOffset] & (~oppOrthogSliders & ~empty) ) ) )
		{
			// Is the closest blocker closer than the closest slider?
			if( (blockerOffset = (63-getLSBindex(blockersInPath))) > ( sliderOffset = (63-getLSBindex(slidersInPath) ) ) )
			{
				// Is it a friendly piece?
				if(squareMask[blockerOffset] & b.colors[onMoveColor])
				{
					// Remove it, and see what is now closest to king
					clearlsb(blockersInPath);

					// Are there any more blockers in the path?
					if(blockersInPath)
					{
						// Yes... Is the closest slider closer than the closest blocker?
						if( sliderOffset > (63-getLSBindex(blockersInPath) ) )
						{
							// Yes - the removed blocker was therefore pinned.
							pinnedPieces |= (pinnedN = squareMask[blockerOffset]);
						}
					}
					else // No more blockers
					{
						// we are exposed to enemy sliders.  The removed blocker was therefore pinned....
						pinnedPieces |= (pinnedN = squareMask[blockerOffset]);
					}
				}
			}
		}
	}

	// SOUTH

	// Is there an enemy orthogonal slider somewhere in this direction?
	if( (slidersInPath = (ray[SOUTH][onMoveKingOffset] & oppOrthogSliders) ) )
	{
		// Are there any blocking pieces somewhere in this direction (friendly OR oppononent non-orthog-sliders)
		if( (blockersInPath = (ray[SOUTH][onMoveKingOffset] & (~oppOrthogSliders & ~empty) ) ) )
		{
			// Is the closest blocker closer than the closest slider?
			if( (blockerOffset = (63-getMSBindex(blockersInPath))) < ( sliderOffset = (63-getMSBindex(slidersInPath) ) ) )
			{
				BB tempBB;
				// Is it a friendly piece?
				if((tempBB = squareMask[blockerOffset]) & b.colors[onMoveColor])
				{
					// Remove it, and see what is now closest to king
					blockersInPath &= ~tempBB;

					// Are there any more blockers in the path?
					if(blockersInPath)
					{
						// Yes... Is the closest slider closer than the closest blocker?
						if( sliderOffset < (63-getMSBindex(blockersInPath) ) )
						{
							// Yes - the removed blocker was therefore pinned.
							pinnedPieces |= (pinnedS = squareMask[blockerOffset]);
						}
					}
					else // No more blockers
					{
						// we are exposed to enemy sliders.  The removed blocker was therefore pinned....
						pinnedPieces |= (pinnedS = squareMask[blockerOffset]);
					}
				}
			}
		}
	}

	// EAST

	// Is there an enemy orthogonal slider somewhere in this direction?
	if( (slidersInPath = (ray[EAST][onMoveKingOffset] & oppOrthogSliders) ) )
	{
		// Are there any blocking pieces somewhere in this direction (friendly OR oppononent non-orthog-sliders)
		if( (blockersInPath = (ray[EAST][onMoveKingOffset] & (~oppOrthogSliders & ~empty) ) ) )
		{
			// Is the closest blocker closer than the closest slider?
			if( (blockerOffset = (63-getMSBindex(blockersInPath))) < ( sliderOffset = (63-getMSBindex(slidersInPath) ) ) )
			{
				BB tempBB;
				// Is it a friendly piece?
				if((tempBB = squareMask[blockerOffset]) & b.colors[onMoveColor])
				{
					// Remove it, and see what is now closest to king
					blockersInPath &= ~tempBB;

					// Are there any more blockers in the path?
					if(blockersInPath)
					{
						// Yes... Is the closest slider closer than the closest blocker?
						if( sliderOffset < (63-getMSBindex(blockersInPath) ) )
						{
							// Yes - the removed blocker was therefore pinned.
							pinnedPieces |= (pinnedE = squareMask[blockerOffset]);
						}
					}
					else // No more blockers
					{
						// we are exposed to enemy sliders.  The removed blocker was therefore pinned....
						pinnedPieces |= (pinnedE = squareMask[blockerOffset]);
					}
				}
			}
		}
	}

	// WEST

	// Is there an enemy orthogonal slider somewhere in this direction?
	if( (slidersInPath = (ray[WEST][onMoveKingOffset] & oppOrthogSliders) ) )
	{
		// Are there any blocking pieces somewhere in this direction (friendly OR oppononent non-orthog-sliders)
		if( (blockersInPath = (ray[WEST][onMoveKingOffset] & (~oppOrthogSliders & ~empty) ) ) )
		{
			// Is the closest blocker closer than the closest slider?
			if( (blockerOffset = (63-getLSBindex(blockersInPath))) > ( sliderOffset = (63-getLSBindex(slidersInPath) ) ) )
			{
				// Is it a friendly piece?
				if(squareMask[blockerOffset] & b.colors[onMoveColor])
				{
					// Remove it, and see what is now closest to king
					clearlsb(blockersInPath);

					// Are there any more blockers in the path?
					if(blockersInPath)
					{
						// Yes... Is the closest slider closer than the closest blocker?
						if( sliderOffset > (63-getLSBindex(blockersInPath) ) )
						{
							// Yes - the removed blocker was therefore pinned.
							pinnedPieces |= (pinnedW = squareMask[blockerOffset]);
						}
					}
					else // No more blockers
					{
						// we are exposed to enemy sliders.  The removed blocker was therefore pinned....
						pinnedPieces |= (pinnedW = squareMask[blockerOffset]);
					}
				}
			}
		}
	}

	// NORTHEAST

	// Is there an enemy diag slider somewhere in this direction?
	if( (slidersInPath = (ray[NORTHEAST][onMoveKingOffset] & oppDiagSliders) ) )
	{
		// Are there any blocking pieces somewhere in this direction (friendly OR oppononent non-diag-sliders)
		if( (blockersInPath = (ray[NORTHEAST][onMoveKingOffset] & (~oppDiagSliders & ~empty) ) ) )
		{
			// Is the closest blocker closer than the closest slider?
			if( (blockerOffset = (63-getLSBindex(blockersInPath))) > ( sliderOffset = (63-getLSBindex(slidersInPath) ) ) )
			{
				// Is it a friendly piece?
				if(squareMask[blockerOffset] & b.colors[onMoveColor])
				{
					// Remove it, and see what is now closest to king
					clearlsb(blockersInPath);

					// Are there any more blockers in the path?
					if(blockersInPath)
					{
						// Yes... Is the closest slider closer than the closest blocker?
						if( sliderOffset > (63-getLSBindex(blockersInPath) ) )
						{
							// Yes - the removed blocker was therefore pinned.
							pinnedPieces |= (pinnedNE = squareMask[blockerOffset]);
						}
					}
					else // No more blockers
					{
						// we are exposed to enemy sliders.  The removed blocker was therefore pinned....
						pinnedPieces |= (pinnedNE = squareMask[blockerOffset]);
					}
				}
			}
		}
	}

	// NORTHWEST

	// Is there an enemy diag slider somewhere in this direction?
	if( (slidersInPath = (ray[NORTHWEST][onMoveKingOffset] & oppDiagSliders) ) )
	{
		// Are there any blocking pieces somewhere in this direction (friendly OR oppononent non-diag-sliders)
		if( (blockersInPath = (ray[NORTHWEST][onMoveKingOffset] & (~oppDiagSliders & ~empty) ) ) )
		{
			// Is the closest blocker closer than the closest slider?
			if( (blockerOffset = (63-getLSBindex(blockersInPath))) > ( sliderOffset = (63-getLSBindex(slidersInPath) ) ) )
			{
				// Is it a friendly piece?
				if(squareMask[blockerOffset] & b.colors[onMoveColor])
				{
					// Remove it, and see what is now closest to king
					clearlsb(blockersInPath);

					// Are there any more blockers in the path?
					if(blockersInPath)
					{
						// Yes... Is the closest slider closer than the closest blocker?
						if( sliderOffset > (63-getLSBindex(blockersInPath) ) )
						{
							// Yes - the removed blocker was therefore pinned.
							pinnedPieces |= (pinnedNW = squareMask[blockerOffset]);
						}
					}
					else // No more blockers
					{
						// we are exposed to enemy sliders.  The removed blocker was therefore pinned....
						pinnedPieces |= (pinnedNW = squareMask[blockerOffset]);
					}
				}
			}
		}
	}

	// SOUTHEAST

	// Is there an enemy diag slider somewhere in this direction?
	if( (slidersInPath = (ray[SOUTHEAST][onMoveKingOffset] & oppDiagSliders) ) )
	{
		// Are there any blocking pieces somewhere in this direction (friendly OR oppononent non-diag-sliders)
		if( (blockersInPath = (ray[SOUTHEAST][onMoveKingOffset] & (~oppDiagSliders & ~empty) ) ) )
		{
			// Is the closest blocker closer than the closest slider?
			if( (blockerOffset = (63-getMSBindex(blockersInPath))) < ( sliderOffset = (63-getMSBindex(slidersInPath) ) ) )
			{
				BB tempBB;
				// Is it a friendly piece?
				if((tempBB = squareMask[blockerOffset]) & b.colors[onMoveColor])
				{
					// Remove it, and see what is now closest to king
					blockersInPath &= ~tempBB;

					// Are there any more blockers in the path?
					if(blockersInPath)
					{
						// Yes... Is the closest slider closer than the closest blocker?
						if( sliderOffset < (63-getMSBindex(blockersInPath) ) )
						{
							// Yes - the removed blocker was therefore pinned.
							pinnedPieces |= (pinnedSE = squareMask[blockerOffset]);
						}
					}
					else // No more blockers
					{
						// we are exposed to enemy sliders.  The removed blocker was therefore pinned....
						pinnedPieces |= (pinnedSE = squareMask[blockerOffset]);
					}
				}
			}
		}
	}

	// SOUTHWEST

	// Is there an enemy diag slider somewhere in this direction?
	if( (slidersInPath = (ray[SOUTHWEST][onMoveKingOffset] & oppDiagSliders) ) )
	{
		// Are there any blocking pieces somewhere in this direction (friendly OR oppononent non-diag-sliders)
		if( (blockersInPath = (ray[SOUTHWEST][onMoveKingOffset] & (~oppDiagSliders & ~empty) ) ) )
		{
			// Is the closest blocker closer than the closest slider?
			if( (blockerOffset = (63-getMSBindex(blockersInPath))) < ( sliderOffset = (63-getMSBindex(slidersInPath) ) ) )
			{
				BB tempBB;
				// Is it a friendly piece?
				if((tempBB = squareMask[blockerOffset]) & b.colors[onMoveColor])
				{
					// Remove it, and see what is now closest to king
					blockersInPath &= ~tempBB;

					// Are there any more blockers in the path?
					if(blockersInPath)
					{
						// Yes... Is the closest slider closer than the closest blocker?
						if( sliderOffset < (63-getMSBindex(blockersInPath) ) )
						{
							// Yes - the removed blocker was therefore pinned.
							pinnedPieces |= (pinnedSW = squareMask[blockerOffset]);
						}
					}
					else // No more blockers
					{
						// we are exposed to enemy sliders.  The removed blocker was therefore pinned....
						pinnedPieces |= (pinnedSW = squareMask[blockerOffset]);
					}
				}
			}
		}
	}


	// Move generation logic is simplified if we first know whether we are in check or not....
	if(inCheck == FALSE)
	{

		//////////////////////
		// FIND ALL KING MOVES
		//////////////////////

		// All squares king can attack/defend
		scratch = onMoveKingAttacks;

		// MINUS friendly pieces
		scratch &= ~b.colors[onMoveColor];

		// MINUS squares under attack by opponent
		scratch &= ~oppAttacks;

		// For each king move
		while(scratch)
		{
			int targetOffset = 63-getLSBindex(scratch);

		   addMove(onMoveKingOffset, targetOffset, moveList);

 			clearlsb(scratch);
		}

      // TODO CHESS960

		// CAN WE CASTLE?
		if(onMoveColor == WHITE)
		{
			if(onMoveKingOffset == E1)
			{
				if( (b.castleBits & WHITE_CASTLE_SHORT) && (((f1|g1) & empty) == (f1|g1)))
				{
					if( ( (f1|g1) & oppAttacks) == 0)
					{
						addMove(E1, G1, moveList);
					}
				}
				if( (b.castleBits & WHITE_CASTLE_LONG)  && (((b1|c1|d1) & empty) == (b1|c1|d1)))
				{
					if( ( (c1|d1) & oppAttacks) == 0)
					{
						addMove(E1, C1, moveList);
					}
				}
			}
		}
		else
		{
			if(onMoveKingOffset == E8)
			{
				if( (b.castleBits & BLACK_CASTLE_SHORT) && (((f8|g8) & empty) == (f8|g8)))
				{
					if( ( (f8|g8) & oppAttacks) == 0)
					{
						addMove(E8, G8, moveList);
					}
				}
				if( (b.castleBits & BLACK_CASTLE_LONG) && (((b8|c8|d8) & empty) == (b8|c8|d8)))
				{
					if( ( (c8|d8) & oppAttacks) == 0)
					{
						addMove(E8, C8, moveList);
					}
				}
			}
		}

		//////////////////////
		// FIND ALL PAWN MOVES
		//////////////////////

		// Create scratch bitboard of all onMove pawns.
		scratch = onMovePawns;

		// While there are more pawns to evaluate...
		while(scratch)
		{

			int offset;

			// Create bitboard of a single pawn
			BB thisPawn = squareMask[(offset = 63-getLSBindex(scratch))];

			// Direction to king for pinned pawns (defaults DIR_NONE, meaning piece is not pinned)
			//  This default will apply to any NON-PINNED pawn...
			dir_t dir = DIR_NONE;

			// Make sure pawn is NOT on first or last rank.
			ASSERT( (thisPawn & (rowMask[0] | rowMask[7] ) ) == 0);

			// Is this pawn pinned?
			if(thisPawn & pinnedPieces)
			{
				// Find direction to king.
				// Order of search is different based upon most likely pinned direction...
				if(onMoveColor == WHITE)
				{
					// Search most likely directions first....
					if     ( pinnedS  & thisPawn) dir = SOUTH;
					else if( pinnedSE & thisPawn) dir = SOUTHEAST;
					else if( pinnedSW & thisPawn) dir = SOUTHWEST;
					else if( pinnedW  & thisPawn) dir = WEST;
					else if( pinnedE  & thisPawn) dir = EAST;
					else if( pinnedNE & thisPawn) dir = NORTHEAST;
					else if( pinnedNW & thisPawn) dir = NORTHWEST;
					else if( pinnedN  & thisPawn) dir = NORTH;
					else ASSERT(0);
				}
				else
				{
					if     ( pinnedN  & thisPawn) dir = NORTH;
					else if( pinnedNE  & thisPawn) dir = NORTHEAST;
					else if( pinnedNW  & thisPawn) dir = NORTHWEST;
					else if( pinnedW  & thisPawn) dir = WEST;
					else if( pinnedE  & thisPawn) dir = EAST;
					else if( pinnedSE  & thisPawn) dir = SOUTHEAST;
					else if( pinnedSW & thisPawn) dir = SOUTHWEST;
					else if( pinnedS  & thisPawn) dir = SOUTH;
					else ASSERT(0);
				}
			}

			// Duplicate sections for BLACK / WHITE used to optimize for speed (at expense of maintenance)
			if(onMoveColor == WHITE)
			{
				BB oneShift;
				// If pawn can move forward
				if( (oneShift = shiftN(thisPawn)) & empty )
				{
					// If pawn is not pinned to king OR its pinned from NORTH or SOUTH...
					if(dir == DIR_NONE || dir == NORTH || dir == SOUTH)
					{
						// Will this promote?
						if(thisPawn & rowMask[1])
						{
							addMovePromote(offset, offset-8, moveList);
						}
						else
						{
							addMove(offset, offset-8, moveList);

							// Check if on original square AND a 2nd push still possible...
							if( (thisPawn & rowMask[6]) && (shiftN(oneShift) & empty) )
							{
								addMove(offset, offset-16, moveList);
							}
						}
					}
				}

				// If last move for opp was a double pawn push
				if( b.enPassantCol	!= 8)
				{
					// If thisPawn is on Row 3 (5 on chessboard)
					if(offset / 8 == 3)
					{
						// If enPassant COl is one towards East
						if( (offset%8 + 1) == b.enPassantCol )
						{
							// If pawn is not pinned OR is pinned NE/SW
							if (dir == DIR_NONE || dir == NORTHEAST || dir == SOUTHWEST)
							{
								//  Test to ensure that removal of both pawns from the given row will NOT
								//  open up a E/W (or W/E) attack on king...

								BB temp = Wattacks(thisPawn,empty) | Eattacks(shiftE(thisPawn),empty);

								if(! ( (temp & onMoveKing) &&  (temp & oppOrthogSliders) ) )
								{
									addMove(offset, offset-7, moveList);
								}
							}
						}
						else if( (offset%8 - 1) == b.enPassantCol )
						{
							// If pawn is not pinned OR is pinned NW/SE
							if (dir == DIR_NONE || dir == NORTHWEST || dir == SOUTHEAST)
							{
								BB temp = Eattacks(thisPawn,empty) | Wattacks(shiftW(thisPawn),empty);

								if(! ( (temp & onMoveKing) &&  (temp & oppOrthogSliders) ) )
								{
									addMove(offset, offset-9, moveList);
								}
							}
						}
					}
				}
				// Check for NE capture
				if( shiftNE(thisPawn) & b.colors[BLACK] )
				{
					if(dir == DIR_NONE || dir == NORTHEAST || dir == SOUTHWEST)
					{
						// Will this promote?
						if(thisPawn & rowMask[1])
						{
							addMovePromote(offset, offset-7, moveList);
						}
						else
						{
							addMove(offset, offset-7, moveList);
						}
					}
				}

				// Check for NW capture
				if( shiftNW(thisPawn) & b.colors[BLACK] )
				{
					if(dir == DIR_NONE || dir == NORTHWEST || dir == SOUTHEAST)
					{
						// Will this promote?
						if(thisPawn & rowMask[1])
						{
							addMovePromote(offset, offset-9, moveList);
						}
						else
						{
							addMove(offset, offset-9, moveList);
						}
					}
				}

			}
			else
			{
				BB oneShift;
				// If pawn can move forward
				if( (oneShift = shiftS(thisPawn)) & empty )
				{
					// If pawn is not pinned to king OR its pinned from NORTH or SOUTH...
					if(dir == DIR_NONE || dir == NORTH || dir == SOUTH)
					{
						// Will this promote?
						if(thisPawn & rowMask[6])
						{
							addMovePromote(offset, offset+8, moveList);
						}
						else
						{
							addMove(offset, offset+8, moveList);
							if( (thisPawn & rowMask[1]) && (shiftS(oneShift) & empty) )
							{
								addMove(offset, offset+16, moveList);
							}
						}
					}
				}
				// If last move for opp was a double pawn push
				if( b.enPassantCol	!= 8)
				{
					// If thisPawn is on Row 4 (also 4 on chessboard)
					if(offset / 8 == 4)
					{
						// If enPassant COl is one towards East
						if( (offset%8 + 1) == b.enPassantCol )
						{
							// If pawn is not pinned OR is pinned NW/SE
							if (dir == DIR_NONE || dir == NORTHWEST || dir == SOUTHEAST)
							{
								BB temp = Wattacks(thisPawn,empty) | Eattacks(shiftE(thisPawn),empty);

								if(! ( (temp & onMoveKing) &&  (temp & oppOrthogSliders) ) )
								{
									addMove(offset, offset+9, moveList);
								}
							}
						}
						else if( (offset%8 - 1) == b.enPassantCol )
						{
							// If pawn is not pinned OR is pinned NW/SE
							if (dir == DIR_NONE || dir == NORTHEAST || dir == SOUTHWEST)
							{
								BB temp = Eattacks(thisPawn,empty) | Wattacks(shiftW(thisPawn),empty);

								if(! ( (temp & onMoveKing) &&  (temp & oppOrthogSliders) ) )
								{
									addMove(offset, offset+7, moveList);
								}
							}
						}
					}
				}

				if( shiftSE(thisPawn) & b.colors[WHITE] )
				{
					if(dir == DIR_NONE || dir == NORTHWEST || dir == SOUTHEAST)
					{
						// Will this promote?
						if(thisPawn & rowMask[6])
						{
							addMovePromote(offset, offset+9, moveList);
						}
						else
						{
							addMove(offset, offset+9, moveList);
						}
					}
				}
				if( shiftSW(thisPawn) & b.colors[WHITE] )
				{
					if(dir == DIR_NONE || dir == NORTHEAST || dir == SOUTHWEST)
					{
						// Will this promote?
						if(thisPawn & rowMask[6])
						{
							addMovePromote(offset, offset+7, moveList);
						}
						else
						{
							addMove(offset, offset+7, moveList);
						}
					}
				}


			}
			clearlsb(scratch);
		}

		//////////////////////
		// FIND ALL KNIGHT MOVES
		//////////////////////

		scratch = onMoveKnights;

		while(scratch)
		{
			int offset;

			// Create bitboard of a single knight
			BB thisKnight = squareMask[(offset = 63-getLSBindex(scratch))];

			// Is this knight pinned?
			if( (thisKnight & pinnedPieces) == 0)
			{
				BB knightTargets = knightCoverage[offset] & ~(b.colors[onMoveColor]);
				while(knightTargets)
				{
					U64 temp = 63-getLSBindex(knightTargets);

   				addMove(offset, temp, moveList);

					clearlsb(knightTargets);
				}
			}
			clearlsb(scratch);
		}


		//////////////////////
		// FIND ALL ORTHOGONAL SLIDER MOVES
		//////////////////////

		scratch = onMoveOrthogSliders;

		while(scratch)
		{
			int offset;

			dir_t dir = DIR_NONE;

			// Create bitboard of single ortho slider
			BB thisSlider = squareMask[(offset = 63-getLSBindex(scratch))];

			// If this slider in pinned, determine direction to king...
			if(thisSlider & pinnedPieces)
			{
				if     ( pinnedN  & thisSlider) dir = NORTH;
				else if( pinnedNE & thisSlider) dir = NORTHEAST;
				else if( pinnedNW & thisSlider) dir = NORTHWEST;
				else if( pinnedW  & thisSlider) dir = WEST;
				else if( pinnedE  & thisSlider) dir = EAST;
				else if( pinnedSE & thisSlider) dir = SOUTHEAST;
				else if( pinnedSW & thisSlider) dir = SOUTHWEST;
				else if( pinnedS  & thisSlider) dir = SOUTH;
				else ASSERT(0);
			}

			if(dir == DIR_NONE || dir == NORTH || dir == SOUTH)
			{
				BB attacks;

				attacks =  Nattacks(thisSlider, empty);
				attacks |= Sattacks(thisSlider, empty);
				attacks &= ~b.colors[onMoveColor];

				while(attacks)
				{
					int targetOffset = 63-getLSBindex(attacks);

               addMove(offset, targetOffset, moveList);

               clearlsb(attacks);
				}
			}

			if(dir == DIR_NONE || dir == EAST || dir == WEST)
			{
				BB attacks;

				attacks =  Wattacks(thisSlider, empty);
				attacks |= Eattacks(thisSlider, empty);
				attacks &= ~b.colors[onMoveColor];

				while(attacks)
				{
					int targetOffset = 63-getLSBindex(attacks);
  					addMove(offset, targetOffset, moveList);
					clearlsb(attacks);
				}
			}
			clearlsb(scratch);
		}

		//////////////////////
		// FIND ALL DIAGONAL SLIDER MOVES
		//////////////////////

		scratch = onMoveDiagSliders;

		while(scratch)
		{
			int offset;

			dir_t dir = DIR_NONE;

			// Create bitboard of single diag slider
			BB thisSlider = squareMask[(offset = 63-getLSBindex(scratch))];

			// If this slider in pinned, determine direction to king...
			if(thisSlider & pinnedPieces)
			{
				if     ( pinnedN  & thisSlider) dir = NORTH;
				else if( pinnedNE & thisSlider) dir = NORTHEAST;
				else if( pinnedNW & thisSlider) dir = NORTHWEST;
				else if( pinnedW  & thisSlider) dir = WEST;
				else if( pinnedE  & thisSlider) dir = EAST;
				else if( pinnedSE & thisSlider) dir = SOUTHEAST;
				else if( pinnedSW & thisSlider) dir = SOUTHWEST;
				else if( pinnedS  & thisSlider) dir = SOUTH;
				else ASSERT(0);
			}

			if(dir == DIR_NONE || dir == NORTHEAST || dir == SOUTHWEST)
			{
				BB attacks;

				attacks =  NEattacks(thisSlider, empty);
				attacks |= SWattacks(thisSlider, empty);
				attacks &= ~b.colors[onMoveColor];

				while(attacks)
				{
					int targetOffset = 63-getLSBindex(attacks);
					addMove(offset, targetOffset, moveList);
					clearlsb(attacks);
				}
			}

			if(dir == DIR_NONE || dir == NORTHWEST || dir == SOUTHEAST)
			{
				BB attacks;

				attacks =  NWattacks(thisSlider, empty);
				attacks |= SEattacks(thisSlider, empty);
				attacks &= ~b.colors[onMoveColor];

				while(attacks)
				{
					int targetOffset = 63-getLSBindex(attacks);
  					addMove(offset, targetOffset, moveList);
					clearlsb(attacks);
				}
			}
			clearlsb(scratch);
		}
	}
	else // WE ARE IN CHECK
	{

		BB kingAttackers = 0;

		// Holds attackers in individual directions for later reference....
		BB Nattackers;
		BB Sattackers;
		BB Eattackers;
		BB Wattackers;
		BB NEattackers;
		BB NWattackers;
		BB SEattackers;
		BB SWattackers;

		// determine number of king Attackers....

		//KNIGHTS
		kingAttackers |= (knightCoverage[onMoveKingOffset] & b.colors[oppColor] & b.pieces[KNIGHT] );

		// ROOKS, QUEENS
		kingAttackers |=  ( (Nattackers = Nattacks(onMoveKing, empty) ) |
		                    (Sattackers = Sattacks(onMoveKing, empty) ) |
		                    (Eattackers = Eattacks(onMoveKing, empty) ) |
		                    (Wattackers = Wattacks(onMoveKing, empty) ) ) & oppOrthogSliders;

		// BISHOPS, QUEENS
		kingAttackers |=  ( (NEattackers = NEattacks(onMoveKing, empty) ) |
						    (NWattackers = NWattacks(onMoveKing, empty) ) |
		                    (SEattackers = SEattacks(onMoveKing, empty) ) |
		                    (SWattackers = SWattacks(onMoveKing, empty) ) ) & oppDiagSliders;

		// PAWNS
		if(b.toMove == WHITE)
		{
			kingAttackers |= (shiftNE(onMoveKing) | shiftNW(onMoveKing) ) & oppPawns;
		}
		else
		{
			kingAttackers |= (shiftSE(onMoveKing) | shiftSW(onMoveKing) ) & oppPawns;
		}

		// No legal board positions can have more than 2 pieces delivering check to the king
		ASSERT(bitCount(kingAttackers) < 3);

		// We should be in check, so attack count can't be zero...
		ASSERT(bitCount(kingAttackers) > 0);

		//////////////////////
		// FIND ALL KING MOVES
		//////////////////////

		// Get list of king's attacking squares
		scratch = onMoveKingAttacks;

		// Remove squares with friendly pieces
		scratch &= ~b.colors[onMoveColor];

		// Remove possible king moves squares that would be screened from sliding oppAttack logic by the king itself..
		if( (Nattackers & oppOrthogSliders) ) scratch &= ~(shiftS(onMoveKing));
		if( (Sattackers & oppOrthogSliders) ) scratch &= ~(shiftN(onMoveKing));
		if( (Eattackers & oppOrthogSliders) ) scratch &= ~(shiftW(onMoveKing));
		if( (Wattackers & oppOrthogSliders) ) scratch &= ~(shiftE(onMoveKing));
		if( (NEattackers & oppDiagSliders) ) scratch &= ~(shiftSW(onMoveKing));
		if( (NWattackers & oppDiagSliders) ) scratch &= ~(shiftSE(onMoveKing));
		if( (SEattackers & oppDiagSliders) ) scratch &= ~(shiftNW(onMoveKing));
		if( (SWattackers & oppDiagSliders) ) scratch &= ~(shiftNE(onMoveKing));

		// Remove squares attacked by opponent
		scratch &= ~oppAttacks;

		while(scratch)
		{

			int targetOffset = 63-getLSBindex(scratch);

			addMove(onMoveKingOffset, targetOffset, moveList);

			clearlsb(scratch);
		}

		// If there is a single attacker (if not, only king moves, which were already generated, are legal)
		if(bitCount(kingAttackers) == 1)
		{

		 	BB interposers;

			// (1) CAN WE CAPTURE THIS ATTACKER?

			// find all non-pinned, friendly pieces that can capture the attacker
			BB attackerAttackers = 0;

			int offset = 63-getLSBindex(kingAttackers);

			// determine number of king Attacker Attackers....
			attackerAttackers |= (knightCoverage[offset] & b.colors[b.toMove] & b.pieces[KNIGHT] );

			attackerAttackers |=  ( Nattacks(kingAttackers, empty) |
								Sattacks(kingAttackers, empty) |
								Eattacks(kingAttackers, empty) |
								Wattacks(kingAttackers, empty) ) & onMoveOrthogSliders;

			attackerAttackers |=  ( NEattacks(kingAttackers, empty) |
								NWattacks(kingAttackers, empty) |
								SEattacks(kingAttackers, empty) |
								SWattacks(kingAttackers, empty) ) & onMoveDiagSliders;
			if(b.toMove == WHITE)
			{
				attackerAttackers |= (shiftSE(kingAttackers) | shiftSW(kingAttackers) ) & onMovePawns;
			}
			else
			{
				attackerAttackers |= (shiftNE(kingAttackers) | shiftNW(kingAttackers) ) & onMovePawns;
			}

			// Eliminate all those who are pinned...
			attackerAttackers &= ~pinnedPieces;

			// For each one found...
			while(attackerAttackers)
			{
				int defenderOffset = 63-getLSBindex(attackerAttackers);

				if( (b.toMove == WHITE) && (squareMask[defenderOffset] & rowMask[1] & b.pieces[PAWN]) )
				{
					addMovePromote(defenderOffset, offset, moveList);
				}
				else if ( (b.toMove == BLACK) && (squareMask[defenderOffset] & rowMask[6] & b.pieces[PAWN]) )
				{
					addMovePromote(defenderOffset, offset, moveList);
				}
				else
				{
					addMove(defenderOffset, offset, moveList);
				}
				clearlsb(attackerAttackers);
			}

			// Special case: Can attacker be remove enPassant?

			// A pawn just pushed two forward...
			if(b.enPassantCol != 8)
			{
				// If pawn is the sole attacker of the king
				if(oppPawns & kingAttackers)
				{
					// if white to move
					if(b.toMove == WHITE)
					{
						// If attacker is to the NE
						if( shiftNE(onMoveKing) & oppPawns )
						{
							// If we have a non-pinned pawn to the north...
							if( shiftN(onMoveKing) & onMovePawns & ~pinnedPieces)
							{
							// +---+---+
							// |PWN|pwn|  <- row 5 on chess board
							// +---+---+
							// |KNG|
							// +---+
								addMove(onMoveKingOffset -8, onMoveKingOffset - 15, moveList);
							}
							if( shiftNE(shiftE(onMoveKing)) & onMovePawns)
							//     +---+---+
							//     |pwn|PWN| <- row 5 on chess board
							// +---+---+---+
							// |KNG|
						   	// +---+
							{
								addMove(onMoveKingOffset -6, onMoveKingOffset - 15, moveList);
							}
						}
						else if( shiftNW(onMoveKing) & oppPawns )
						{
							if( shiftN(onMoveKing) & onMovePawns & ~pinnedPieces)
							{
							// +---+---+
							// |pwn|PWN|  <- row 5 on chess board
							// +---+---+
							//     |KNG|
							//     +---+
								addMove(onMoveKingOffset -8, onMoveKingOffset - 17, moveList);
							}
							if( shiftNW(shiftW(onMoveKing)) & onMovePawns)
							// +---+---+
							// |PWN|pwn| <- row 5 on chess board
							// +---+---+---+
							//         |KNG|
						   	//         +---+
							{
								addMove(onMoveKingOffset -10, onMoveKingOffset - 17, moveList);
							}
						}
					}
					else
					{
						if( shiftSE(onMoveKing) & oppPawns )
						{
							if( shiftS(onMoveKing) & onMovePawns & ~pinnedPieces)
							{
							// +---+
							// |kng|
							// +---+---+
							// |pwn|PWN|   <- row 4 on chess board
							// +---+---+
								addMove(onMoveKingOffset +8, onMoveKingOffset + 17, moveList);
							}
							if( shiftSE(shiftE(onMoveKing)) & onMovePawns)
							// +---+
							// |kng|
							// +---+---+---+
							//     |PWN|pwn|   <- row 4 on chess board
							//     +---+---+
							{
								addMove(onMoveKingOffset +10, onMoveKingOffset + 17, moveList);
							}
						}
						else if( shiftSW(onMoveKing) & oppPawns )
						{
							if( shiftS(onMoveKing) & onMovePawns & ~pinnedPieces)
							{
							//     +---+
							//     |kng|
							// +---+---+
							// |PWN|pwn|   <- row 4 on chess board
							// +---+---+
								addMove(onMoveKingOffset +8, onMoveKingOffset + 15, moveList);
							}
							if( shiftSW(shiftW(onMoveKing)) & onMovePawns)
							//         +---+
							//         |kng|
							// +---+---+---+
							// |pwn|PWN|   <- row 4 on chess board
							// +---+---+
							{
								addMove(onMoveKingOffset +6, onMoveKingOffset + 15, moveList);
							}
						}
					}
				}

			}
			// (2) CAN WE INTERPOSE?

			// If the attacker is a slider...
			if(kingAttackers & (oppOrthogSliders | oppDiagSliders))
			{
				BB path = 0;
				if(Nattackers & oppOrthogSliders)      path = Nattackers & ~oppOrthogSliders;
				else if(Sattackers & oppOrthogSliders) path = Sattackers & ~oppOrthogSliders;
				else if(Eattackers & oppOrthogSliders) path = Eattackers & ~oppOrthogSliders;
				else if(Wattackers & oppOrthogSliders) path = Wattackers & ~oppOrthogSliders;
				else if(NEattackers & oppDiagSliders)  path = NEattackers & ~oppDiagSliders;
				else if(NWattackers & oppDiagSliders)  path = NWattackers & ~oppDiagSliders;
				else if(SEattackers & oppDiagSliders)  path = SEattackers & ~oppDiagSliders;
				else if(SWattackers & oppDiagSliders)  path = SWattackers & ~oppDiagSliders;
				else ASSERT(0);

				// For each square in path, look for interposers...
				while(path)
				{
					int pathOffset = 63 - getLSBindex(path);

                    BB thisSquare = squareMask[pathOffset];

                    interposers = 0;

					interposers |= (knightCoverage[pathOffset] & b.colors[b.toMove] & b.pieces[KNIGHT] );

					interposers |=  ( Nattacks(thisSquare, empty) |
										Sattacks(thisSquare, empty) |
										Eattacks(thisSquare, empty) |
										Wattacks(thisSquare, empty) ) & onMoveOrthogSliders;

					interposers |=  ( NEattacks(thisSquare, empty) |
										NWattacks(thisSquare, empty) |
										SEattacks(thisSquare, empty) |
										SWattacks(thisSquare, empty) ) & onMoveDiagSliders;

					if(b.toMove == WHITE)
					{
						BB testSquare = shiftS(thisSquare);

						// Is there a white pawn to the S?
						if(testSquare & b.colors[WHITE] & b.pieces[PAWN])
						{
							// Add to list of interposers...
							interposers |= testSquare;
						}

						// If path is on rank 4...
					    else if(thisSquare & rowMask[4])
					    {
					    	// If the square to the south is empty
					    	if(testSquare & empty)
					    	{
					    		testSquare = shiftS(testSquare);

					    		// If the square to the south of the empty square is a non-pinned pawn...
					        	if(testSquare & b.colors[WHITE] & b.pieces[PAWN])
					        	{
					        		interposers |= testSquare;
					        	}
					        }
						}

					}
					else
					{
						BB testSquare = shiftN(thisSquare);

						// Is there a black pawn to the N?
						if(testSquare & b.colors[BLACK] & b.pieces[PAWN])
						{
							// Add to list of interposers...
							interposers |= testSquare;
						}

						// If path is on (board rank 5)...
					    else if(thisSquare & rowMask[3])
					    {
					    	// If the square to the north is empty
					    	if(testSquare & empty)
					    	{
					    		testSquare = shiftN(testSquare);

					    		// If the square to the north of the empty square is a non-pinned pawn...
					        	if(testSquare & b.colors[BLACK] & b.pieces[PAWN])
					        	{
					        		interposers |= testSquare;
					        	}
					        }
						}
					}

					// eliminate pinned pieces
					interposers &= ~pinnedPieces;

					while(interposers)
					{
						int interposerOffset;
						BB interposerMask = squareMask[( interposerOffset = 63 - getLSBindex(interposers))];

						if( (b.toMove == WHITE) && (interposerMask & b.pieces[PAWN] & rowMask[1]))
						{
							addMovePromote(interposerOffset, pathOffset, moveList);
						}
						else if ( (b.toMove == BLACK) && (interposerMask & b.pieces[PAWN] & rowMask[6]))
						{
							addMovePromote(interposerOffset, pathOffset, moveList);
						}
						else
						{
							addMove(interposerOffset, pathOffset, moveList);
						}

						clearlsb(interposers);
					}
					clearlsb(path);
				}

			}
		}
	}

	if(moveIndex == 0)
	{
		if(inCheck == TRUE) return CHECKMATE; else return STALEMATE;
	}
	else
	{
		return moveIndex;
	}
}

// Convert a move and board position to SAN notation
char *moveToSAN(move_t mv, board_t *b)
{
	piece_t moved;
	color_t onMove = b->toMove;
	color_t opp = b->toMove == WHITE ? BLACK : WHITE;
	BB empty = ~(b->colors[WHITE] | b->colors[BLACK]);
    revMove_t rev;


	// Place to keep SAN text...
	static char SANtext[11];

	// Some flags we will use to help things later...
	bool_t  capture = FALSE;
	bool_t  disambigRow = FALSE;
	bool_t  disambigFile = FALSE;
	bool_t  enPassant = FALSE;
	bool_t  castle = FALSE;
  //	bool_t  kingInCheck = FALSE;

	// Start with empty
	SANtext[0] = '\0';

	// determine which piece moved...
	if     ( squareMask[mv.from] & b->pieces[PAWN]   ) moved = PAWN;
	else if( squareMask[mv.from] & b->pieces[KNIGHT] ) moved = KNIGHT;
	else if( squareMask[mv.from] & b->pieces[BISHOP] ) moved = BISHOP;
	else if( squareMask[mv.from] & b->pieces[ROOK]   ) moved = ROOK;
	else if( squareMask[mv.from] & b->pieces[QUEEN]  ) moved = QUEEN;
	else moved = KING;

	// determine if there was a capture
	if( squareMask[mv.to] & b->colors[opp] )
	{
		capture = TRUE;
	}

	// determine if there was an enpassant capture
	if( (moved == PAWN) && (capture == FALSE) && ( (mv.to % 8) != (mv.from % 8) ) )
	{
		capture = TRUE;
		enPassant = TRUE;
	}

   // TODO CHESS960
	// determine if there was a castle
	if( (moved == KING) && (abs(mv.from - mv.to) == 2) )
	{
		castle = TRUE;
	}

	// determine if move is unabiguous
	BB allCandidates = 0;

	// Pawn and kings moves are never anbiguous in SAN notation...
	if(moved != PAWN && moved != KING)
	{
		// Candidate piece counts for same-file and same-row.
		int sameFileCount;
		int sameRowCount;

		if (moved == KNIGHT)
		{
			allCandidates = knightCoverage[mv.to] & b->pieces[KNIGHT] & b->colors[onMove];
		}
		else if (moved == BISHOP)
		{
			BB target = squareMask[mv.to];
			allCandidates = NEattacks(target, empty);
			allCandidates |= NWattacks(target, empty);
			allCandidates |= SEattacks(target, empty);
			allCandidates |= SWattacks(target, empty);

			// Mask off Bishops
			allCandidates &= (b->colors[onMove] & b->pieces[BISHOP]);
		}
		else if (moved == ROOK)
		{
			BB target = squareMask[mv.to];
			allCandidates = Nattacks(target, empty);
			allCandidates |= Sattacks(target, empty);
			allCandidates |= Eattacks(target, empty);
			allCandidates |= Wattacks(target, empty);

			// Mask off rooks
			allCandidates &= (b->colors[onMove] & b->pieces[ROOK]);
		}
		else if (moved == QUEEN)
		{
			BB target = squareMask[mv.to];
			allCandidates = Nattacks(target, empty);
			allCandidates |= Sattacks(target, empty);
			allCandidates |= Eattacks(target, empty);
			allCandidates |= Wattacks(target, empty);
			allCandidates |= NEattacks(target, empty);
			allCandidates |= NWattacks(target, empty);
			allCandidates |= SEattacks(target, empty);
			allCandidates |= SWattacks(target, empty);

			// mask off queens
			allCandidates &= (b->colors[onMove] & b->pieces[QUEEN]);
		}

        if(bitCount(allCandidates)>1)
        {

            // Count the candidates that have the same "from" file
            sameFileCount = bitCount(allCandidates & fileMask[mv.from % 8]);

            // Count the candidates that have the same "from" row
            sameRowCount  = bitCount(allCandidates & rowMask[mv.from/8]);

            // Do we need file or row (or both) to disambiguate this move?
            if     (sameFileCount == 1) disambigFile = TRUE;
            else if(sameRowCount  == 1) disambigRow  = TRUE;
            else                       {disambigFile = TRUE; disambigRow = TRUE;}
        }

	}

	// Now build the string....

	// Letter of piece that moved
	//    EXCEPTION1: Pawn doesn't apply
	//    EXCEPTION2: Castling
	switch(moved)
	{
		case KNIGHT:
			strcat(SANtext,"N");
			break;
		case BISHOP:
			strcat(SANtext,"B");
			break;
		case ROOK:
			strcat(SANtext,"R");
			break;
		case QUEEN:
			strcat(SANtext,"Q");
			break;
		case KING:
			if(castle != TRUE)         strcat(SANtext,"K");
         // TODO CHESS960
			else if( (mv.to % 8) == 2) strcat(SANtext,"O-O-O");
			else                       strcat(SANtext,"O-O");
			break;
		default:
			break;
	}

   // DEBUG We aren't using this now, but could be useful if we ever
   // decide to emit "e.p."
   //
   (void)enPassant;

	// If we need to disambiguate...
	if(disambigFile)
	{
		char fileStr[2];
		fileStr[0] = 'a'+mv.from%8;
		fileStr[1] = '\0';
		strcat(SANtext,fileStr);
	}
	if(disambigRow)
	{
		char rowStr[2];
		rowStr[0] = '1'+ 7 - mv.from/8;
		rowStr[1] = '\0';
		strcat(SANtext,rowStr);
	}

	// If there was a capture, emit an 'X'
	//   EXCEPTION: for pawns, we need to specify the originating file
	if(capture)
	{
		if(moved == PAWN)
		{
			char str[3];
			str[0] = 'a'+mv.from%8;
			str[1] = 'x';
			str[2] = '\0';
			strcat(SANtext, str);
		}
		else
		{
			strcat(SANtext,"x");
		}
	}

	// If we didn't castle, emit the target coordinate
	if(!castle)
	{
		char str[3];
		str[0] = 'a' + mv.to%8;
		str[1] = '1' + 7 - mv.to/8;
		str[2] = '\0';
		strcat(SANtext,str);
	}

	// If there was a promotion, add it...
	if(mv.promote != PIECE_NONE)
	{
		switch(mv.promote)
		{
			case KNIGHT:
				strcat(SANtext,"=N");
				break;
			case BISHOP:
				strcat(SANtext,"=B");
				break;
			case ROOK:
				strcat(SANtext,"=R");
				break;
			case QUEEN:
				strcat(SANtext,"=Q");
				break;
			default:
				break;
		}
	}

    // Determine if this move results in check or checkmate....

    // temporarily apply this move


	rev = move(b, mv);

	// If we are now in check, append the appropriate symbol...
	if(testInCheck(b))
	{
	    if( findMoves(b, NULL) == CHECKMATE )
	    {
	        strcat(SANtext, "#");
	    }
	    else
	    {
	        strcat(SANtext, "+");
	    }
	}
	unmove(b, rev);

	return SANtext;
}

// Adds 4 "copies" of a pawn promotion with the four possible promotion pieces...
static void addMovePromote(int from, int to, move_t *moveList)
{

    if(moveList != NULL)
    {

        ASSERT(moveIndex <= MAX_LIST_SIZE - 4);

        ASSERT(from >= 0 && from <= 63);

        ASSERT(to >= 0 && to <= 63);

        moveList[moveIndex].from = from;
        moveList[moveIndex].to   = to;
        moveList[moveIndex++].promote = KNIGHT;

        moveList[moveIndex].from = from;
        moveList[moveIndex].to   = to;
        moveList[moveIndex++].promote = BISHOP;

        moveList[moveIndex].from = from;
        moveList[moveIndex].to   = to;
        moveList[moveIndex++].promote = ROOK;

        moveList[moveIndex].from = from;
        moveList[moveIndex].to   = to;
        moveList[moveIndex++].promote = QUEEN;
    }
    else
    {
        moveIndex += 4;
    }
}

// Add a move to the array of moves
static void addMove(int from, int to, move_t *moveList)
{

    if(moveList != NULL)
    {
        ASSERT(moveIndex <= MAX_LIST_SIZE - 4);

        ASSERT(from >= 0 && from <= 63);

        ASSERT(to >= 0 && to <= 63);

        moveList[moveIndex].from = from;
        moveList[moveIndex].to   = to;
        moveList[moveIndex++].promote = PIECE_NONE;
    }
    else
    {
        moveIndex++;
    }
}
