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
      menuAddItem(optionMenu, ADD_TO_END, "Exit",  EV_GOTO_MAIN_MENU, EV_GOTO_MAIN_MENU, NULL);
      menuAddItem(optionMenu, ADD_TO_END, "White", 0,                 0,                 optionsMenu_pickWhitePlayer);
      menuAddItem(optionMenu, ADD_TO_END, "Black", 0,                 0,                 optionsMenu_pickBlackPlayer);
   }

   drawMenu( optionMenu );
}

void optionMenuExit( event_t ev )
{
   destroyMenu(optionMenu);
   optionMenu = NULL;
}

char*  optionsMenu_pickWhitePlayer( int dir )   // 0 = return current, 1 = set/return next, -1 = set/return prev.
{

   if( dir == 1 || dir == -1 )
   {
      if(options.game.white == PLAYER_HUMAN )
      {
         options.game.white = PLAYER_COMPUTER;
      }
      else
      {
         options.game.white = PLAYER_HUMAN;
      }
   }

   return (options.game.white == PLAYER_HUMAN ? "Human" : "Computer");
}

char* optionsMenu_pickBlackPlayer( int dir )   // 0 = return current, 1 = set/return next, -1 = set/return prev.
{

   if( dir == 1 || dir == -1 )
   {
      if(options.game.black == PLAYER_HUMAN )
      {
         options.game.black = PLAYER_COMPUTER;
      }
      else
      {
         options.game.black = PLAYER_HUMAN;
      }
   }

   return (options.game.black == PLAYER_HUMAN ? "Human" : "Computer");
}
