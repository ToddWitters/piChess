#include "hsm.h"
#include "hsmDefs.h"
#include "stdio.h"

#include "st_diagMenu.h"
#include "st_diagSwitch.h"
#include "st_mainMenu.h"
#include "st_menus.h"
#include "st_top.h"
#include "st_splashScreen.h"
#include "st_menus.h"
#include "st_initPosSetup.h"
#include "st_inGame.h"
#include "st_playingGame.h"
#include "st_optionMenu.h"
#include "st_playerMove.h"
#include "st_computerMove.h"
#include "st_moveForComputer.h"
#include "st_exitingGame.h"
#include "util.h"

// Used for diagnostics only
const char *eventName[] =
{
   "",
   "buttonState",
   "buttonPos",
   "pieceDrop",
   "pieceLift",
   "startSwitchDiag",
   "startInitSetup",
   "gotoMainMenu",
   "gotoDiagMenu"
   "moveClockTick"
};

// This array must be the same size as the state enum in hsmDefs.h
//   define the states in the same order
stateDef_t myStateDef[] =
{

//   displayName        parent            substatePicker              entry               exit
   { "top",             ST_NONE,          topPickSubstate,            topEntry,           NULL_EXIT_FUNC    },
   { "splashScreen",    ST_TOP,           NULL_SUBSTATE_PICKER_FUNC,  splashScreenEntry,  splashScreenExit  },
   { "menus",           ST_TOP,           NULL_SUBSTATE_PICKER_FUNC,  NULL_ENTRY_FUNC,    menusExit         },
   { "mainMenu",        ST_MENUS,         NULL_SUBSTATE_PICKER_FUNC,  mainMenuEntry,      mainMenuExit      },
   { "diagMenu",        ST_MENUS,         NULL_SUBSTATE_PICKER_FUNC,  diagMenuEntry,      diagMenuExit      },
   { "optionMenu",      ST_MENUS,         NULL_SUBSTATE_PICKER_FUNC,  optionMenuEntry,    optionMenuExit    },
   { "initPosSetup",    ST_TOP,           NULL_SUBSTATE_PICKER_FUNC,  initPosSetupEntry,  initPosSetupExit  },
   { "inGame",          ST_TOP,           inGamePickSubstate,         inGameEntry,        inGameExit        },
   { "playingGame",     ST_IN_GAME,       playingGamePickSubstate,    playingGameEntry,   playingGameExit   },
   { "playerMove",      ST_PLAYING_GAME,  NULL_SUBSTATE_PICKER_FUNC,  playerMoveEntry,    playerMoveExit    },
   { "computerMove",    ST_PLAYING_GAME,  NULL_SUBSTATE_PICKER_FUNC,  computerMoveEntry,  computerMoveExit  },
   { "moveForComputer", ST_PLAYING_GAME,  NULL_SUBSTATE_PICKER_FUNC , moveForComputerEntry, moveForComputerExit },
/*
   { "gameMenu",        ST_IN_GAME,       NULL_SUBSTATE_PICKER_FUNC   NULL_ENTRY_FUNC,    NULL_EXIT_FUNC    },
   { "fixingBoard",     ST_IN_GAME,       NULL_SUBSTATE_PICKER_FUNC   NULL_ENTRY_FUNC,    NULL_EXIT_FUNC    },
*/
   { "exitingGame",     ST_IN_GAME,       NULL_SUBSTATE_PICKER_FUNC,  exitingGameEntry,   exitingGameExit    },
   { "diagSensors",     ST_TOP,           NULL_SUBSTATE_PICKER_FUNC,  diagSwitchEntry,    diagSwitchExit    },
};

// Transition definitions...

// This list is sorted by event for quicker lookup (may be more than one entry per event)
//   Special case:  Internal events are designated by making the "to" target
//   equal to NULL_STATE_ID.  This causes no transitions and no entry/exit
//   functions to run.

