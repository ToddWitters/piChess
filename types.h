#ifndef TYPES_H
#define TYPES_H

#include "inttypes.h"

#define BB  uint64_t
#define U64 uint64_t
#define U32 uint32_t
#define U16 uint16_t
#define S16  int16_t
#define U8   uint8_t


typedef enum dir_e
{
	NORTH,
	SOUTH,
	EAST,
	WEST,
	NORTHEAST,
	NORTHWEST,
	SOUTHEAST,
	SOUTHWEST,
	DIR_NONE
}dir_t;

typedef enum bool_e
{
	FALSE,
	TRUE
}bool_t;

typedef enum color_e
{
	BLACK,
	WHITE,
	COLOR_NONE
}color_t;

typedef enum piece_e
{
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	PIECE_NONE,
}piece_t;

#define WHITE_CASTLE_SHORT  0x08
#define WHITE_CASTLE_LONG   0x04
#define BLACK_CASTLE_SHORT  0x02
#define BLACK_CASTLE_LONG   0x01

// Passed to eval function...
typedef struct board_s
{
	BB colors[2]; // bit boards for each color (color_t is used as offset)
	BB pieces[6]; // bit boards for each piece type (piece_t is used as offset)

	U16 moveNumber; // The move number about to be made
	U16 halfMoves;  // # of half moves since last irreversible move

	color_t toMove; // Which color is on move

	U8 castleBits;  // Castle bits (ordered bit 3 to bit 0) in same order as FEN bits.

	U8 enPassantCol;  // 0-7 = a-h, and 8 = no enPassant

	U8 zobristEnPassantCol; // 0-7  differs from enPassantCol in that it must have adjacent pawn

	// running material count (excluding kings)
	U16 materialCount[2]; // running tally of material count for each color (color_t used as offset)

	// running hash value of board state used for opening book comparison
	U64 hash;
}board_t;

// minimal info needed to apply a move to the board...
typedef struct move_s
{
	unsigned short from             :6; // index of originating square.
	unsigned short to               :6; // index of target square
	unsigned short promote          :3; // piece pawn promotes to (ignored unless PAWN moves into promoting row)
}move_t;


// Current state of the game...
typedef struct game_s
{
    // fen of start position (NULL if normal start position)
    char *startPos;

    // The current board state
    board_t  *brd;

    // 0.1 second intervals on the two player clocks
    uint32_t wtime;
    uint32_t btime;

    // Time allowed for player to make move for computer
    uint16_t graceTime;

    // posHistory (position hash value of last 75 moves)
    //   Used to enforce the 3-fold and 5-fold repetition rules
    //  Holds the hash values of all positions since last capture, pawn push OR castling availability change
    U64 posHash[150];

    // playedMoves
    move_t moves[500];

    // TRUE for chess960 games
    bool_t chess960;
    
}game_t;

typedef enum boardErr_e
{

    BRD_NO_ERROR,
    ERR_BAD_WHITE_KING_COUNT,
    ERR_BAD_BLACK_KING_COUNT,
    ERR_OPPOSING_KINGS,
    ERR_OPP_ALREADY_IN_CHECK,
    ERR_BAD_PAWN_POSITION,
    ERR_TOO_MANY_WHITE_PIECES,
    ERR_TOO_MANY_BLACK_PIECES,

}boardErr_t;

// Move + information to reverse it....
typedef struct revMov_s
{
	move_t move;

	unsigned short priorCastleBits  :4; // status of castle bits prior to move being made
	unsigned short priorEnPassant   :4; // previous status of enPassant
	unsigned short captured         :3; // piece captured by this move (PIECE_NONE if no capture)

	unsigned short priorHalfMoveCnt;    // previous value of half-move counter
	unsigned short priorZobristEnPassantCol;

	uint32_t priorwtime;
	uint32_t priorbtime;

}revMove_t;

// Each position in the move tree has the following:
typedef struct moveList_s
{
    short   numLegalMoves;  // Number of legal moves from this position.  Determines size of moves and links arrays.
    move_t *moves;          // pointer to move list array.  NULL if not yet generated OR checkmate/stalemate position
    U8      modifying;      // Tracks recursion levels that modify this

}moveList_t;





#endif
