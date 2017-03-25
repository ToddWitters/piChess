#ifndef UTIL_H
#define UTIL_H

#include "types.h"
#include "hsm.h"

char *convertSqNumToCoord(int sq);
move_t convertCoordMove( char *coord );
uint64_t reverseBitOrder64( uint64_t input);
piece_t getPieceAtSquare( board_t *brd, uint8_t sq );
color_t getColorAtSquare( board_t *brd, uint8_t sq );


#endif
