#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "display.h"
#include "diag.h"
#include "st_menus.h"
#include "event.h"

// Create a new menu
menu_t *createMenu(char *title, uint16_t backEvent)
{

   // Create memory space
   menu_t *ptr = malloc(sizeof(menu_t));

   // Verify valid pointer
   if(ptr == NULL) return NULL;

   if(title != NULL)
   {
      // Create space for title...
      ptr->title = malloc(strlen(title)+1);

      // Verify valid pointer
      if(ptr->title == NULL)
      {
         free(ptr);
         return NULL;
      }

      strcpy(ptr->title, title);
   }
   else
   {
      ptr->title = NULL;
   }

   // Set the other parameters...
   ptr->backEvent = backEvent;
   ptr->selectedItem = 0;
   ptr->cursorLine = strlen(title) == 0 ? 0 : 1;
   ptr->itemCount = 0;
   ptr->items = NULL;

   return ptr;
}

void destroyMenu(menu_t *menu)
{
   int i = 0;

   if(menu == NULL) return;

   if(menu->title != NULL) free(menu->title);

   if(menu->items != NULL)
   {
      while(menu->itemCount--)
      {
         if(menu->items[i].text != NULL) free(menu->items[i].text);
         i++;
      }
      free(menu->items);
   }

   free(menu);
}

// Adds a menu item...
bool_t menuAddItem(menu_t *menu,
                   int offset,
                   char   *text,
                   uint16_t    pressEv,
                   uint16_t    rightEv,
                   itemValuePicker_t pick)
{

   if(menu == NULL) return FALSE;

   if(menu->items == NULL)
   {
      menu->items = malloc(sizeof(menuItem_t));

      if(menu->items == NULL) return FALSE;

      menu->itemCount = 1;
   }
   else
   {
      menu->items = realloc(menu->items, (menu->itemCount + 1) * sizeof(menuItem_t));

      if(menu->items == NULL) return FALSE;

      ++menu->itemCount;

   }

   if(offset == ADD_TO_END) offset = menu->itemCount - 1;

   // If we are inserting before the end of the list, move stuff forward
   if(offset < menu->itemCount - 1)
   {
      memmove(&menu->items[offset+1], &menu->items[offset], ( (menu->itemCount - 1) - offset) * sizeof(menuItem_t));
   }

   // Ensure offset is capped at last item
   if(offset > menu->itemCount - 1)
   {
      offset = menu->itemCount - 1;
   }

   // Allocate space for the string
   menu->items[offset].text = malloc(strlen(text)+1);

   // test malloc results
   if(menu->items[offset].text == NULL) return FALSE;

   // Move all the data in...
   strcpy(menu->items[offset].text, text);

   menu->items[offset].pressEvent = pressEv;
   menu->items[offset].rightEvent = rightEv;
   menu->items[offset].picker     = pick;

   return TRUE;

}


bool_t menuDeleteItem(menu_t *menu, char *text)
{
   int i;
   int numElements;
   bool_t found = FALSE;

   if(menu == NULL) return FALSE;

   if(menu->items == NULL) return FALSE;

   numElements = menu->itemCount;

   while(numElements--)
   {
      if(!strcmp(text, menu->items[i].text))
      {
         memmove(&menu->items[i], &menu->items[i+1], ((menu->itemCount - 1) - i) * sizeof(menuItem_t));
         menu->items = realloc(menu->items, (--menu->itemCount * sizeof(menuItem_t)));
         found = TRUE;
         break;
      }
      i++;
   }
   if(found) return TRUE; else return FALSE;

}

