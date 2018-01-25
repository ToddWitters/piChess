// Display handler.
// NOTE:  This driver favors execution speed over code size, hence the many macros

#include <string.h>

#include "bcm2835.h"
#include "diag.h"
#include "display.h"
#include "types.h"
#include "i2c.h"
#include "gpio.h"
#include "specChars.h"
#include "hsm.h"

// Some display parameters
#define DISPLAY_STACK_DEPTH 10

// GPIO EXPANDER port addresses
#define DATA_PORT_REG_ADDR     GPIOA_ADDR
#define CTRL_PORT_REG_ADDR     GPIOB_ADDR

// Address in the display controller for the given lines
#define ROW_1_ADDR 0x00
#define ROW_2_ADDR 0x40
#define ROW_3_ADDR 0x14
#define ROW_4_ADDR 0x54

// Setup MACROS to prepare state of control pins
// These simply modifiy a local variable in prep for sending this to the display
#define SET_E       (controlBits |= E_MASK)
#define CLR_E       (controlBits &= ~E_MASK)
#define SET_RS      (controlBits |= RS_MASK)
#define CLR_RS      (controlBits &= ~RS_MASK)
#define SET_RW      (controlBits |= RW_MASK)
#define CLR_RW      (controlBits &= ~RW_MASK)
#define SET_DATA(X) (dataByte = X)

// Send control or data out via i2c
// This actually sends the data to the gpio contoller which in turn presents the port values to the display driver
#define FLUSH_CONTROL \
   i2c_command[0] = CTRL_PORT_REG_ADDR; \
   i2c_command[1] = controlBits; \
   i2cSendCommand(GPIO_EXPANDER_UI_ADDR, i2c_command, 2);

#define FLUSH_DATA  \
   i2c_command[0] = DATA_PORT_REG_ADDR; \
   i2c_command[1] = dataByte; \
   i2cSendCommand(GPIO_EXPANDER_UI_ADDR, i2c_command, 2);

// Read the ready bit (and DDRAM address) into dataByte
#define READ_DATA \
  i2c_command[0] = DATA_PORT_REG_ADDR; \
  i2cSendReceive( GPIO_EXPANDER_UI_ADDR, i2c_command, 1, &dataByte, 1);

// Display command bytes and bit positions of their parameters
#define CLEAR_DISP       0x01
#define RETURN_HOME      0x02
#define CMD_ENTRY_MODE   0x04
 #define BIT_ID              1
  #define ID_INCREMENT        1
  #define ID_DECREMENT        0
 #define BIT_S               0
  #define S_SHIFT             1
  #define S_NOSHIFT           0
#define CMD_DISPLAY_CTRL 0x08
 #define BIT_D               2
  #define D_DISPLAY_ON        1
  #define D_DISPLAY_OFF       0
 #define BIT_C               1
  #define C_CURSOR_ON         1
  #define C_CURSOR_OFF        0
 #define BIT_B               0
  #define B_BLINK_ON          1
  #define B_BLINK_OFF         0
#define CMD_SHIFT_CTRL   0x10
 #define BIT_SC              3
  #define SC_DISP_SHIFT       1
  #define SC_CURSOR_SHIFT     0
 #define BIT_RL              2
  #define RL_SHIFTRIGHT       1
  #define RL_SHIFTLEFT        0
#define CMD_FUNC_SET     0x20
 #define BIT_DL              4
  #define DL_8_BITS           1
  #define DL_4_BITS           0
 #define BIT_N               3
  #define N_2_LINES           1
  #define N_1_LINE            0
 #define BIT_F               2
  #define F_5x10              1
  #define F_5x7               0
#define CMD_SET_CG_ADDR  0x40
#define CMD_SET_DD_ADDR  0x80

// Busy flag position
#define BUSY_FLAG_MASK   0x80

// Constructing Command bytes
// NOTE!  The following macros assume the parameters are zero or one
#define ENTRY_MODE(ID,S)    ( CMD_ENTRY_MODE   | ( ID << BIT_ID ) | (  S << BIT_S ) )
#define DISPLAY_CTRL(D,C,B) ( CMD_DISPLAY_CTRL | (  D << BIT_D  ) | (  C << BIT_C ) | ( B << BIT_B ) )
#define SHIFT_CTRL(SC,RL)   ( CMD_SHIFT_CTRL   | ( SC << BIT_SC ) | ( RL << BIT_RL) )
#define FUNC_SET(DL,N,F)    ( CMD_FUNC_SET     | ( DL << BIT_DL ) | (  N << BIT_N ) | ( F << BIT_F ) )

