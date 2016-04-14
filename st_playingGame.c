#include "hsm.h"
#include "hsmDefs.h"
#include "st_playingGame.h"

void playingGameEntry( event_t ev )
{
   
}

void playingGameExit( event_t ev )
{
   
}

uint16_t playingGamePickSubstate( event_t ev)
{
#if 0
   // TODO: WHAT IF GAME STRUCTURE IS NOT IN "PLAYING" State?

   if(computerMovePending == true)
      return ST_MOVE_FOR_COMPUTER;
   else if(game.brd.toMove == WHITE)
      if(options.game.white == PLAYER_HUMAN)
         return ST_PLAYER_MOVE;
      else
         return ST_COMPUTER_MOVE;
   else
      if(options.game.black == PLAYER_HUMAN)
         return ST_PLAYER_MOVE;
      else
         return ST_COMPUTER_MOVE;
         
#endif
         
   return ST_PLAYER_MOVE;
}