#include "hsm.h"
#include "hsmDefs.h"
#include "st_gameOptionMenu.h"
#include "menu.h"
#include "options.h"
#include "types.h"

#include <stddef.h>

static char* gameOptionsMenu_pickWhitePlayer( int dir );
static char* gameOptionsMenu_pickBlackPlayer( int dir );
static char* gameOptionsMenu_pickCoaching(int dir);
static char* gameOptionsMenu_pickTakeback(int dir);
// static char* gameOptionsMenu_pickTimeSetting(int dir);

menu_t *gameOptionMenu = NULL;

void gameOptionMenuEntry( event_t ev )
{

   if(gameOptionMenu == NULL)
   {
      gameOptionMenu = createMenu("----Game Options----", EV_GOTO_OPTION_MENU);

      menuAddItem(gameOptionMenu, ADD_TO_END, "Go Back",      EV_GOTO_OPTION_MENU, EV_GOTO_OPTION_MENU, NULL);
      menuAddItem(gameOptionMenu, ADD_TO_END, "White",        0,                   0,                   gameOptionsMenu_pickWhitePlayer);
      menuAddItem(gameOptionMenu, ADD_TO_END, "Black",        0,                   0,                   gameOptionsMenu_pickBlackPlayer);
      menuAddItem(gameOptionMenu, ADD_TO_END, "Clocks...",    EV_GOTO_TIME_OPTIONS,      0,                   NULL);
      menuAddItem(gameOptionMenu, ADD_TO_END, "Coaching",     0,                   0,                   gameOptionsMenu_pickCoaching);
      menuAddItem(gameOptionMenu, ADD_TO_END, "Takeback",     0,                   0,                   gameOptionsMenu_pickTakeback);

   }

   drawMenu(gameOptionMenu);
}

void gameOptionMenuExit( event_t ev )
{
   destroyMenu(gameOptionMenu);
   gameOptionMenu = NULL;

}

static char* gameOptionsMenu_pickWhitePlayer( int dir )   // 0 = return current, 1 = set/return next, -1 = set/return prev.
{

   if( dir == 1 || dir == -1 )
   {
      if(isOptionStr("whitePlayer", "human"))
      {
         setOptionStr("whitePlayer", "computer");
      }
      else
      {
         setOptionStr("whitePlayer", "human");
      }
   }

   return getOptionStr("whitePlayer");
}

static char* gameOptionsMenu_pickBlackPlayer( int dir )   // 0 = return current, 1 = set/return next, -1 = set/return prev.
{

   if( dir == 1 || dir == -1 )
   {
      if(isOptionStr("blackPlayer", "human"))
      {
         setOptionStr("blackPlayer", "computer");
      }
      else
      {
         setOptionStr("blackPlayer", "human");
      }
   }

   return getOptionStr("blackPlayer");
}

static char* gameOptionsMenu_pickCoaching(int dir)
{
   if( dir == 1 || dir == -1 )
   {
      if( isOptionStr("coaching", "true") )
      {
         setOptionStr("coaching", "false");
      }
      else
      {
         setOptionStr("coaching", "true");
      }
   }

   return ( isOptionStr("coaching", "true") ? "On" : "Off");

}

static char* gameOptionsMenu_pickTakeback(int dir)
{
   if( dir == 1 || dir == -1 )
   {
      if(isOptionStr("takeBack", "true"))
      {
         setOptionStr("takeBack", "false");
      }
      else
      {
         setOptionStr("takeBack", "true");
      }
   }

   return ( isOptionStr("takeBack", "true") ? "On" : "Off");

}