// Set the CG or DD address.
#define SET_CG_ADDR(ADDR)   ( CMD_SET_CG_ADDR  | (ADDR) )
#define SET_DD_ADDR(ADDR)   ( CMD_SET_DD_ADDR  | (ADDR) )

// Manipulation of RS pin
#define RAISE_RS  SET_RS; FLUSH_CONTROL;
#define LOWER_RS  CLR_RS; FLUSH_CONTROL;

// Manipulation of RW pin
#define RAISE_RW  SET_RW; FLUSH_CONTROL;
#define LOWER_RW  CLR_RW; FLUSH_CONTROL;

// Manipulation of E pin
#define RAISE_E   SET_E; FLUSH_CONTROL;
#define LOWER_E   CLR_E; FLUSH_CONTROL;

// Manipulation of data lines
#define WRITE_DISPLAY_DATA(X) \
   SET_DATA(X);\
   FLUSH_DATA;

// Macro to clock out data once RS and RW have been set properly
#define SEND(X) \
   RAISE_E; \
   WRITE_DISPLAY_DATA(X); \
   LOWER_E;

// Macro to setup the DDRAM address.  Assumes RS and RW are low
#define SETUP_DD_ADDR(ADDR)   SEND(SET_DD_ADDR(ADDR));

#define MAX_RETRIES_FOR_BUSY 10

// Macro to check for display busy (with max # retries)
// Assumes RS is already low...
// Note that Data lines must be changed inputs, and back to outputs after done...
#define WAIT_TILL_READY                                                                   \
   {                                                                                      \
      uint8_t retryCounter = MAX_RETRIES_FOR_BUSY;                                        \
      uint8_t command[2];                                                                 \
      command[0] = IODIRA_ADDR;                                                           \
      command[1] = 0xFF;                                                                  \
      i2cSendCommand(GPIO_EXPANDER_UI_ADDR, command, 2);                                  \
      do {                                                                                \
         RAISE_RW;                                                                        \
         RAISE_E;                                                                         \
         READ_DATA;                                                                       \
         LOWER_E;                                                                         \
         LOWER_RW;                                                                        \
      }while( (dataByte & BUSY_FLAG_MASK) && retryCounter--);                             \
      command[0] = IODIRA_ADDR;                                                           \
      command[1] = 0x00;                                                                  \
      i2cSendCommand(GPIO_EXPANDER_UI_ADDR, command, 2);                                  \
      if(dataByte & BUSY_FLAG_MASK) DPRINT("Display failed to clear busy flag\n");        \
   }


// Current state of control bits (only uses bits 7-5)
uint8_t controlBits = 0;

// Current data byte
uint8_t dataByte = 0;

// Command buffer
uint8_t i2c_command[2];

// The insane mapping of the display register offsets of the 20 bytes of data in each row...
const uint8_t rowOffset[4] = { 0x00, 0x40, 0x14, 0x54 };

// Cursor parameters
typedef struct cursorInfo_s
{
   bool_t on;      // None of the other fields are valid if this is false
   bool_t blink;   // Blink the cursor
   int    line;    // Display line (0-3)
   int    col;     // Display col (0-19)
}cursorInfo_t;

cursorInfo_t cursor;

// This allows us to copy/restore a display with the cursor information
typedef struct display_s
{
   char          contents[NUM_LINES][LINE_LENGTH];
   cursorInfo_t  cursor;
}display_t;

// Current display contents
display_t display;

// Stacked display contents..
display_t displayStack[DISPLAY_STACK_DEPTH];

// Offset into display stack
int displayStackOffset = 0;

// Local helper functions
static void sendString( char *data, uint8_t line, uint8_t col, uint8_t len );

// Initialize parameters of display
// NOTE:  Assumes R/S and R/W are already low
void displayInit( void )
{

   DPRINT("Initializing Display\n");
   WAIT_TILL_READY;
   SEND(ENTRY_MODE(ID_INCREMENT, S_NOSHIFT)); // Increment, no display shift on entry
   SEND(FUNC_SET(DL_8_BITS, N_2_LINES, F_5x7)); // Data len 8, 2 "lines", 5x7 characters
   displayClearCursor();  // Shut off cursor
}

