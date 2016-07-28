#include "hsm.h"
#include "hsmDefs.h"
#include "st_engineOptionMenu.h"
#include "menu.h"
#include <stddef.h>
#include "options.h"
#include "stdio.h"

static char *engineOptionsMenu_pickStrength( int dir );

menu_t *engineOptionMenu;

void engineOptionMenuEntry( event_t ev )
{
   if(engineOptionMenu == NULL)
   {
      engineOptionMenu = createMenu("---Engine Options---", EV_GOTO_OPTION_MENU);

      menuAddItem(engineOptionMenu, ADD_TO_END, "Go Back",  EV_GOTO_OPTION_MENU, EV_GOTO_OPTION_MENU, NULL);
      menuAddItem(engineOptionMenu, ADD_TO_END, "Strength",  0, 0, engineOptionsMenu_pickStrength);

   }

   drawMenu(engineOptionMenu);

}

void engineOptionMenuExit( event_t ev )
{
   destroyMenu(engineOptionMenu);
   engineOptionMenu = NULL;

}

static char *engineOptionsMenu_pickStrength( int dir )
{
   static char textString[3];

   if(dir == 1)
   {
      //TODO... set periodic timer to insert right events?
      if(options.engine.strength < 20) options.engine.strength++;
   }
   else if(dir == -1)
   {
      //TODO... set periodic timer to insert left events?
      if(options.engine.strength > 0)  options.engine.strength--;

   }

   sprintf(textString, "%d", options.engine.strength);
   textString[2] = 0;
   return textString;
}
