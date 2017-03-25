#ifndef MENU_H
#define MENU_H

#include "types.h"
#include "switch.h"
#include "hsm.h"

// Type for picker functions
typedef char*  (*itemValuePicker_t)  ( int dir );   // 0 = return current, 1 = set/return next, -1 = set/return prev.

// Passed as index into menuAddItem to append to end..
#define ADD_TO_END      (-1)

// Attributes of each menu item
typedef struct menuItem_s
{
   char               *text;        // Text for this item.  NULL used to terminate list.

   // PRESS
   int                pressEvent;   // If button pressed, return this value

   // RIGHT
   int                rightEvent;   // If right, return this value.  IGNORED if picker is not NULL

   // RIGHT/LEFT
   itemValuePicker_t  picker;       // modifies/returns value to display right justifed at end of line.

}menuItem_t;


typedef struct menu_s
{
   char       *title;         // Menu title (if any).  If not NULL, this title will always appear on top line of display
                              //  with other three lines used to scroll menu items.  If NULL, all four lines will
                              //  scroll menu items

   uint16_t   backEvent;  // TRUE  = Left button on any item without picker will trigger return of "M_EV_BACK" event,
                              // FALSE = left button ignored on non-picker lines

   int        selectedItem;   // Currently selected item

   int        cursorLine;     // Line which contains currently selected item

   int        itemCount;      // Total items in following list

   menuItem_t *items;         // A list of items.  First item with NULL for the text marks the end of the list

}menu_t;


// Create a new menu
menu_t *createMenu(char *title,        // Optional title.  Use NULL if no title needed
                   uint16_t backEvent);  // true = left button returns M_EV_BACK, false = left button returns M_EV_IGNORED

// Destroy a menu, freeing its space...
void    destroyMenu(menu_t *menu);

// Display the menu (useful when first creating or returning back a sub menu or other process)
void    displayMenu(menu_t *menu);

// Add a menu item to the menu
bool_t menuAddItem(menu_t *menu,             // Menu to add to
                   int offset,               // offset to add to (ADD_TO_END or offset > last item used to append to end)
                   char   *text,             // The text of this item
                   uint16_t    pressEv,           // Value returned on press
                   uint16_t    rightEv,           // Value returned on right (ignored if pick is not NULL)
                   itemValuePicker_t pick);  // Picker function

// Deletes an item from a menu.
bool_t menuDeleteItem(menu_t *menu, char *text);

// Handles button position events for given menu
void menuProcessNavButton(menu_t *menu, event_t button);

// Handles button pos/press events for given menu
void menuProcessButtonPress(menu_t *menu);

// Move selection and displayed line back...
void menuReset(menu_t *menu);

// Using current context, clear the screen and render the menu
void drawMenu(menu_t *menu);

#endif
