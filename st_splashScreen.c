#include "hsm.h"
#include "hsmDefs.h"
#include "st_splashScreen.h"

#include "display.h"
#include "timer.h"


void splashScreenEntry( event_t ev)
{
   displayClear();
   displayWriteLine( 0, "piChess version 1.0", TRUE );
   displayWriteLine( 1, "by Todd Witters", TRUE );
   displayWriteLine( 2, "powered by", TRUE );
   displayWriteLine( 3, "Stockfish 8", TRUE );

   // Start time-out for splash screen.
   timerStart(TMR_UI_TIMEOUT, 5000, 0, EV_GOTO_MAIN_MENU);

}

void splashScreenExit( event_t ev)
{
   timerKill(TMR_UI_TIMEOUT);
   displayClear();
}


