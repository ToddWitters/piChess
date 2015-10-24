#include "types.h"

#define CHECKMATE -1
#define STALEMATE -2


#define MAX_LIST_SIZE 200

char *moveToSAN(move_t mv, board_t *b);

int findMoves(board_t *b, move_t *moveList);
