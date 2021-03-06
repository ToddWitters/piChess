#include "hsm.h"
#include "hsmDefs.h"
#include "st_engineOptionMenu.h"
#include "menu.h"
#include <stddef.h>
#include "options.h"
#include "stdio.h"

static char *engineOptionsMenu_pickStrength( int dir );
static char *engineOptionsMenu_pickBook( int dir );

menu_t *engineOptionMenu;

void engineOptionMenuEntry( event_t ev )
{
   if(engineOptionMenu == NULL)
   {
      engineOptionMenu = createMenu("---Engine Options---", EV_GOTO_OPTION_MENU);

      menuAddItem(engineOptionMenu, ADD_TO_END, "Go Back",      EV_GOTO_OPTION_MENU, EV_GOTO_OPTION_MENU, NULL);
      menuAddItem(engineOptionMenu, ADD_TO_END, "Strength",     0,                   0,                   engineOptionsMenu_pickStrength);
      menuAddItem(engineOptionMenu, ADD_TO_END, "OpeningBook",  0,                   0,                   engineOptionsMenu_pickBook);

   }

   drawMenu(engineOptionMenu);

}

void engineOptionMenuExit( event_t ev )
{
   destroyMenu(engineOptionMenu);
   engineOptionMenu = NULL;

}

static char* engineOptionsMenu_pickBook( int dir )
{
   if( dir == 1 || dir == -1 )
   {
      if(options.game.useOpeningBook == TRUE )
      {
         options.game.useOpeningBook = FALSE;
      }
      else
      {
         options.game.useOpeningBook = TRUE;
      }
   }

   return (options.game.useOpeningBook == TRUE ? "On" : "Off");

}


static char *engineOptionsMenu_pickStrength( int dir )
{
   static char textString[3];

   int strength = getOptionVal("engineStrength");

   if(dir == 1)
   {
      if(strength < 20)
         setOptionVal("engineStrength", ++strength);
   }
   else if(dir == -1)
   {
      if(strength > 0)
         setOptionVal("engineStrength", --strength);
   }

   sprintf(textString, "%d", strength);
   textString[2] = 0;
   return textString;
}
