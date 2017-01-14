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
#include "st_arbPosSetup.h"
#include "st_inGame.h"
#include "st_playingGame.h"
#include "st_optionMenu.h"
#include "st_gameOptionMenu.h"
#include "st_boardOptionMenu.h"
#include "st_engineOptionMenu.h"
#include "st_playerMove.h"
#include "st_computerMove.h"
#include "st_moveForComputer.h"
#include "st_exitingGame.h"
#include "st_inGameMenu.h"
#include "st_moveForComputer.h"
#include "st_timeOptionMenu.h"
#include "st_fixBoard.h"
#include "util.h"

extern bool_t waitingForButton;

// This array must be the same size as the state enum in hsmDefs.h
//   define the states in the same order
stateDef_t myStateDef[] =
{

//   parent            init                     entry                  exit
   { ST_NONE,          topPickSubstate,         topEntry,              NULL_EXIT_FUNC       },
   { ST_TOP,           NULL_INIT_FUNC,          splashScreenEntry,     splashScreenExit     },
   { ST_TOP,           menuPickSubstate,        NULL_ENTRY_FUNC,       menusExit            },
   { ST_MENUS,         NULL_INIT_FUNC,          mainMenuEntry,         mainMenuExit         },
   { ST_MENUS,         NULL_INIT_FUNC,          diagMenuEntry,         diagMenuExit         },
   { ST_MENUS,         NULL_INIT_FUNC,          optionMenuEntry,       optionMenuExit       },
   { ST_MENUS,         NULL_INIT_FUNC,          boardOptionMenuEntry,  boardOptionMenuExit  },
   { ST_MENUS,         NULL_INIT_FUNC,          gameOptionMenuEntry,   gameOptionMenuExit   },
   { ST_MENUS,         NULL_INIT_FUNC,          engineOptionMenuEntry, engineOptionMenuExit },
   { ST_TOP,           NULL_INIT_FUNC,          timeOptionMenuEntry,   timeOptionMenuExit   },
   { ST_TOP,           NULL_INIT_FUNC,          initPosSetupEntry,     initPosSetupExit     },
   { ST_TOP,           NULL_INIT_FUNC,          arbPosSetupEntry,      arbPosSetupExit      },
   { ST_TOP,           inGamePickSubstate,      inGameEntry,           inGameExit           },
   { ST_IN_GAME,       playingGamePickSubstate, playingGameEntry,      playingGameExit      },
   { ST_PLAYING_GAME,  NULL_INIT_FUNC,          playerMoveEntry,       playerMoveExit       },
   { ST_PLAYING_GAME,  NULL_INIT_FUNC,          computerMoveEntry,     computerMoveExit     },
   { ST_PLAYING_GAME,  NULL_INIT_FUNC,          moveForComputerEntry,  moveForComputerExit  },
   { ST_IN_GAME,       NULL_INIT_FUNC,          inGameMenuEntry,       inGameMenuExit       },
   { ST_IN_GAME,       NULL_INIT_FUNC,          fixBoardEntry,         fixBoardExit         },
   { ST_IN_GAME,       NULL_INIT_FUNC,          exitingGameEntry,      exitingGameExit      },
   { ST_TOP,           NULL_INIT_FUNC,          diagSwitchEntry,       diagSwitchExit       },
};

// Transition definitions...

// This list is sorted by event for quicker lookup (may be more than one entry per event)
//   Special case:  Internal events are designated by making the "to" target
//   equal to ST_NONE.  This causes no transitions and no entry/exit
//   functions to run.

