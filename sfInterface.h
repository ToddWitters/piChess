#ifndef SFINTERFACE_H
#define SFINTERFACE_H

#include "types.h"

#define OUTPUT_FILE "/home/pi/chess/result.txt"

void   SF_initEngine( void );
void   SF_setPosition( char *fen, char *moveList);
void   SF_setOption( char *name, char *value);
void   SF_findMove( uint32_t wt, uint32_t bt, uint32_t wi, uint32_t bi);
void   SF_findMoveFixedDepth( int d );
void   SF_findMoveFixedTime( uint32_t t );
void   SF_stop( void );
void   SF_go( void );
void   SF_closeEngine( void );

#endif
