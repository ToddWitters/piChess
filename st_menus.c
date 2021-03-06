#include "hsm.h"
#include "hsmDefs.h"
#include "st_menus.h"

#include <stddef.h>
#include "display.h"
#include "menu.h"

static menu_t *currentMenu = NULL;

uint16_t menuPickSubstate(event_t ev)
{
   return ST_MAINMENU;
}


void menusExit( event_t ev)
{
   displayClear();
   currentMenu = NULL;
}

// Called from drawMenu() function...
void menuRegisterButtonEvents(menu_t *menu)
{
   currentMenu = menu;
}

void menus_centerButton_pressed( event_t ev )
{
   (void)ev;
   menuProcessButtonPress( currentMenu );
}

void menus_navButton_pressed( event_t ev )
{
   menuProcessNavButton( currentMenu, ev );
}