transDef_t myTransDef[] =
{
//   event                      from                  guard                          action                       to                     local?
   { EV_BUTTON_STATE,           ST_SPLASH_SCREEN,     isStatePress,                  NULL_ACTION_FUNC,            ST_MAINMENU,           FALSE },
   { EV_BUTTON_STATE,           ST_DIAG_SENSORS,      isStatePress,                  NULL_ACTION_FUNC,            ST_MAINMENU,           FALSE },
   { EV_BUTTON_STATE,           ST_MENUS,             isStatePress,                  menus_buttonState_pressed,   ST_NONE,               FALSE },
   { EV_BUTTON_STATE,           ST_GAMEMENU,          isStatePress,                  menus_buttonState_pressed,   ST_NONE,               FALSE },
   { EV_BUTTON_STATE,           ST_INIT_POS_SETUP,    isStatePress,                  NULL_ACTION_FUNC,            ST_MAINMENU,           FALSE },
   { EV_BUTTON_STATE,           ST_EXITING_GAME,      isStatePress,                  NULL_ACTION_FUNC,            ST_MAINMENU,           FALSE },
   { EV_BUTTON_STATE,           ST_PLAYER_MOVE,       isStatePress,                  NULL_ACTION_FUNC,            ST_GAMEMENU,           FALSE },
   { EV_BUTTON_STATE,           ST_TIME_OPTION_MENU,  isStatePress,                  timeOptionMenuButtonHandler, ST_NONE,               FALSE },
   { EV_BUTTON_STATE,           ST_COMPUTER_MOVE,     computerMoveWaitingButton,     computerMoveButtonStop,      ST_NONE,               FALSE },
   { EV_BUTTON_STATE,           ST_ARB_POS_SETUP,     arbPosSetupCheckPosition,      NULL_ACTION_FUNC,            ST_IN_GAME,            FALSE },
   { EV_BUTTON_STATE,           ST_ARB_POS_SETUP,     isStatePress,                  NULL_ACTION_FUNC,            ST_MAINMENU,           FALSE },
   { EV_BUTTON_STATE,           ST_FIX_BOARD,         isStatePress,                  NULL_ACTION_FUNC,            ST_MAINMENU,           FALSE },

   { EV_BUTTON_POS,             ST_MENUS,             NULL_GUARD_FUNC,               menus_buttonPos,             ST_NONE,               FALSE },
   { EV_BUTTON_POS,             ST_GAMEMENU,          NULL_GUARD_FUNC,               menus_buttonPos,             ST_NONE,               FALSE },
   { EV_BUTTON_POS,             ST_TIME_OPTION_MENU,  NULL_GUARD_FUNC,               timeOptionMenuButtonHandler, ST_NONE,               FALSE },
   { EV_BUTTON_POS,             ST_ARB_POS_SETUP,     NULL_GUARD_FUNC,               arbPosSetupHandleBtnPos,     ST_NONE,               FALSE },
   { EV_BUTTON_POS,             ST_PLAYER_MOVE,       playerMoves_promoting,         playerMoves_changeProPiece,  ST_NONE,               FALSE },
   { EV_PIECE_DROP,             ST_DIAG_SENSORS,      NULL_GUARD_FUNC,               diagSwitch_boardChange,      ST_NONE,               FALSE },
   { EV_PIECE_DROP,             ST_INIT_POS_SETUP,    NULL_GUARD_FUNC,               initPosSetup_boardChange,    ST_NONE,               FALSE },
   { EV_PIECE_DROP,             ST_PLAYER_MOVE,       NULL_GUARD_FUNC,               playerMoves_boardChange,     ST_NONE,               FALSE },
   { EV_PIECE_DROP,             ST_MOVE_FOR_COMPUTER, NULL_GUARD_FUNC,               moveForComputer_boardChange, ST_NONE,               FALSE },
   { EV_PIECE_DROP,             ST_ARB_POS_SETUP,     NULL_GUARD_FUNC,               arbPosSetup_boardChange,     ST_NONE,               FALSE },
   { EV_PIECE_DROP,             ST_FIX_BOARD,         NULL_GUARD_FUNC,               fixBoard_boardChange,        ST_NONE,               FALSE },

   { EV_PIECE_LIFT,             ST_DIAG_SENSORS,      NULL_GUARD_FUNC,               diagSwitch_boardChange,      ST_NONE,               FALSE },
   { EV_PIECE_LIFT,             ST_INIT_POS_SETUP,    NULL_GUARD_FUNC,               initPosSetup_boardChange,    ST_NONE,               FALSE },
   { EV_PIECE_LIFT,             ST_PLAYER_MOVE,       NULL_GUARD_FUNC,               playerMoves_boardChange,     ST_NONE,               FALSE },
   { EV_PIECE_LIFT,             ST_MOVE_FOR_COMPUTER, NULL_GUARD_FUNC,               moveForComputer_boardChange, ST_NONE,               FALSE },
   { EV_PIECE_LIFT,             ST_ARB_POS_SETUP,     NULL_GUARD_FUNC,               arbPosSetup_boardChange,     ST_NONE,               FALSE },
   { EV_PIECE_LIFT,             ST_FIX_BOARD,         NULL_GUARD_FUNC,               fixBoard_boardChange,        ST_NONE,               FALSE },

   { EV_START_SENSOR_DIAG,      ST_DIAGMENU,          NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_DIAG_SENSORS,       FALSE },
   { EV_START_INIT_POS_SETUP,   ST_MAINMENU,          NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_INIT_POS_SETUP,     FALSE },

   { EV_START_ARB_POS_SETUP,    ST_MAINMENU,          NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_ARB_POS_SETUP,      FALSE },

   { EV_GOTO_MAIN_MENU,         ST_TOP,               NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_MAINMENU,           TRUE  },
   { EV_GOTO_DIAG_MENU,         ST_MAINMENU,          NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_DIAGMENU,           FALSE },
   { EV_GOTO_OPTION_MENU,       ST_TOP,               NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_OPTIONMENU,         FALSE },
   { EV_GOTO_BOARD_OPTIONS,     ST_OPTIONMENU,        NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_BOARD_OPTION_MENU,  FALSE },
   { EV_GOTO_GAME_OPTIONS,      ST_OPTIONMENU,        NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_GAME_OPTION_MENU,   FALSE },
   { EV_GOTO_ENGINE_OPTIONS,    ST_OPTIONMENU,        NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_ENGINE_OPTION_MENU, FALSE },
   { EV_GOTO_TIME_OPTIONS,      ST_GAME_OPTION_MENU,  NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_TIME_OPTION_MENU,   FALSE },
   { EV_GOTO_GAME,              ST_TOP,               NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_IN_GAME,            FALSE },
   { EV_GOTO_PLAYING_GAME,      ST_IN_GAME,           NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_PLAYING_GAME,       TRUE  },
   { EV_GOTO_GAMEMENU,          ST_IN_GAME,           NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_GAMEMENU,           FALSE },
   { EV_GAME_DONE,              ST_IN_GAME,           NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_EXITING_GAME,       FALSE },
   { EV_MOVE_CLOCK_TIC,         ST_IN_GAME,           NULL_GUARD_FUNC,               inGame_moveClockTick,        ST_NONE,               FALSE },
   { EV_PROCESS_COMPUTER_MOVE,  ST_IN_GAME,           NULL_GUARD_FUNC,               computerMove_computerPicked, ST_NONE,               FALSE },
   { EV_PLAYER_MOVED_FOR_COMP,  ST_MOVE_FOR_COMPUTER, NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_PLAYING_GAME,       TRUE  },

   { EV_FIX_BOARD,              ST_IN_GAME,           NULL_GUARD_FUNC,               NULL_ACTION_FUNC,            ST_FIX_BOARD,          TRUE  },
};

const uint16_t transDefCount = (sizeof(myTransDef)/sizeof(myTransDef[0]));
