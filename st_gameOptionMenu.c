#include "hsm.h"
#include "hsmDefs.h"
#include "st_gameOptionMenu.h"
#include "menu.h"
#include "options.h"
#include "types.h"

#include <stddef.h>

static char* gameOptionsMenu_pickWhitePlayer( int dir );
static char* gameOptionsMenu_pickBlackPlayer( int dir );
static char* gameOptionsMenu_pickBook(int dir);
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
      menuAddItem(gameOptionMenu, ADD_TO_END, "OpeningBook",  0,                   0,                   gameOptionsMenu_pickBook);

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

static char* gameOptionsMenu_pickBlackPlayer( int dir )   // 0 = return current, 1 = set/return next, -1 = set/return prev.
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

static char* gameOptionsMenu_pickBook( int dir )
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

#if 0
static char* gameOptionsMenu_pickTimeSetting(int dir)
{

   if(dir == 1)
   {
      switch(options.game.timeControl.type)
      {
         case TIME_NONE:  options.game.timeControl.type = TIME_EQUAL; break;
         case TIME_EQUAL: options.game.timeControl.type = TIME_ODDS;  break;
         case TIME_ODDS:  options.game.timeControl.type = TIME_NONE;  break;
      }
   }
   else if(dir == -1)
   {
      switch(options.game.timeControl.type)
      {
         case TIME_NONE:  options.game.timeControl.type = TIME_ODDS;  break;
         case TIME_EQUAL: options.game.timeControl.type = TIME_NONE;  break;
         case TIME_ODDS:  options.game.timeControl.type = TIME_EQUAL; break;
      }
   }

   switch(options.game.timeControl.type)
   {

      case TIME_NONE:
         options.game.timeControl.compStrategySetting.type     = STRAT_FIXED_DEPTH;
         options.game.timeControl.compStrategySetting.timeInMs = 16;
         return "None...";
      case TIME_EQUAL:
         options.game.timeControl.timeSettings[0].totalTime = 300;
         options.game.timeControl.timeSettings[0].increment = 0;
         return "Equal...";
      case TIME_ODDS:
         options.game.timeControl.timeSettings[WHITE].totalTime = 300;
         options.game.timeControl.timeSettings[WHITE].increment = 0;
         options.game.timeControl.timeSettings[BLACK].totalTime = 60;
         options.game.timeControl.timeSettings[BLACK].increment = 0;
         return "Odds...";
      default:
         return "???";
   }

}
#endif
