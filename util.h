#ifndef UTIL_H
#define UTIL_H

#include "types.h"
#include "hsm.h"

char *convertSqNumToCoord(int sq);
move_t convertCoordMove( char *coord );
bool_t isStatePress(event_t ev );
uint64_t reverseBitOrder64( uint64_t input);


#endif