// Set the cursor at given position and blink if required
void displaySetCursor( int line, int col, bool_t cursor, bool_t blink)
{

   display.cursor.on    = cursor;
   display.cursor.blink = blink;
   display.cursor.line  = line;
   display.cursor.col   = col;

   WAIT_TILL_READY;

   SETUP_DD_ADDR(rowOffset[line] + col);

   SEND(DISPLAY_CTRL(D_DISPLAY_ON ,
                     cursor == TRUE ? C_CURSOR_ON : C_CURSOR_OFF ,
                     blink  == TRUE ? B_BLINK_ON  : B_BLINK_OFF ));
}

void displayClearCursor()
{
   displaySetCursor( display.cursor.line, display.cursor.col, FALSE, FALSE);
}

// Clear display contents to all spaces
void displayClear( void  )
{
    displayClearCursor();
    memset(&display.contents, ' ', NUM_LINES * LINE_LENGTH);
    WAIT_TILL_READY;
    SEND(CLEAR_DISP);
}

void displayClearLine( int line )
{
   // verify passed parameter
   if( line >= NUM_LINES)
   {
      DPRINT("Invalid line number passed to displayClearLine\n");
      return;
   }

   // Set desired line contents to all spaces
   memset(display.contents[line], ' ', LINE_LENGTH);

   sendString(&display.contents[line][0], line, 0, LINE_LENGTH);

}

// Write text to a display line, optionally centering it
void displayWriteLine( int line, char *data, bool_t centered)
{

    int offset = 0;
    int len;

    if( line >= NUM_LINES)
    {
      DPRINT("Invalid line number passed to displayWriteLine\n");
      return;
    }

    // Start with line at all spaces
    memset(display.contents[line], ' ', LINE_LENGTH);

    // On Null pointer, nothing more to do...
    if(data != NULL)
    {

       // Count the length (minus null terminator)
       len = strlen(data);

       // truncate if too long
       if (len > LINE_LENGTH) len = LINE_LENGTH;

       // Adjust offset for centered lines
       if(centered == TRUE)
       {
          offset = ( LINE_LENGTH / 2 ) - ( (len + 1) / 2 );
       }

       // NOTE:  We don't want to copy closing null terminator
       strncpy(&display.contents[line][offset], data, len);
    }

    sendString( &display.contents[line][0], line, 0, LINE_LENGTH);

}

// Write to line at given offset without overwriting existing contents.
void displayWriteChars( int line, int offset, int len, char *data)
{

   if( line >= NUM_LINES )
   {
      DPRINT("Invalid line number passed to displayWriteChars\n");
      return;
   }

   if( data == NULL )
   {
      DPRINT("NULL data pointer passed to displayWriteChars\n");
      return;
   }

   if( offset >= LINE_LENGTH)
   {
      DPRINT("Inavlid offset passed to displayWriteChars\n");
      return;
   }


   if( offset + len > LINE_LENGTH)
   {
      DPRINT("Warning: truncating line passed to displayWriteChars\n");
      len = LINE_LENGTH - offset;
   }

   memcpy(&display.contents[line][offset], data, len);

   sendString( &display.contents[line][offset], line, offset, len);

}

// Save contents of display and cursor state
void displayPush( void )
{
   if( displayStackOffset < (DISPLAY_STACK_DEPTH - 1) )
   {
      memcpy(&displayStack[displayStackOffset++], &display, sizeof(display_t));
   }
   else
   {
      DPRINT("ERROR: Display stack full\n");
   }
}

// Restore last display from stack
void displayPop( void )
{
   int i;

   if( displayStackOffset )
   {
      memcpy(&display, &displayStack[--displayStackOffset], sizeof(display_t));

      for(i=0; i < NUM_LINES; i++)
      {
         sendString( &display.contents[i][0], i, 0, LINE_LENGTH);
      }

      displaySetCursor(display.cursor.line, display.cursor.col, display.cursor.on, display.cursor.blink);
   }

   else
   {
      DPRINT("ERROR: No stacked display to pop\n");
   }
}

// Send character stream of given length to display on given line (zero based) and given col

// Assumes necessary parameter checks already made on passed data.
static void sendString( char *data, uint8_t line, uint8_t col, uint8_t len )
{

      char lineContents[21];
      int i;

      memset(lineContents, 0x00, 21);
      strncpy(lineContents, data, len);

      WAIT_TILL_READY;

      SETUP_DD_ADDR(rowOffset[line] + col);

      RAISE_RS;

      for(i=0;i<len;i++)
      {
         SEND(*data++);
      }
      LOWER_RS;

      // Restore cursor position
      displaySetCursor(display.cursor.line, display.cursor.col, display.cursor.on, display.cursor.blink);
}


