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

/// Direction indicators
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

/// Simple boolean
typedef enum bool_e
{
	FALSE,
	TRUE
}bool_t;

/// Color
typedef enum color_e
{
	BLACK,
	WHITE,
	COLOR_NONE
}color_t;

/// Piece designations
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

/// Information about the current board state
typedef struct board_s
{
	BB colors[2]; ///< bit boards for each color (color_t is used as offset)
	BB pieces[6]; ///< bit boards for each piece type (piece_t is used as offset)

	U16 moveNumber; ///< The move number about to be made
	U16 halfMoves;  ///< number of half moves since last irreversible move

	color_t toMove; ///< Which color is on move

	U8 castleBits;  ///< Castle bits (ordered bit 3 to bit 0) in same order as FEN bits.

	U8 enPassantCol;  ///< 0-7 = a-h, and 8 = no enPassant

	U8 zobristEnPassantCol; ///< 0-7  differs from enPassantCol in that it must have adjacent pawn

	U16 materialCount[2]; ///< running tally of material count for each color (color_t used as offset).  Excludes King

	U64 hash; ///< running hash value of board state used for opening book comparison
}board_t;

/// Minimal info needed to apply a move to the board...
typedef struct move_s
{
	unsigned short from             :6; ///< index of originating square.
	unsigned short to               :6; ///< index of target square
	unsigned short promote          :3; ///< piece pawn promotes to (ignored unless PAWN moves into promoting row)
}move_t;

/// Move plus information for reversing it.
typedef struct revMov_s
{
	move_t move;

	unsigned short priorCastleBits  :4; ///< status of castle bits prior to move being made
	unsigned short priorEnPassant   :4; ///< previous status of enPassant
	unsigned short captured         :3; ///< piece captured by this move (PIECE_NONE if no capture/<)

	unsigned short priorHalfMoveCnt;          ///< previous value of half-move counter
	unsigned short priorZobristEnPassantCol;  ///< previous value of Zobrist enPassant column.
}revMove_t;


/// Current disposition of the game
typedef enum gameDisposition_e
{
   GAME_INVALID,       ///< Invalid position
   GAME_PLAYABLE,      ///< Valid position with available moves
   GAME_AT_CHECKMATE,  ///< Side to move in check with no legal moves
   GAME_AT_STALEMATE   ///< Size to move not in check with no legal moves
}gameDisposition_t;

/// History of all previous positions
typedef struct posHistory_s
{
   uint32_t clocks[2];  ///< Clock value when this position reached
   U64      posHash;    ///< The hash value of this position
   move_t   move;       ///< The move that was selected from this position
   revMove_t   revMove; ///< The undo information to go back to the previous position
}posHistory_t;


/// Current state of the game...
typedef struct game_s
{
    char *startPos; ///< fen of start position (NULL if normal start position)


    board_t  brd; ///< The current board state


    uint32_t wtime; ///< White clock in 0.1 second intervals
    uint32_t btime; ///< Black clock in 0.1 second intervals

    uint32_t wIncrement; ///< White clock increment in seconds
    uint32_t bIncrement; ///< Black clock increment in seconds

    uint16_t graceTime; ///< Time remaining for player to make move for computer before time is counted against them...

    /// Position hash value.  Used to enforce the 3-fold and 5-fold repetition rules
    posHistory_t posHistory[MAX_MOVES_IN_GAME];

    char moveRecord[MAX_MOVES_IN_GAME * 5 + 16 + 1]; ///< The coord notation text string of the moves made in this game.  This is passed to the chess engine
    char SANRecord[MAX_MOVES_IN_GAME * 5]; ///< The SAN notation text string of the moves made in this game.  This is used for recording of game and display of recent moves on display

    int playedMoves;  ///< total number of half-moves already made in game


    bool_t chess960; ///< indicates a chess960 starting position

    gameDisposition_t disposition; ///< Current disposition of the game

}game_t;

/// Board position check results
typedef enum boardErr_e
{

    BRD_NO_ERROR,              ///< Legal position
    ERR_BAD_WHITE_KING_COUNT,  ///< Missing or too many white kings
    ERR_BAD_BLACK_KING_COUNT,  ///< Missing or too many black kings
    ERR_OPPOSING_KINGS,        ///< Kings on adjacent squares
    ERR_OPP_ALREADY_IN_CHECK,  ///< King is capturable
    ERR_BAD_PAWN_POSITION,     ///< Pawn on first or last rank
    ERR_TOO_MANY_WHITE_PIECES, ///< White has >16 pieces on board
    ERR_TOO_MANY_BLACK_PIECES, ///< Black has >16 pieces on board

}boardErr_t;

/// Reasons for game termination
typedef enum endReason_s
{
   GAME_END_CHECKMATE,              ///< Checkmate
   GAME_END_STALEMATE,              ///< Stalemate
   GAME_END_INSUFFICIENT_MATERIAL,  ///< Neither side can deliver checkmate
   GAME_END_50_MOVE,                ///< 50 move rule claimed
   GAME_END_75_MOVE,                ///< 75 move rule occurred
   GAME_END_3FOLD_REP,              ///< 3-fold repetition claimed
   GAME_END_5FOLD_REP,              ///< 5-fold repetition occurred
   GAME_END_TIME_EXPIRED,           ///< Time expiration claimed
   GAME_END_ABORT,                  ///< Game was aborted before finish
}endReason_t;


#define SECONDS_IN_MINUTE 60

/// Clock modes
typedef enum timingType_e
{
   TIME_NONE,   ///< Untimed game
   TIME_EQUAL,  ///< Sides have equal times/increments
   TIME_ODDS,   ///< Sides have different times/increments
}timingType_t;

/// Computer strategy when clocks not used
typedef enum computerStrategy_e
{
   STRAT_FIXED_DEPTH,  ///< Search to a fixed depth
   STRAT_FIXED_TIME,   ///< Search for a fixed time
   STRAT_TILL_BUTTON   ///< Search until user presses a button
}computerStrategy_t;

/// Parameters for computer strategy when clocks not used
typedef struct compStrategySetting_s
{
   computerStrategy_t type;  ///< Type of strategy used
   uint8_t  depth;           ///< Depth of search (if type == STRAT_FIXED_DEPTH)
   uint32_t timeInMs;        ///< Time of search (if type == STRAT_FIXED_TIME)
}compStrategySetting_t;

/// Time parameters governing a period
typedef struct periodTimingSettings_s
{
   uint16_t totalTime; ///< Total clock time allowed in seconds
   uint8_t  increment; ///< Inrement to be applied after each move
   uint8_t  moves;     ///< Number of moves in this period.  0 = sudden death.
}periodTimingSettings_t;

/// Overall time settings
typedef struct timeControl_s
{

   timingType_t           type;                    ///< Clock mode in use
   compStrategySetting_t     compStrategySetting;  ///< Computer strategy (if untimed game)

   // If type == NONE, this field is unused.
   // If type == EQUAL
   //   timeSettings[0] = Period one settings
   //   timeSettings[1] = Period two settings   (invalid if timeSettings[0].moves == 0)
   //   timeSettings[2] = Period three settings (invalid if timeSettings[0].moves == 0 || timeSettings[1].moves)
   //     note: timeSettings[2].moves is ignored
   // If type == ODDS ([x].moves ignored)
   //   timeSettings[WHITE] = white player settings
   //   timeSettings[BLACK] = black player settings

   periodTimingSettings_t timeSettings[3]; ///< Time settings for each period

}timeControl_t;

#endif
