#include "types.h"

typedef char *displayLine_t;

typedef struct displayMessage_s
{
   displayLine_t top;
   displayLine_t bottom;
}displayMessage_t;

typedef struct displayMenu_s
{
   displayLine_t  title;
   displayLine_t  entries[];
}displayMenu_t;

typedef enum displayCap_e
{
   CAPS_NONE,
   CAPS_FIRST,
   CAPS_ALL
}displayCap_t;


// Present the given menu (removing disabled items and start at given offset), return selected item.
//   -1 = back selected (i.e. no selection made)
int displayMenuSelect(displayMenu_t *menu, uint32_t disableMask, uint8_t offset);

// Put a message on the display
void displayMessage(displayMessage_t *msg);

// Search for "COLOR" in string, replace either "black" or "white" (with given cap setting)
char *displayColorSub(char *string, color_t color, displayCap_t caps);

// Search for "_PIECE" in string, replace with "pawn", "knight".... (with given cap setting)
char *displayPieceSub(char *string, piece_t piece, displayCap_t caps);