// Creates user-defined character with ASCII value "pos", defined by bits in data
void defineCharacter(uint8_t pos, specCharDefn data)
{

   int i;

   if(pos >= 8)
   {
      DPRINT("ERROR:  invalid pos parameter in function defineCharacter\n");
      return;
   }

   SEND(SET_CG_ADDR(pos << 3));

   RAISE_RS;

   for(i=0;i<8;i++)
   {
      SEND((*data)[i]);
   }

   LOWER_RS;

}


void rollUp( char *data, bool_t centered, bool_t keepTopLine)
{

   if(data == NULL)
   {
      DPRINT("Null passed as data pointer into rollUp function");
      return;
   }

   if(keepTopLine)
   {
      // Move lines 2,3 to 1,2
      memmove(&display.contents[1][0], &display.contents[2][0], LINE_LENGTH * (NUM_LINES - 2) );

      sendString( &display.contents[1][0], 1, 0, LINE_LENGTH );
      sendString( &display.contents[2][0], 2, 0, LINE_LENGTH );
   }
   else
   {
      // Move lines 1,2,3 to 0,1,2
      memmove(&display.contents[0][0], &display.contents[1][0], LINE_LENGTH * (NUM_LINES - 1) );

      sendString( &display.contents[0][0], 0, 0, LINE_LENGTH );
      sendString( &display.contents[1][0], 1, 0, LINE_LENGTH );
      sendString( &display.contents[2][0], 2, 0, LINE_LENGTH );

   }

   // New data goes in line 4
   displayWriteLine(3, data, centered);

}

void rollDown( char *data, bool_t centered, bool_t keepTopLine)
{

   if(data == NULL)
   {
      DPRINT("Null passed as data pointer into rollUp function");
      return;
   }

   if(keepTopLine)
   {
      // Move lines 1,2 to 2,3
      memmove(&display.contents[2][0], &display.contents[1][0], LINE_LENGTH * (NUM_LINES - 2) );

      // New data goes in line 1
      displayWriteLine(1, data, centered);

      sendString( &display.contents[2][0], 2, 0, LINE_LENGTH );
      sendString( &display.contents[3][0], 3, 0, LINE_LENGTH );

   }

   else
   {
      // Move lines 0,1,2 to 1,2,3
      memmove(&display.contents[1][0], &display.contents[0][0], LINE_LENGTH * (NUM_LINES - 1) );

      // New data goes in line 0
      displayWriteLine(0, data, centered);

      sendString( &display.contents[1][0], 1, 0, LINE_LENGTH );
      sendString( &display.contents[2][0], 2, 0, LINE_LENGTH );
      sendString( &display.contents[3][0], 3, 0, LINE_LENGTH );

   }
}

// Prints the contents of the display structure to the debug console.
void DEBUG_ShowDisplayContents( void )
{
      char lineContents[21];
      int i;


      lineContents[20] = 0x00;

      for(i=0;i<4;i++)
      {
         strncpy(lineContents, &display.contents[i][0], 20);
         DPRINT("[%s]\n", lineContents);
      }

}

void checkDisplay( event_t ev )
{

   uint8_t cmd = IODIRB_ADDR;
   uint8_t rsp;
   int i;

   // If we can communicate with GPIO Expander...
   if( i2cSendReceive( GPIO_EXPANDER_UI_ADDR, &cmd, 1, &rsp, 1) == BCM2835_I2C_REASON_OK)
   {
       // If Configuration is not correct...
      if(rsp != 0x1F)
      {
         // re-init the GPIO expander...
         uiBoxInit();

         // re-init the displpay....
         WAIT_TILL_READY;
         SEND(ENTRY_MODE(ID_INCREMENT, S_NOSHIFT)); // Increment, no display shift on entry
         SEND(FUNC_SET(DL_8_BITS, N_2_LINES, F_5x7)); // Data len 8, 2 "lines", 5x7 characters

         // Refresh the contents...
         for(i=0; i < NUM_LINES; i++) sendString( &display.contents[i][0], i, 0, LINE_LENGTH);
         displaySetCursor(display.cursor.line, display.cursor.col, display.cursor.on, display.cursor.blink);
      }
   }
}
