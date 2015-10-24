#include "types.h"
#include "specChars.h"

#define NUM_LINES            4
#define LINE_LENGTH         20



// Initialize display driver
void displayInit( void );

// Sets the cursor position and enables/disables its display and blink pattern
void displaySetCursor( int line, int col, bool_t cursor, bool_t blink);

// Removes display of cursor and blink pattern
void displayClearCursor( void );

// Clears the display contents
void displayClear( void );

// Clears a single line
void displayClearLine( int line );

// Clears, then writes contents of line
void displayWriteLine( int line, char *data, bool_t centered );

// Modifies only a portion of the line, leaving rest in tact
void displayWriteChars( int line, int offset, int len, char *data );

// Makes a copy of the display and cursor information on display stack
void displayPush( void );

// Restores display to last pushed value
void displayPop( void );

// Defines one of 8 user-programmable characters
void defineCharacter(uint8_t pos, specCharDefn data);

// Rolls display up, new content enters at bottom
void rollUp( char *data, bool_t centered, bool_t keepTopLine);

// Rolls display down, new content enters at top
void rollDown( char *data, bool_t centered, bool_t keepTopLine);

void DEBUG_ShowDisplayContents( void );
