#include "hsm.h"
#include "hsmDefs.h"
#include "stdio.h"

const char *eventName[] =
{
   "",
   "Timeout",
   "MoveClockTick",
   "EnginePick",
   "ButtonStateChange",
   "ButtonPositionChange",
   "StartGame",
   "PieceDrop",
   "PieceLift",
   "PlayerMoved",
   "PlayerComputerMoveDone",
   "GameEnd"
};


///////////////////
// USER DEFINITION OF STATE MACHINE
///////////////////

// FUNCTION PROTOTYPES

// entry Functions
// <stateName>Entry
extern void topEntry( event_t ev );
extern void splashScreenEntry( event_t ev );
extern void inGameEntry( event_t ev );
extern void mainMenuEntry( event_t ev);
extern void playerMoveEntry( event_t ev);
extern void computerMoveEntry( event_t ev);

// exit Functions
// <stateName>Exit
extern void splashScreenExit( event_t ev );
extern void mainMenuExit( event_t ev);


// pickerFunctions
// <stateName>PickSubstate
extern uint16_t topPickSubstate( event_t ev);
extern uint16_t inGamePickSubstate( event_t ev);

// action functions
// <stateName><eventName>Action
extern void clearScreen( event_t ev );
extern void processDropInGame( event_t ev);
extern void processMenuPress( event_t ev);
extern void processLiftInGame( event_t ev);
extern void pickUnderPromote( event_t ev);
extern void startGame( event_t ev);
extern void processEngineMove( event_t ev);

// guard funtions
// <stateName><eventName>Guard
extern bool_t whiteIsHuman( event_t ev );
extern bool_t isStatePress( event_t ev );
extern bool_t canUnderPromote( event_t ev);
extern bool_t nextMoveIsPlayer( event_t ev);


// State definitions...

// This array must be the same size as the enum above an must
//   define the states in the same order
stateDef_t myStateDef[] =
{

//   displayName       parent      substatePicker              entry               exit
   { "top",            ST_NONE,    topPickSubstate,            topEntry,           NULL_EXIT_FUNC    }, // ST_TOP
   { "splashScreen",   ST_TOP,     NULL_SUBSTATE_PICKER_FUNC,  splashScreenEntry,  splashScreenExit  }, // ST_SPLASH_SCREEN
   { "mainMenu",       ST_TOP,     NULL_SUBSTATE_PICKER_FUNC,  mainMenuEntry,      mainMenuExit      }, // ST_MAIN_MENU
   { "inGame",         ST_TOP,     inGamePickSubstate,         inGameEntry,        NULL_EXIT_FUNC    }, // ST_IN_GAME
   { "playerMove",     ST_IN_GAME, NULL_SUBSTATE_PICKER_FUNC,  playerMoveEntry,    NULL_EXIT_FUNC    }, // ST_PLAYER_MOVE
   { "computerMove",   ST_IN_GAME, NULL_SUBSTATE_PICKER_FUNC,  computerMoveEntry,  NULL_EXIT_FUNC    }, // ST_COMPUTER_MOVE
   { "makeCompMove",   ST_IN_GAME, NULL_SUBSTATE_PICKER_FUNC,  NULL_ENTRY_FUNC,    NULL_EXIT_FUNC    }, // ST_MAKE_COMP_MOVE
   { "inGameMenu",     ST_IN_GAME, NULL_SUBSTATE_PICKER_FUNC,  NULL_ENTRY_FUNC,    NULL_EXIT_FUNC    }, // ST_IN_GAME_MENU
   { "gameConclusion", ST_TOP,     NULL_SUBSTATE_PICKER_FUNC,  NULL_ENTRY_FUNC,    NULL_EXIT_FUNC    }, // ST_GAME_CONCLUSION
   { "initPosSetup",   ST_TOP,     NULL_SUBSTATE_PICKER_FUNC,  NULL_ENTRY_FUNC,    NULL_EXIT_FUNC    }, // ST_INIT_POS_SETUP
   { "custPosSetup",   ST_TOP,     NULL_SUBSTATE_PICKER_FUNC,  NULL_ENTRY_FUNC,    NULL_EXIT_FUNC    }, // ST_CUST_POS_SETUP

//   { "stateName",      ST_<PARENT>,statename_SUBSTATE_PICKER,  statename_ENTRY,    statename_EXIT    },

};

// Transition definitions...

// This list is sorted by event for quicker lookup (may be more than one entry per event)
//   Special case:  Internal events are designated by making the "to" target
//   equal to NULL_STATE_ID.  This causes no transitions and no entry/exit
//   functions to run.

