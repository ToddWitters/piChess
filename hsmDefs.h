#ifndef HSM_DEFS_H
#define HSM_DEFS_H

#include "hsm.h"

extern stateDef_t myStateDef[];
extern transDef_t myTransDef[];

extern const uint16_t transDefCount;

// NOTE:  Indentation used below for a visual aide...
typedef enum stateId_e
{
   ST_TOP,                         // Top-most containing state
     ST_SPLASH_SCREEN,             // Displaying splash screen
     ST_MENUS,                     // In one of the top menus
       ST_MAINMENU,                // Top-most menu
       ST_DIAGMENU,                // Diagnostic menu
       ST_OPTIONMENU,              // Top level option menu
       ST_BOARD_OPTION_MENU,       // Board option menu
       ST_GAME_OPTION_MENU,        // Game option menu
       ST_ENGINE_OPTION_MENU,      // Engine option menu
     ST_TIME_OPTION_MENU,          // Time option selections.  Not a typical menu...
     ST_INIT_POS_SETUP,            // Set up initial position
     ST_ARB_POS_SETUP,             // User is setting board to an arbitrary position
     ST_IN_GAME,                   // A game is in progress
       ST_PLAYING_GAME,            // Actively making moves (or thinking)
         ST_PLAYER_MOVE,           // Player is moving
         ST_COMPUTER_MOVE,         // Computer is thinking
         ST_MOVE_FOR_COMPUTER,     // Player is making computer's chosen move
       ST_GAMEMENU,                // Navigating in-game menu
       ST_FIX_BOARD,               // Prompting user to restore board to desired position
       ST_CHECK_BOARD,             // User is verifying pieces at occupied squares are correct
       ST_EXITING_GAME,            // Game has concluded, waiting for confirmation
     ST_DIAG_SENSORS,              // Testing reed switches and sensors

   ST_NONE, // MUST BE LAST ITEM IN LIST...
   ST_COUNT = ST_NONE
}stateId_t;


// Events
typedef enum eventId_e
{
   EV_NULL,           // Dummy event.  Value of 0 reserved for use by HSM logic

   // BUTTONS
   EV_BUTTON_NONE,   // All buttons now released
   EV_BUTTON_RIGHT,  // Just the right button is pressed
   EV_BUTTON_LEFT,   // Just the left button is pressed
   EV_BUTTON_UP,     // Just the up button is pressed
   EV_BUTTON_DOWN,   // Just the down button is pressed
   EV_BUTTON_CENTER, // Just the center button is pressed
   EV_BUTTON_CHORD,  // Two or more buttons are pressed (argument determines which ones)

   // PIECE MOVEMENT
   EV_PIECE_DROP,    // A piece has been dropped on an empty square
   EV_PIECE_LIFT,    // A piece has been removed

   // MENU SELECTIONS
   EV_START_SENSOR_DIAG,     // User selected "sensor diagnostics" from menu
   EV_START_INIT_POS_SETUP,  // User selected "play" from top menu
   EV_START_ARB_POS_SETUP,   // User selected "setup board" from top menu
   EV_START_BOARD_CHECK,     // User selected "verify board" from in-game menu

   EV_GOTO_MAIN_MENU,        // User
   EV_GOTO_DIAG_MENU,
   EV_GOTO_OPTION_MENU,
   EV_GOTO_BOARD_OPTIONS,
   EV_GOTO_GAME_OPTIONS,
   EV_GOTO_ENGINE_OPTIONS,
   EV_GOTO_TIME_OPTIONS,

   EV_GOTO_GAME,
   EV_GOTO_PLAYING_GAME,
   EV_GOTO_GAMEMENU,
   EV_GAME_DONE,

   // TIMER EVENTS
   EV_MOVE_CLOCK_TIC,
   EV_UI_BOX_CHECK,

   EV_PROCESS_COMPUTER_MOVE,

   EV_PLAYER_MOVED_FOR_COMP,

   EV_FIX_BOARD,

   EV_TAKEBACK

}eventId_t;

#endif
