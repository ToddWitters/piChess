#ifndef TYPES_H
#define TYPES_H

#include "inttypes.h"

#define BB  uint64_t
#define U64 uint64_t
#define U32 uint32_t
#define U16 uint16_t
#define S16  int16_t
#define U8   uint8_t

#define MAX_MOVES_IN_GAME  500

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

// Move + information to reverse it....
typedef struct revMov_s
{
	move_t move;

	unsigned short priorCastleBits  :4; // status of castle bits prior to move being made
	unsigned short priorEnPassant   :4; // previous status of enPassant
	unsigned short captured         :3; // piece captured by this move (PIECE_NONE if no capture)

	unsigned short priorHalfMoveCnt;    // previous value of half-move counter
	unsigned short priorZobristEnPassantCol;

}revMove_t;


typedef enum gameDisposition_e
{
   GAME_INVALID,       // Invalid position
   GAME_PLAYABLE,      // Valid position with available moves
   GAME_AT_CHECKMATE,  // Side to move in check with no legal moves
   GAME_AT_STALEMATE   // Size to move not in check with no legal moves
}gameDisposition_t;


typedef struct posHistory_s
{
   uint32_t clocks[2];  // Clock value when this position reached
   U64      posHash;    // The hash value of this position
   move_t   move;       // The move that was selected from this position
   revMove_t   revMove;    // The undo information to go back to the previous position
}posHistory_t;


// Current state of the game...
typedef struct game_s
{
    // fen of start position (NULL if normal start position)
    char *startPos;

    // The current board state
    board_t  brd;

    // 0.1 second intervals on the two player clocks
    uint32_t wtime;
    uint32_t btime;

    uint32_t wIncrement;
    uint32_t bIncrement;

    // Time remaining for player to make move for computer before time is
    ///   counted against them...
    uint16_t graceTime;

    // posHistory (position hash value of last 75 moves)
    //   Used to enforce the 3-fold and 5-fold repetition rules
    //  Holds the hash values of all positions since last capture, pawn push OR castling availability change
    posHistory_t posHistory[MAX_MOVES_IN_GAME];

    // Holds the coord notation text string of the moves made in this game
    //  Sized to hold 4-character + 1 space per move + 16 promotion indicators + null string terminator.
    char moveRecord[MAX_MOVES_IN_GAME * 5 + 16 + 1];
    char SANRecord[MAX_MOVES_IN_GAME * 5];

    int playedMoves;  // total number of half-moves already made in game

    // TRUE for chess960 games
    bool_t chess960;

    gameDisposition_t disposition;

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

// Each position in the move tree has the following:
typedef struct moveList_s
{
    short   numLegalMoves;  // Number of legal moves from this position.  Determines size of moves and links arrays.
    move_t *moves;          // pointer to move list array.  NULL if not yet generated OR checkmate/stalemate position
    U8      modifying;      // Tracks recursion levels that modify this

}moveList_t;

typedef enum endReason_s
{
   GAME_END_CHECKMATE,
   GAME_END_STALEMATE,
   GAME_END_INSUFFICIENT_MATERIAL,
   GAME_END_50_MOVE,
   GAME_END_75_MOVE,
   GAME_END_3FOLD_REP,
   GAME_END_5FOLD_REP,
   GAME_END_TIME_EXPIRED,
   GAME_END_ABORT,
}endReason_t;


#define SECONDS_IN_MINUTE 60

typedef enum timingType_e
{
   TIME_NONE,
   TIME_EQUAL,
   TIME_ODDS,
}timingType_t;

typedef enum computerStrategy_e
{
   STRAT_FIXED_DEPTH,
   STRAT_FIXED_TIME,
   STRAT_TILL_BUTTON
}computerStrategy_t;

typedef struct compStrategySetting_s
{
   computerStrategy_t type;
   uint8_t  depth;
   uint32_t timeInMs;
}compStrategySetting_t;

typedef struct periodTimingSettings_s
{
   uint16_t totalTime; // in seconds
   uint8_t  increment; // in seconds
   uint8_t  moves;     // moves in this period.  0 = sudden death.
}periodTimingSettings_t;

typedef struct timeControl_s
{

   timingType_t           type;
   compStrategySetting_t     compStrategySetting; // only used if type == TIME_NONE

   // If type == NONE, this field is unused.
   // If type == EQUAL
   //   timeSettings[0] = Period one settings
   //   timeSettings[1] = Period two settings   (invalid if timeSettings[0].moves == 0)
   //   timeSettings[2] = Period three settings (invalid if timeSettings[0].moves == 0 || timeSettings[1].moves)
   //     note: timeSettings[2].moves is ignored
   // If type == ODDS ([x].moves ignored)
   //   timeSettings[WHITE] = white player settings
   //   timeSettings[BLACK] = black player settings
   periodTimingSettings_t timeSettings[3];

}timeControl_t;

#endif
