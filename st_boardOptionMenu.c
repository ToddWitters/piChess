#include "hsm.h"
#include "hsmDefs.h"
#include "st_boardOptionMenu.h"
#include "menu.h"
#include <stddef.h>

#include "led.h"
#include "options.h"
#include <stdio.h>

menu_t *boardOptionMenu;

static char *boardOptionMen_pickLEDBrightness( int dir );
static char *boardOptionMen_pickFlipBoard( int dir);


void boardOptionMenuEntry( event_t ev )
{
   if(boardOptionMenu == NULL)
   {
      boardOptionMenu = createMenu("----Board Options---", EV_GOTO_OPTION_MENU);

      menuAddItem(boardOptionMenu, ADD_TO_END, "Go Back",         EV_GOTO_OPTION_MENU, EV_GOTO_OPTION_MENU, NULL);
      menuAddItem(boardOptionMenu, ADD_TO_END, "LED Brightness",  0,                   0                  , boardOptionMen_pickLEDBrightness);
      menuAddItem(boardOptionMenu, ADD_TO_END, "Flip board",  0,                   0                  , boardOptionMen_pickFlipBoard);
   }

   drawMenu(boardOptionMenu);
   LED_SetGridState(0x00000000000000FF);

}

void boardOptionMenuExit( event_t ev )
{
   LED_AllOff();
   destroyMenu(boardOptionMenu);
   boardOptionMenu = NULL;
   LED_SetGridState(0x0000000000000000);
}


static char *boardOptionMen_pickFlipBoard( int dir)
{
   static char valueString[4];

   if(dir == 1 || dir == -1)
   {
      if(SW_getFlippedState() == true)
      {
         SW_SetFlip(false);
         LED_SetFlip(false);
      }
      else
      {
         SW_SetFlip(true);
         LED_SetFlip(true);
      }

      LED_SetGridState(0x00000000000000FF);
   }
   sprintf(valueString, "%s", (SW_getFlippedState() == true ? " on" : "off"));
   valueString[3] = 0x00;

   return valueString;
}

static char *boardOptionMen_pickLEDBrightness( int dir )
{

   static char valueString[3];

   if(dir == 1 && options.board.LED_Brightness < 15)
   {
      options.board.LED_Brightness++;
   }
   else if( dir == -1 && options.board.LED_Brightness > 1)
   {
      options.board.LED_Brightness--;
   }

   sprintf(valueString, "%d", options.board.LED_Brightness);
   valueString[2] = 0;

   if(dir != 0)
   {
      LED_SetBrightness( options.board.LED_Brightness );
      LED_SetGridState( 0x0000001818000000);
   }
   return valueString;
}
