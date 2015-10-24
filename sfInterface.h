#ifndef SFINTERFACE_H
#define SFINTERFACE_H

#include "types.h"

void   SF_initEngine( void );
void   SF_setPosition( char *fen);
void   SF_setOption( char *name, char *value);
void   SF_findMoveFixedDepth( int d );
void   SF_findMoveFixedTime( uint32_t t );
move_t SF_checkDone( move_t *ponder );

#define MAX_LINE_LEN 300

#endif