transDef_t myTransDef[] =
{
// event                        from               to                  guard             action                     local?
   { EV_BUTTON_STATE,           ST_SPLASH_SCREEN,  ST_MAINMENU,        isStatePress,     NULL_ACTION_FUNC,          FALSE },
   { EV_BUTTON_STATE,           ST_DIAG_SENSORS,   ST_MAINMENU,        isStatePress,     NULL_ACTION_FUNC,          FALSE },
   { EV_BUTTON_STATE,           ST_MENUS,          ST_NONE,            isStatePress,     menus_buttonState_pressed, FALSE },
   { EV_BUTTON_STATE,           ST_INIT_POS_SETUP, ST_MAINMENU,        isStatePress,     NULL_ACTION_FUNC,          FALSE },
   { EV_BUTTON_STATE,           ST_EXITING_GAME,   ST_MAINMENU,        isStatePress,     NULL_ACTION_FUNC,          FALSE },

   { EV_BUTTON_POS,             ST_MENUS,          ST_NONE,            NULL_GUARD_FUNC,  menus_buttonPos,           FALSE },

   { EV_PIECE_DROP,             ST_DIAG_SENSORS,   ST_NONE,            NULL_GUARD_FUNC,  diagSwitch_boardChange,    FALSE },
   { EV_PIECE_DROP,             ST_INIT_POS_SETUP, ST_NONE,            NULL_GUARD_FUNC,  initPosSetup_boardChange,  FALSE },
   { EV_PIECE_DROP,             ST_PLAYER_MOVE,    ST_NONE,            NULL_GUARD_FUNC,  playerMoves_boardChange,   FALSE },

   { EV_PIECE_LIFT,             ST_DIAG_SENSORS,   ST_NONE,            NULL_GUARD_FUNC,  diagSwitch_boardChange,    FALSE },
   { EV_PIECE_LIFT,             ST_INIT_POS_SETUP, ST_NONE,            NULL_GUARD_FUNC,  initPosSetup_boardChange,  FALSE },
   { EV_PIECE_LIFT,             ST_PLAYER_MOVE,    ST_NONE,            NULL_GUARD_FUNC,  playerMoves_boardChange,   FALSE },

   { EV_START_SENSOR_DIAG,      ST_DIAGMENU,       ST_DIAG_SENSORS,    NULL_GUARD_FUNC,  NULL_ACTION_FUNC,          FALSE },

   { EV_START_INIT_POS_SETUP,   ST_MAINMENU,       ST_INIT_POS_SETUP,  NULL_GUARD_FUNC,  NULL_ACTION_FUNC,          FALSE },

   { EV_GOTO_MAIN_MENU,         ST_TOP,            ST_MAINMENU,        NULL_GUARD_FUNC,  NULL_ACTION_FUNC,          TRUE  },

   { EV_GOTO_DIAG_MENU,         ST_MAINMENU,       ST_DIAGMENU,        NULL_GUARD_FUNC,  NULL_ACTION_FUNC,          FALSE },

   { EV_GOTO_OPTION_MENU,       ST_MAINMENU,       ST_OPTIONMENU,      NULL_GUARD_FUNC,  NULL_ACTION_FUNC,          FALSE },

   { EV_GOTO_GAME,              ST_TOP,            ST_IN_GAME,         NULL_GUARD_FUNC,  NULL_ACTION_FUNC,          FALSE },

   { EV_GOTO_PLAYING_GAME,      ST_IN_GAME,        ST_PLAYING_GAME,    NULL_GUARD_FUNC,  NULL_ACTION_FUNC,          FALSE },

   { EV_GAME_DONE,              ST_IN_GAME,        ST_EXITING_GAME,    NULL_GUARD_FUNC,  NULL_ACTION_FUNC,          FALSE },

   { EV_MOVE_CLOCK_TIC,         ST_IN_GAME,        ST_NONE,            NULL_GUARD_FUNC,  inGame_moveClockTick,      FALSE },


};

const uint16_t transDefCount = (sizeof(myTransDef)/sizeof(myTransDef[0]));
