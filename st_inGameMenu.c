#include "hsm.h"
#include "hsmDefs.h"
#include "st_inGameMenu.h"

#include <stddef.h>
#include "menu.h"
#include "display.h"

menu_t *inGameMenu = NULL;

void inGameMenuEntry( event_t ev )
{
   if(inGameMenu == NULL)
   {
      inGameMenu = createMenu("---- Game Menu -----", 0);

      //          menu      offset      text                   press                  right   picker
      menuAddItem(inGameMenu, ADD_TO_END, "Back to Game",        EV_GOTO_PLAYING_GAME,  0,      NULL);
      menuAddItem(inGameMenu, ADD_TO_END, "Abort Game",          EV_GOTO_MAIN_MENU,     0,      NULL);
      menuAddItem(inGameMenu, ADD_TO_END, "Verify Board",        EV_START_BOARD_CHECK,  0,      NULL);
   }

   drawMenu(inGameMenu);

}

void inGameMenuExit( event_t ev )
{
   destroyMenu(inGameMenu);
   inGameMenu = NULL;
   displayClear();
}
