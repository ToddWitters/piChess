#include "hsm.h"
#include "hsmDefs.h"
#include "st_initPosSetup.h"

#include "display.h"
#include "led.h"
#include "event.h"
#include "switch.h"
#include "diag.h"

#define TARGET_POSITION 0xFFFF00000000FFFF

static void showDiffs( void );

void initPosSetupEntry( event_t ev)
{

   if(GetSwitchStates() == TARGET_POSITION)
   {
      event_t event = {EV_GOTO_GAME, 0};
      putEvent(EVQ_EVENT_MANAGER, &event);
   }
   else
   {
      displayClear();
      displayWriteLine( 0, "Set pieces to", TRUE );
      displayWriteLine( 1, "initial position", TRUE );
      displayWriteLine( 3, "Press btn to go back", TRUE );
      showDiffs();
   }
}

void initPosSetupExit( event_t ev)
{
   displayClear();
   LED_AllOff();
}

void initPosSetup_boardChange( event_t ev)
{
   (void)ev;

   if(GetSwitchStates() == TARGET_POSITION)
   {
      // DEBUG... This just takes us back to the main menu for now...
      event_t event = {EV_GOTO_GAME, 0};

      DPRINT("Initial state reached\n");
      putEvent(EVQ_EVENT_MANAGER, &event);
   }
   else
   {
      showDiffs();
   }
}

static void showDiffs( void )
{
   // Set LEDs to solid where starting position are vacant
   LED_SetGridState(   ~GetSwitchStates() &  TARGET_POSITION );

   // Set LEDs to flash if pieces found on non-starting squares.
   LED_FlashGridState ( GetSwitchStates() & ~TARGET_POSITION );
}
