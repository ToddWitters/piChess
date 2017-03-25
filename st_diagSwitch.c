#include "hsm.h"
#include "hsmDefs.h"
#include "st_diagSwitch.h"

#include "menu.h"
#include "display.h"
#include "options.h"
#include "timer.h"
#include "led.h"
#include <stddef.h>


uint16_t prevLiftDeb, prevDropDeb;

void diagSwitchEntry( event_t ev )
{
   (void)ev;

   displayClear();
   displayWriteLine(1, "Sensor Test Mode", TRUE);
   displayWriteLine(2, "any button to exit", TRUE);

   prevDropDeb = options.board.pieceDropDebounce;
   prevLiftDeb = options.board.pieceLiftDebounce;

   options.board.pieceDropDebounce = (50 / MS_PER_TIC);
   options.board.pieceLiftDebounce = (50 / MS_PER_TIC);

   LED_SetGridState(GetSwitchStates());

}

void diagSwitchExit( event_t ev )
{
   (void)ev;
   LED_AllOff();
   displayClear();

   options.board.pieceDropDebounce = prevDropDeb;
   options.board.pieceLiftDebounce = prevLiftDeb;
}

void diagSwitch_boardChange( event_t ev )
{
   LED_SetGridState(GetSwitchStates());
}
