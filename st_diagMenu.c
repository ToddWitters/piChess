#include "hsm.h"
#include "hsmDefs.h"
#include "st_diagMenu.h"

#include "menu.h"
#include <stddef.h>

menu_t *diagMenu = NULL;

void diagMenuEntry( event_t ev )
{

   if(diagMenu == NULL)
   {
      diagMenu = createMenu("-----Diagnostics-----", EV_GOTO_MAIN_MENU);
      menuAddItem(diagMenu, ADD_TO_END, "Switches", EV_START_SENSOR_DIAG, EV_START_SENSOR_DIAG, NULL);
   }

   drawMenu( diagMenu );
}

void diagMenuExit( event_t ev )
{
   destroyMenu(diagMenu);
   diagMenu = NULL;
}
