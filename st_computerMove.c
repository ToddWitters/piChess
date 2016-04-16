#include "hsm.h"
#include "hsmDefs.h"
#include "st_computerMove.h"

#include "st_inGame.h"
#include "display.h"

void computerMoveEntry( event_t ev )
{
   displayWriteLine(1, "COMPUTER'S MOVE", TRUE);

}

void computerMoveExit( event_t ev )
{
   // Zero out any remaining time...
   game.graceTime = 0;

}
