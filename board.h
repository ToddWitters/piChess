#include "types.h"



typedef enum fenErr_e
{
	FEN_OK,
	FEN_BAD_SEPARATOR,
	FEN_ILLEGAL_CHARACTER_FIELD_1,
	FEN_PARSE_ERR_FIELD_1,
	FEN_INVALID_FIELD_2,
	FEN_INVALID_FIELD_3,
	FEN_INVALID_FIELD_4,
	FEN_INVALID_FIELD_5,
	FEN_INVALID_FIELD_6
}fenErr_t;

revMove_t move(board_t *b, const move_t m);
void unmove(board_t *b, const revMove_t m);
fenErr_t setBoard(board_t *brd, const char *FEN);
char *getFEN(const board_t *b);
boardErr_t testValidBoard(board_t *b);
bool_t testInCheck( board_t *b );
void removePiece(board_t *b, U8 sq, piece_t p, color_t c);
void addPiece(board_t *b, U8 sq, piece_t p, color_t c);


#define POSITION_HISTORY_SIZE 200
extern U64 positionHistory[POSITION_HISTORY_SIZE];
extern int positionIndex;

extern const char *startString;
