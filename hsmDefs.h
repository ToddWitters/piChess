#ifndef HSM_DEFS_H
#define HSM_DEFS_H

typedef enum stateId_e
{
   ST_TOP,                // This must be the top-most state
     ST_SPLASH_SCREEN,
     ST_MAIN_MENU,
     ST_IN_GAME,
       ST_PLAYER_MOVE,
       ST_COMPUTER_MOVE,
       ST_MAKE_COMP_MOVE,
       ST_IN_GAME_MENU,
     ST_GAME_CONCLUSION,
     ST_INIT_POS_SETUP,
     ST_CUST_POS_SETUP,

   ST_COUNT,
   ST_NONE = ST_COUNT
}stateId_t;


// Events
typedef enum eventId_e
{

   // TIME BASED
   EV_TIMEOUT = 1,         // NOTE:  Zero is reserved value passed to entry functions
                           //        on initialization...
   EV_MOVE_CLOCK_TICK,

   // ENGINE
   EV_ENGINE_PICK,

   // BUTTONS
   EV_BUTTON_STATE,
   EV_BUTTON_POS,

   // MENU ACTIONS
   EV_START_GAME,

   // PIECE MOVEMENT
   EV_PIECE_DROP,
   EV_PIECE_LIFT,
   EV_PLAYER_MOVED,
   EV_PLAYER_COMP_MOVE_DONE,

   EV_GAME_END,

}eventId_t;

#endif