void menuProcessButtonPos(menu_t *menu, buttonPos_t pos)
{

   event_t ev = {0, 0};

   if(menu == NULL) return;

   switch(pos)
   {
      case POS_RIGHT:
         // If the selected item doesn't have a picker, return the event
         if(menu->items[menu->selectedItem].picker == NULL)
         {
            ev.ev = menu->items[menu->selectedItem].rightEvent;
         }
         else
         {
            // Set up a string of blanks to overwrite the current value
            char *blanks = "                    ";

            // Get the length of the string for the current value...
            int oldlen   = strlen(menu->items[menu->selectedItem].picker(0));

            // Point to the new value
            char *option = menu->items[menu->selectedItem].picker(1);

            // Get the length of the new value
            int len      = strlen(option);

            // Write blanks over the old
            displayWriteChars( menu->cursorLine, LINE_LENGTH-oldlen, oldlen, blanks);

            // Fill in the new
            displayWriteChars( menu->cursorLine, LINE_LENGTH-len,    len,    option);
         }
         break;

      case POS_LEFT:
         // If no picker...
         if(menu->items[menu->selectedItem].picker == NULL)
         {
            // .. and this menu allows exit via the left button....
            if(menu->backEvent)
            {
               ev.ev = menu->backEvent;
            }
         }
         else
         {

            // Set up a string of blanks to overwrite the current value
            char *blanks = "                    ";

            // Get the length of the string for the current value...
            int oldlen   = strlen(menu->items[menu->selectedItem].picker(0));

            // Point to the new value
            char *option = menu->items[menu->selectedItem].picker(-1);

            // Get the length of the new value
            int len      = strlen(option);

            // Write blanks over the old
            displayWriteChars( menu->cursorLine, LINE_LENGTH-oldlen, oldlen, blanks);

            // Fill in the new
            displayWriteChars( menu->cursorLine, LINE_LENGTH-len,    len,    option);
         }
         break;

      case POS_UP:

         // If the selected item isn't already the first one..
         if(menu->selectedItem > 0)
         {

            // figure out top line based upon presence of a menu title...
            int topDisplayLine = menu->title == NULL ? 0 : 1;

            // remove the selection indicator...
            displayWriteChars( menu->cursorLine, 0, 1, " " );

            // Move selection up by one
            menu->selectedItem--;

            // If we are already at top, roll next line in...
            if(menu->cursorLine == topDisplayLine)
            {
               char newline[LINE_LENGTH + 1];
               newline[0] = ' ';

               // new line starts at offset 0 to allow for selection character
               strncpy(&newline[1], menu->items[menu->selectedItem].text, 19);

               // roll display down
               rollDown( newline, FALSE, topDisplayLine = 0 ? FALSE : TRUE);

               // if this line has has a picker, display current value...
               if(menu->items[menu->selectedItem].picker != NULL)
               {
                  char *option = menu->items[menu->selectedItem].picker(0);
                  int  len = strlen(option);

                  displayWriteChars( topDisplayLine, LINE_LENGTH-len, len, option);
               }
            }
            else
            {
               // Item is visible... just move cursor
               menu->cursorLine--;
            }

            displayWriteChars( menu->cursorLine, 0, 1, ">" );

         }
         break;

      case POS_DOWN:
         if(menu->selectedItem < menu->itemCount - 1)
         {

            int topDisplayLine = menu->title == NULL ? 0 : 1;

            displayWriteChars( menu->cursorLine, 0, 1, " " );

            menu->selectedItem++;

            if(menu->cursorLine == 3)
            {
               char newline[LINE_LENGTH + 1];
               newline[0] = ' ';

               // new line starts at offset 0 to allow for selection character
               strncpy(&newline[1], menu->items[menu->selectedItem].text, 19);

               // roll display up
               rollUp( newline, FALSE, topDisplayLine = 0 ? FALSE : TRUE);
               if(menu->items[menu->selectedItem].picker != NULL)
               {
                  char *option = menu->items[menu->selectedItem].picker(0);
                  int  len = strlen(option);

                  displayWriteChars( 3, LINE_LENGTH-len, len, option);
               }
            }
            else
            {
               // Item is visible... just move cursor
               menu->cursorLine++;
            }

            displayWriteChars( menu->cursorLine, 0, 1, ">" );
         }
         break;

      default:
         break;
   }
   if(ev.ev != 0)
   {
      putEvent(EVQ_EVENT_MANAGER, &ev);
   }

}

void menuProcessButtonPress(menu_t *menu)
{

   if(menu != NULL)
   {
      if(menu->items[menu->selectedItem].pressEvent != 0);
      {
         event_t ev;
         ev.ev = menu->items[menu->selectedItem].pressEvent;
         ev.data = 0;
         if(ev.ev !=0) putEvent(EVQ_EVENT_MANAGER, &ev);
      }
   }
}

void drawMenu(menu_t *menu)
{

   int limit = NUM_LINES;
   int i;

   //
   // Sanity Checks
   //
   //

   if(menu == NULL)
   {
      DPRINT("ERROR: Trying to draw a menu from a NULL pointer\n");
      return;
   }

   if(menu->itemCount == 0)
   {
      DPRINT("ERROR: Trying to draw a menu with no items\n");
      return;
   }

   if(menu->selectedItem >= menu->itemCount)
   {
      DPRINT("Warning: Invalid selection index when drawing menu.  Setting back to zero\n");
      menu->selectedItem = 0;
   }

   int topDisplayLine = ( (menu->title == NULL) ? 0 : 1);

   if(menu->cursorLine > (NUM_LINES - 1) || menu->cursorLine < topDisplayLine)
   {
      DPRINT("Warning: Invalid cursorLine range when drawing menu.  Setting back to zero\n");
      menu->cursorLine = 0;
   }

   if(topDisplayLine == 0)
   {
      if(menu->cursorLine > menu->selectedItem)
      {
         DPRINT("Warning: Invalid cursorLine when drawing menu.  Setting back\n");
         menu->cursorLine = menu->selectedItem;
      }
   }
   else
   {
      if(menu->cursorLine > menu->selectedItem + 1)
      {
         DPRINT("Warning: Invalid cursorLine when drawing menu.  Setting back\n");
         menu->cursorLine = menu->selectedItem;
      }
   }

   // Start by clearing the display
   displayClear();

   menuRegisterButtonEvents(menu);

   // Display the title centered
   if(menu->title != NULL) displayWriteLine(0, menu->title, TRUE);

   // Set the line limit for our display loop if number of items is small
   if(menu->itemCount < (NUM_LINES - topDisplayLine))
   {
      limit = menu->itemCount + topDisplayLine;
   }

   // Loop from first menu item line to our limit
   for(i=topDisplayLine; i<limit; i++)
   {
      // since we need this offset several times below, compute and store once here...
      int itemOffset = menu->selectedItem - (menu->cursorLine - 1) + (i-topDisplayLine);

      // Create a new line for holding the shifted menu item
      char newline[LINE_LENGTH + 1];

      newline[0] = ' ';

      // new line starts at offset 1 to allow for selection character
      strncpy(&newline[1], menu->items[itemOffset].text, 19);

      // Display this line
      displayWriteLine(i, newline, FALSE);

      // If there is a picker for this line, display the current setting
      if(menu->items[itemOffset].picker != NULL)
      {
         char *option = menu->items[itemOffset].picker(0);
         int  len = strlen(option);

         displayWriteChars( i, LINE_LENGTH-len, len, option);
      }
   }
   displayWriteChars( menu->cursorLine, 0, 1, ">" );
}

void menuReset(menu_t *menu)
{
   menu->selectedItem = 0;
   menu->cursorLine = ( ( menu->title == NULL) ? 0 : 1);
}
