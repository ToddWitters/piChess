#include "hsm.h"
#include "hsmDefs.h"
#include "st_optionMenu.h"
#include "options.h"
#include "display.h"

#include "menu.h"
#include <stddef.h>

menu_t *optionMenu = NULL;

void optionMenuEntry( event_t ev )
{

   if(optionMenu == NULL)
   {
      optionMenu = createMenu("-------Options------", EV_GOTO_MAIN_MENU);

      menuAddItem(optionMenu, ADD_TO_END, "Go Back", EV_GOTO_MAIN_MENU,     EV_GOTO_MAIN_MENU, NULL);
      menuAddItem(optionMenu, ADD_TO_END, "Board",   EV_GOTO_BOARD_OPTIONS,  EV_GOTO_BOARD_OPTIONS, NULL);
      menuAddItem(optionMenu, ADD_TO_END, "Game",    EV_GOTO_GAME_OPTIONS,   EV_GOTO_GAME_OPTIONS, NULL);
      menuAddItem(optionMenu, ADD_TO_END, "Engine",  EV_GOTO_ENGINE_OPTIONS, EV_GOTO_ENGINE_OPTIONS, NULL);

   }

   drawMenu( optionMenu );
}

void optionMenuExit( event_t ev )
{
   destroyMenu(optionMenu);
   optionMenu = NULL;
}
