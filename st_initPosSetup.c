#include "hsm.h"
#include "hsmDefs.h"
#include "st_initPosSetup.h"
#include "st_inGame.h"

#include <stddef.h>
#include "display.h"
#include "led.h"
#include "event.h"
#include "switch.h"
#include "diag.h"
#include "constants.h"
#include "switch.h"

#define TARGET_POSITION 0xFFFF00000000FFFF

static void showDiffs( void );

void initPosSetupEntry( event_t ev)
{

   inGame_SetPosition( NULL );

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
      displayWriteLine( 3, "any btn = go back", TRUE );
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

   // NOTE:  d8 is "relative", in that a flipped board will see e1 as d8.  Put simply, if the
   //   flashing square does not map to the side the user has placed the white king, we are going
   //   to flip the LED logid and reed switch logic to "flip" the board
   static bool lastChangeWasD8Lift;


   if(ev.ev == EV_PIECE_LIFT && ev.data == D8)
      lastChangeWasD8Lift = true;
   else
   {
      if(ev.ev == EV_PIECE_DROP && ev.data == D8 && lastChangeWasD8Lift == true)
      {
         DPRINT("DEBUG:  Flip the board!\n");
         LED_flipBoard();
         SW_flipBoard();
      }
      lastChangeWasD8Lift = false;
   }



   if(GetSwitchStates() == TARGET_POSITION)
   {
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

   // Flash White King Square if empty
   if(~GetSwitchStates() &  e1 )
      LED_FlashGridState ( e1 );
   else
      LED_FlashGridState( 0 );

}