transDef_t myTransDef[] =
{
//   event            from               to                  guard              action             local?
   { EV_TIMEOUT,      ST_SPLASH_SCREEN,  ST_MAIN_MENU,       NULL_GUARD_FUNC,   NULL_ACTION_FUNC,  false },

   { EV_ENGINE_PICK,  ST_IN_GAME,        ST_NONE,            NULL_GUARD_FUNC,   processEngineMove, false },

//   { EV_BUTTON_STATE, ST_MAIN_MENU,      ST_NONE,            isStatePress,      processMenuPress,  false },
   { EV_BUTTON_STATE, ST_MAIN_MENU,      ST_IN_GAME,         isStatePress,      processMenuPress,  false },
   { EV_BUTTON_STATE, ST_IN_GAME_MENU,   ST_NONE,            isStatePress,      processMenuPress,  false },
   { EV_BUTTON_STATE, ST_SPLASH_SCREEN,  ST_MAIN_MENU,       isStatePress,      NULL_ACTION_FUNC,  false },
   { EV_BUTTON_STATE, ST_PLAYER_MOVE,    ST_NONE,            canUnderPromote,   pickUnderPromote,  false },
   { EV_BUTTON_STATE, ST_PLAYER_MOVE,    ST_IN_GAME_MENU,    isStatePress,      NULL_ACTION_FUNC,  false },

   { EV_START_GAME,   ST_MAIN_MENU,      ST_IN_GAME,         whiteIsHuman,      startGame,         false },
   { EV_START_GAME,   ST_MAIN_MENU,      ST_COMPUTER_MOVE,   NULL_GUARD_FUNC,   startGame,         false },

   { EV_PIECE_DROP,   ST_IN_GAME,        ST_NONE,            NULL_GUARD_FUNC,   processDropInGame, false },

   { EV_PIECE_LIFT,   ST_IN_GAME,        ST_NONE,            NULL_GUARD_FUNC,   processLiftInGame, false },

   { EV_PLAYER_MOVED, ST_PLAYER_MOVE,    ST_PLAYER_MOVE,     nextMoveIsPlayer,  NULL_ACTION_FUNC,  false },
   { EV_PLAYER_MOVED, ST_PLAYER_MOVE,    ST_COMPUTER_MOVE,   NULL_GUARD_FUNC,   NULL_ACTION_FUNC,  false },

   { EV_GAME_END,     ST_PLAYER_MOVE,    ST_GAME_CONCLUSION, NULL_GUARD_FUNC,   NULL_ACTION_FUNC,  false },
   { EV_GAME_END,     ST_MAKE_COMP_MOVE, ST_GAME_CONCLUSION, NULL_GUARD_FUNC,   NULL_ACTION_FUNC,  false },

   // { EV_EVNT, ST_FROM, ST_TO, fromState_evnt_GUARD_xxx, fromState_evnt_ACTION_xxx, false},

};

#define ENTRY printf("    -> Entering user function %s\n", __FUNCTION__);
#define EXIT  printf("    -> Leaving %s\n", __FUNCTION__);


void topEntry( event_t ev )
{
   ENTRY;
}

void splashScreenEntry( event_t ev )
{
   ENTRY;
}
void inGameEntry( event_t ev )
{
   ENTRY;
}
void splashScreenExit( event_t ev )
{
   ENTRY;
}
uint16_t topPickSubstate( event_t ev)
{
   ENTRY;
   return ST_SPLASH_SCREEN;
}
uint16_t inGamePickSubstate( event_t ev)
{
   ENTRY;
   return ST_PLAYER_MOVE;
}
void clearScreen( event_t ev )
{
   ENTRY;
}
void processDropInGame( event_t ev)
{
   ENTRY;
}
void processMenuPress( event_t ev)
{
   ENTRY;
}
void processLiftInGame( event_t ev)
{
   ENTRY;
}
void pickUnderPromote( event_t ev)
{
   ENTRY;
}
void startGame( event_t ev)
{
   ENTRY;
}
void processEngineMove( event_t ev)
{
   ENTRY;
}
bool_t whiteIsHuman( event_t ev )
{
   ENTRY;
   return false;
}
bool_t isStatePress( event_t ev )
{
   ENTRY;
   return true;
}
bool_t canUnderPromote( event_t ev)
{
   ENTRY;
   return true;
}
bool_t nextMoveIsPlayer( event_t ev)
{
   ENTRY;
   return true;
}

void mainMenuEntry( event_t ev)
{
   ENTRY;
}

void mainMenuExit( event_t ev)
{
   ENTRY;
}

void playerMoveEntry( event_t ev)
{
   ENTRY;
}

void computerMoveEntry( event_t ev)
{
   ENTRY;
}
