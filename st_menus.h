#include "menu.h"

void menuRegisterButtonEvents(menu_t *menu);

void menusExit( event_t ev);
void menus_centerButton_pressed( event_t ev );
void menus_navButton_pressed( event_t ev );
uint16_t menuPickSubstate(event_t ev);
