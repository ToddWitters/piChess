#include "hsm.h"
#include "hsmDefs.h"
#include "st_splashScreen.h"

#include <stddef.h>
#include "menu.h"

static menu_t *mainMenu = NULL;

void mainMenuEntry( event_t ev )
{
   if(mainMenu == NULL)
   {
      mainMenu = createMenu("---- Main Menu -----", 0);

      //          menu      offset      text           press                     right                    picker
      menuAddItem(mainMenu, ADD_TO_END, "Play",        EV_START_INIT_POS_SETUP,  EV_START_INIT_POS_SETUP, NULL);
      menuAddItem(mainMenu, ADD_TO_END, "Options",     EV_GOTO_OPTION_MENU,      EV_GOTO_OPTION_MENU,     NULL);
      menuAddItem(mainMenu, ADD_TO_END, "Diagnostics", EV_GOTO_DIAG_MENU,        EV_GOTO_DIAG_MENU,       NULL);
   }

   drawMenu(mainMenu);
}

void mainMenuExit( event_t ev )
{
   destroyMenu(mainMenu);
   mainMenu = NULL;
}
