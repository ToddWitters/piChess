#ifndef HSM_DEFS_H
#define HSM_DEFS_H

extern stateDef_t myStateDef[];
extern transDef_t myTransDef[];

extern const uint16_t transDefCount;

typedef enum stateId_e
{
   ST_TOP,                         // Top-most containing state
     ST_SPLASH_SCREEN,             // Displaying splash screen
     ST_MENUS,                     // In one of the top menus
       ST_MAINMENU,                // Top-most menu
       ST_DIAGMENU,                // Diagnostic menu
       ST_OPTIONMENU,
     ST_INIT_POS_SETUP,            // Set up initial position
     ST_IN_GAME,                   // A game is in progress
       ST_PLAYING_GAME,            // Actively making moves (or thinking)
         ST_PLAYER_MOVE,           // Player is moving
         ST_COMPUTER_MOVE,         // Computer is thinking
         ST_MOVE_FOR_COMPUTER,     // Player is making computer's chosen move
       ST_GAMEMENU,                // Navigating in-game menu
/*
       ST_FIXING_BOARD,            // Player(s) are being guided to fix the board position
*/
       ST_EXITING_GAME,             // Game has concluded, waiting for confirmation

     ST_DIAG_SENSORS,

   ST_COUNT,
   ST_NONE = ST_COUNT
}stateId_t;


// Events
typedef enum eventId_e
{
   EV_RESERVED, // NOTE:  Zero is reserved value passed to entry functions on init
   EV_NULL = EV_RESERVED,

   // BUTTONS
   EV_BUTTON_STATE,
   EV_BUTTON_POS,

   // PIECE MOVEMENT
   EV_PIECE_DROP,
   EV_PIECE_LIFT,

   // MENU SELECTIONS
   EV_START_SENSOR_DIAG,
   EV_START_INIT_POS_SETUP,
   EV_GOTO_MAIN_MENU,
   EV_GOTO_DIAG_MENU,
   EV_GOTO_OPTION_MENU,
   EV_GOTO_GAME,
   EV_GOTO_PLAYING_GAME,
   EV_GOTO_GAMEMENU,

   //
   EV_GAME_DONE,

   // TIMER EVENTS
   EV_MOVE_CLOCK_TIC,

}eventId_t;

#endif
