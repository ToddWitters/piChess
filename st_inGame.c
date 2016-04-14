#include "hsm.h"
#include "hsmDefs.h"
#include "st_inGame.h"

#include "types.h"
#include "board.h"

static game_t game;


void inGameEntry( event_t ev )
{
   // Iniitalize a game object
}
void inGameExit( event_t ev )
{
   game.disposition = GAME_INVALID;
}
uint16_t inGamePickSubstate( event_t ev)
{
   return ST_PLAYING_GAME;
}

void inGame_moveClockTick( event_t ev)
{
   
}

void inGame_SetPosition( const char *FEN)
{
   setBoard( &game.brd, FEN);

   // TODO check valid position
   game.disposition = GAME_IN_PROGRESS;
}


