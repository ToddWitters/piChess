#include "diag.h"
#include "moves.h"
#include "event.h"
#include "board.h"
#include "led.h"
#include "display.h"
#include "switch.h"
#include "util.h"
#include "timer.h"
#include "constants.h"
#include "types.h"
#include "menu.h"
#include "options.h"
#include "sfInterface.h"
#include "specChars.h"

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <stdio.h>

// Holds a valid move for the current board along with the resulting
//   dirtySquares and occupiedSquares
typedef struct moveEffect_s
{
   move_t move;          // A legal move

   // The following can be used to determine when the player has made a valid move.
   BB dirtySquares;      // Bitboard of squares that should see lifts/drops for the given move
   BB occupiedSquares;   // Bitboard of the occupied squares at the conclusion of the move
}moveEffects_t;

typedef enum moveVal_t
{
   MV_ILLEGAL,    // Move primitive is not part of any  legal move
   MV_PRECURSOR,  // Move primitive is a precursor to one or more legal moves.
   MV_LEGAL       // Move primitive has completed a legal move
}moveVal_t;

// Valid states and their descriptions
//  When changing this enum, also need to change:
//    (1) stateNames  array
//    (2) transTable array
typedef enum state_e
{
   ST_INIT,                // Splash screen, system init.  Only exits on timer/center.  Moves to ST_TOP_MENUS when done
   ST_TOP_MENUS,           // User is navigating top menu structure.
   ST_INITIAL_SETUP,       // User(s) are setting up initial board
   ST_SETUP,               // User is setting up a position
   ST_HUMAN_MV,            // A human player is making a move.
   ST_COMPUTER_MV,         // The computer is computing a move
   ST_HUMAN_MV_FOR_COMP,   // The human is making the computer's chosen move
   ST_GAME_OVER,           // The game has reached a conclusion
   ST_FIX_BOARD,           // Prompt player to reach a target position
   ST_VERIFY_BOARD,        // User is verifying the board.
   ST_DIAG_SWITCHES,       // Testing reed switches
   ST_DIAG_DISPLAY,        // Testing display
   ST_DIAG_JOYSTICK,       // Testing joystick
   ST_DIAG_LEDS,           // Testing LEDs

   TOTAL_STATES            // Total number of states
}state_t;

// typedefs for functions called on entry/exit to/from a state
typedef void (* tranEnterFunc_t   )( state_t from );
typedef void (* tranExitFunc_t )   ( state_t to   );

// Array of functions for each enter/exit transition
typedef struct transFuncs_s
{
   tranEnterFunc_t  enter[TOTAL_STATES];
   tranExitFunc_t   exit[TOTAL_STATES];
}transFuncs_t;


char *stateNames[TOTAL_STATES] =
{
   "init",
   "top menus",
   "initial setup",
   "setup board",
   "human moving",
   "computer moving",
   "human moving for computer",
   "game over",
   "fix board",
   "verify board"
   "test switches",
   "test display",
   "test joystick",
   "test LEDs"
};

// The menu that has UI focus at the moment
menu_t *currentMenu = NULL;

// The individual menus...
menu_t *mainMenu = NULL;
menu_t *settingsMenu = NULL;
menu_t *playersMenu = NULL;
menu_t *diagMenu = NULL;
menu_t *onMoveMenu = NULL;
menu_t *timeSettingMenu = NULL;

// The text of the last move in either SAN or coordinate notation.
char lastMoveText[8];

// Prototypes for entry/exit functions
static void enterTopMenus( state_t from);
static void enterInitialSetup( state_t from);
static void enterSetup( state_t from);
static void enterHumanMove( state_t from);
static void enterHumanMoveForComp( state_t from);
static void enterComputerMove( state_t from);
static void enterGameOver( state_t from);
static void enterTestJoystick( state_t from);
static void exitInit( state_t to);
static void exitTopMenus( state_t to);

// If these are used in the table below, no function is called for the given transition
#define NULL_TRAN_ENTER_FUNC   (tranEnterFunc_t)NULL
#define NULL_TRAN_EXIT_FUNC    (tranExitFunc_t)NULL

// The table of transistion functions
transFuncs_t transTable =
{
   {
      NULL_TRAN_ENTER_FUNC,    // entering ST_INIT
      enterTopMenus,           // entering ST_TOP_MENUS
      enterInitialSetup,       // entering ST_INITIAL_SETUP
      enterSetup,              // entering ST_SETUP
      enterHumanMove,          // entering ST_HUMAN_MV
      enterComputerMove,       // entering ST_COMPUTER_MV
      enterHumanMoveForComp,   // entering ST_HUMAN_MOVE_FOR_COMP
      enterGameOver,           // entering ST_GAME_OVER
      NULL_TRAN_ENTER_FUNC,    // entering ST_FIX_BOARD,
      NULL_TRAN_ENTER_FUNC,    // entering ST_VERIFY,
      NULL_TRAN_ENTER_FUNC,    // entering ST_DIAG_SWITCHES,
      NULL_TRAN_ENTER_FUNC,    // entering ST_DIAG_DISPLAY,
      enterTestJoystick,       // entering ST_DIAG_JOYSTICK,
      NULL_TRAN_ENTER_FUNC,    // entering ST_DIAG_LEDS,
   },

   {
      exitInit,                // exiting ST_INIT
      exitTopMenus,            // exiting ST_TOP_MENUS
      NULL_TRAN_EXIT_FUNC,     // exiting ST_INITIAL_SETUP
      NULL_TRAN_EXIT_FUNC,     // exiting ST_SETUP
      NULL_TRAN_EXIT_FUNC,     // exiting ST_HUMAN_MV
      NULL_TRAN_EXIT_FUNC,     // exiting ST_COMPUTER_MV
      NULL_TRAN_EXIT_FUNC,     // exiting ST_HUMAN_MOVE_FOR_COMP
      NULL_TRAN_EXIT_FUNC,     // exiting ST_GAME_OVER
      NULL_TRAN_EXIT_FUNC,     // exiting ST_FIX_BOARD,
      NULL_TRAN_EXIT_FUNC,     // exiting ST_VERIFY,
      NULL_TRAN_EXIT_FUNC,     // exiting ST_DIAG_SWITCHES,
      NULL_TRAN_EXIT_FUNC,     // exiting ST_DIAG_DISPLAY,
      NULL_TRAN_EXIT_FUNC,     // exiting ST_DIAG_JOYSTICK,
      NULL_TRAN_EXIT_FUNC,     // exiting ST_DIAG_LEDS,
   }
};

// Array holding the current options
options_t options;

// The current state of the chess board (pieces/positions/castling rights/etc..)
static board_t brd;

// Used to direct player to a target position (used in FIX_BOARD state when)
//   Player must recover changes to board while computer was thinking before
//   he can make computer's move
static board_t targetBrd;

// The current game
static game_t game;

// Current State machine state
static state_t state = ST_INIT;

// The most previous state.  Used with FIX state to know where to return.
static state_t prevState = ST_INIT;

// Used for the more complex states...
static uint8_t subState = 0;

// Tracks which squares have seen activity since last reset
static BB dirtySquares = 0;

// Tracks which squares are currently occupied by pieces on the physcial board.
static BB occupiedSquares = 0;

// TODO refactor move() to work with moveEffects_t structure.
static move_t legalMoves[200];

// List of legal moves (and their effects) for this position (computed on entry to player moving state)
static moveEffects_t moveEffects[200];

// size of previous lists
static int totalLegalMoves = 0;

// The data related to a triggering event
static eventData_t eventData;

// RTOS stuff..
static pthread_t eventManagerThread;

// Piece selection for setup state
static color_t selectedColor;
static piece_t selectedPiece;

// State transition logic
static void transState(event_t to);

// Event handlers...
static void boardChangeHandler( int sq, event_t ev );
static void buttonPressEventHandler( buttonPress_t bEvent );
static void buttonPosEventHandler  ( buttonPos_t   bPos );
static void menuEventHandler(menuEvent_t ev);
static void engineEventHandler( int param );
static void timerEventHandler( int param );

// menu picker functions
char* openingBookPicker( int dir );

// main task
static void *eventManagerTask ( void *arg );

// utility functions
static void displaySplashScreen( void );
static moveVal_t checkValidMoveProgress(moveEffects_t *moveEffects, int numMoves, BB dirtySquares, BB occupiedSquares, move_t **ret);
static void calculateMoveEffects(const move_t *moves, const board_t *brd, moveEffects_t *effects, int num);
static void startGame( void );
static void applyMove( move_t mv );
static char *convertTimeToString (uint32_t tenths );

// Screen update functions for in-game display
static void drawGameScreen( void );
static void udpateGameDisplayClocks( void );
static void updateGameDisplayBanner( void );
static void updateGameDisplayLastMove( void );
static void updateGameDisplayMaterial( void );
static void updateGameDisplayClockLegend( void );
static char *convertTimeToString (uint32_t tenths );

// Init function called by main.
void eventManagerInit( void )
{

   DPRINT("Initializing event manager\n");

   // create the main thread for the event manager
   pthread_create(&eventManagerThread, NULL, eventManagerTask , NULL);

}

// Main task.  Simply waits for a system event, then calls the
// necessary handler...
static void *eventManagerTask ( void *arg )
{

   displaySplashScreen();

   // Start time-out for splash screen.
   timerStart(TMR_UI_TIMEOUT, 5000, 0);

   while(1)
   {

      eventData_t *evPtr;

      // Wait for an event...
      sem_wait(getQueueSem(EVQ_EVENT_MANAGER));

      // Retrieve it
      evPtr = getEvent(EVQ_EVENT_MANAGER);

      // skip processing on error...
      if(evPtr == NULL) continue;

      // Move new event into local structure
      memcpy(&eventData, evPtr, sizeof(eventData_t));

      // What's the event type?
      switch(eventData.ev)
      {
         // User has added/removed a piece
         case EV_PIECE_DROP:
         case EV_PIECE_LIFT:
            boardChangeHandler((int)eventData.param, eventData.ev);
            break;

         // Button has been pressed/released
         case EV_BUTTON_STATE:
            buttonPressEventHandler((buttonPress_t)eventData.param);
            break;

         // Button position has changed
         case EV_BUTTON_POS:
            buttonPosEventHandler((buttonPos_t)eventData.param);
            break;

         // A menu item has been selected
         case EV_MENU_PICK:
            menuEventHandler(eventData.param);
            break;

         // The chess engine has selected a move
         case EV_ENGINE:
            engineEventHandler(eventData.param);
            break;

         // A timer has elapsed
         case EV_TIMER:
            timerEventHandler(eventData.param);
            break;
      }
   }

   // We should never be here.
   return NULL;
}

///////////////////////
// Event Category Handlers
///////////////////////

// Handles a qualified (debounced) change in a reed switch
static void boardChangeHandler( int sq, event_t ev )
{

   if(sq > 63)
   {
      DPRINT("Invalid square %d passed to boardChangeHandler\n", sq);
      return;
   }

   if( (ev !=  EV_PIECE_DROP) && (ev != EV_PIECE_LIFT) )
   {
      DPRINT("Invalid event %d passed to boardChangeHandler\n", (int)ev);
      return;
   }

   if(ev == EV_PIECE_DROP)
   {
      if(squareMask[sq] & occupiedSquares)
      {
         DPRINT("ERROR: Drop event to square %d occurred with piece already on square\n", sq);
         return;
      }
      occupiedSquares |= squareMask[sq];
   }
   else
   {
      if( !(squareMask[sq] & occupiedSquares))
      {
         DPRINT("ERROR: Lift event to square %d occurred with no piece on square\n", sq);
         return;
      }
      occupiedSquares &= ~squareMask[sq];
   }

   DPRINT("Piece %s %s\n", ((ev == EV_PIECE_DROP) ? "dropped on" : "lifted from"), convertSqNumToCoord(sq) );

   // update dirty square info
   dirtySquares |= squareMask[sq];

   // Now handle the event based upon state...
   switch(state)
   {
      case ST_INITIAL_SETUP: // User(s) setting up initial board position...
         {
            BB delta = occupiedSquares ^ (brd.colors[WHITE] | brd.colors[BLACK]);

            if(delta) LED_SetGridState(delta);
            else
            {
               LED_AllOff();
               startGame(); // will transition to either ST_PLAYER_MV or ST_HUMAN_MV
            }
         }
         break;

      case ST_SETUP:
         switch( ev )
         {
            case EV_PIECE_DROP:
               addPiece(&brd, sq, selectedPiece, selectedColor);
               break;

            case EV_PIECE_LIFT:
               if      ( brd.colors[WHITE] & squareMask[sq] )  { selectedColor = WHITE;  displayWriteChars(1, 4,  5, "White");  }
               else if ( brd.colors[BLACK] & squareMask[sq] )  { selectedColor = BLACK;  displayWriteChars(1, 4,  5, "Black");  }

               if      ( brd.pieces[PAWN]   & squareMask[sq] ) { selectedPiece = PAWN;   displayWriteChars(1, 10, 6, "Pawn  "); }
               else if ( brd.pieces[KNIGHT] & squareMask[sq] ) { selectedPiece = KNIGHT; displayWriteChars(1, 10, 6, "Knight"); }
               else if ( brd.pieces[BISHOP] & squareMask[sq] ) { selectedPiece = BISHOP; displayWriteChars(1, 10, 6, "Bishop"); }
               else if ( brd.pieces[ROOK]   & squareMask[sq] ) { selectedPiece = ROOK;   displayWriteChars(1, 10, 6, "Rook  "); }
               else if ( brd.pieces[QUEEN]  & squareMask[sq] ) { selectedPiece = QUEEN;  displayWriteChars(1, 10, 6, "Queen "); }
               else if ( brd.pieces[KING]   & squareMask[sq] ) { selectedPiece = KING;   displayWriteChars(1, 10, 6, "King  "); }

               removePiece(&brd, sq, selectedPiece, selectedColor);
               LED_SetGridState( brd.pieces[selectedPiece] & brd.colors[selectedColor] );
               break;

            default:
               break;
         }
         break;

      case ST_HUMAN_MV:
         {
            move_t *mv;

            // If we are back to the pre-move state, shut off all LEDs and start over
            if(occupiedSquares == (brd.colors[WHITE] | brd.colors[BLACK]))
            {
               DPRINT("Move was canceled by user... trying again\n");
               dirtySquares = 0;
               LED_AllOff();
               // TODO restore game screen
            }
            else
            {
               switch( checkValidMoveProgress(moveEffects, totalLegalMoves, dirtySquares, occupiedSquares, &mv) )
               {
                  // Piece activity so far cannot result in a valid move.  Flash squares that need to be reset
                  case MV_ILLEGAL:
                     displayWriteLine( 1, "Illegal move", TRUE);
                     displayWriteLine( 2, "Please return pieces", TRUE);
                     LED_FlashGridState(dirtySquares);
                     break;

                     // A legal move has been completed...
                  case MV_LEGAL:

                     // Turn off all the LEDs
                     LED_AllOff();

                     // Apply the move and switch to other player...
                     applyMove(*mv);
                     break;

                     // Move primitives so far match one or more valid move patterns..
                  case MV_PRECURSOR:

                     // Show the squares that have so far been affected
                     LED_SetGridState(dirtySquares);
                     break;
               }
            }
         }
         break;

      default:
         break;
   }
}
// Handle button press/release
static void buttonPressEventHandler( buttonPress_t bEvent )
{

   // If we are in a menu, handle per selected menu item
   if(currentMenu != NULL && bEvent != B_RELEASED)
   {
      int retVal = menuProcessButtonPress(currentMenu);

      // If return value is an event to be handled...
      if(retVal > 0 || retVal == M_EV_BACK)
      {
         eventData_t evnt;
         evnt.ev    = EV_MENU_PICK;
         evnt.param = retVal;
         putEvent(EVQ_EVENT_MANAGER, &evnt);
      }

   }
   else
   {
      // We are not in a menu.  Handle based upon our state...
      switch(bEvent)
      {
         // If the button was pressed...
         case B_PRESSED:
            switch(state)
            {
               case ST_SETUP:
                  // substate0 : source? manual or load
                  // substate1 : clear board
                  // substate2 : add pieces
                  // substate3 : Prompt for side on move
                  // substate4 : Prompt for castling rights
                  // substate5 : Prompt for enpassant
                  // substate6 : Prompt for next action (analyze, play as w, play as b)
                  break;

               case ST_INIT:
                  // Short circuit to top menu
                  transState( ST_TOP_MENUS );
                  break;

               case ST_GAME_OVER:
                  // User has acknowledged game outcome... return to top menu.
                  transState(ST_TOP_MENUS);
                  break;

               case ST_DIAG_JOYSTICK:
                  displayWriteChars( 1, 9, 1, "*");
                  // Restart 5 second time
                  timerStart(TMR_DIAG_TIMEOUT, 5000, 0);
                  break;

               case ST_HUMAN_MV:

                  if (onMoveMenu != NULL)
                  {
                     destroyMenu(onMoveMenu);
                  }

                  onMoveMenu = createMenu("--- In-game Menu ---", TRUE);/* code */

                  menuAddItem(onMoveMenu, ADD_TO_END, "Cancel",             M_EV_BACK,       M_EV_IGNORED, NULL);
                  menuAddItem(onMoveMenu, ADD_TO_END, "Resign",             M_EV_IGNORED,    M_EV_IGNORED, NULL);
                  menuAddItem(onMoveMenu, ADD_TO_END, "Adjourn Game",       M_EV_IGNORED,    M_EV_IGNORED, NULL);


                  // TODO  should these items appear if not claimable?
                  menuAddItem(onMoveMenu, ADD_TO_END, "Claim 50mv Draw",    M_EV_IGNORED,    M_EV_IGNORED, NULL);
                  menuAddItem(onMoveMenu, ADD_TO_END, "Claim 3-fold rep",   M_EV_IGNORED,    M_EV_IGNORED, NULL);

                  // If opponent's clock has expired AND it started with a non-zero value...
                  if( ( (brd.toMove == WHITE) && (game.btime == 0) && (options.game.blackTime != 0) ) ||
                      ( (brd.toMove == BLACK) && (game.wtime == 0) && (options.game.whiteTime != 0) ) )
                  {
                     menuAddItem(onMoveMenu, ADD_TO_END, "Claim Time",         M_EV_IGNORED,    M_EV_IGNORED, NULL);
                  }

                  menuAddItem(onMoveMenu, ADD_TO_END, "Verify Board",       M_EV_IGNORED,    M_EV_IGNORED, NULL);
                  menuAddItem(onMoveMenu, ADD_TO_END, "Save Position",      M_EV_IGNORED,    M_EV_IGNORED, NULL);

                  currentMenu = onMoveMenu;

                  drawMenu(currentMenu);

               default:
                  break;
            }
            break;

         case B_RELEASED:
            switch(state)
            {
               case ST_DIAG_JOYSTICK:
                  displayWriteChars( 1, 9, 1, "+");
                  timerKill(TMR_DIAG_TIMEOUT);
                  break;

               default:
                  break;
            }

            break;

         default:
            break;
      }
   }
}

// Handle joystick position changes
static void buttonPosEventHandler  (buttonPos_t bPos )
{

   // Handle with menu functions if currently in menu
   if(currentMenu != NULL)
   {
      int retVal = menuProcessButtonPos(currentMenu, bPos);

      if(retVal >  0 || retVal == M_EV_BACK)
      {
         eventData_t evnt;
         evnt.ev    = EV_MENU_PICK;
         evnt.param = retVal;
         putEvent(EVQ_EVENT_MANAGER, &evnt);
      }
   }
   else
   {
      // Handle based upon state
      switch(state)
      {
         eventData_t ev;

         // Button position changes in this state determine what the next dropped piece will become...
         case ST_SETUP:
         switch(bPos)
         {
            case POS_UP:
               {
                  switch(selectedPiece)
                  {
                     case KING:   selectedPiece = PAWN;   displayWriteChars(1, 10, 6, "Pawn  "); break;
                     case QUEEN:  selectedPiece = KING;   displayWriteChars(1, 10, 6, "King  "); break;
                     case ROOK:   selectedPiece = QUEEN;  displayWriteChars(1, 10, 6, "Queen "); break;
                     case BISHOP: selectedPiece = ROOK;   displayWriteChars(1, 10, 6, "Rook  "); break;
                     case KNIGHT: selectedPiece = BISHOP; displayWriteChars(1, 10, 6, "Bishop"); break;
                     case PAWN:   selectedPiece = KNIGHT; displayWriteChars(1, 10, 6, "Knight"); break;
                     default: break;
                  }

                  LED_SetGridState( brd.pieces[selectedPiece] & brd.colors[selectedColor] );
               }
               break;

            case POS_DOWN:
               {
                  switch(selectedPiece)
                  {
                     case KING:   selectedPiece = QUEEN;  displayWriteChars(1, 10, 6, "Queen "); break;
                     case QUEEN:  selectedPiece = ROOK;   displayWriteChars(1, 10, 6, "Rook  "); break;
                     case ROOK:   selectedPiece = BISHOP; displayWriteChars(1, 10, 6, "Bishop"); break;
                     case BISHOP: selectedPiece = KNIGHT; displayWriteChars(1, 10, 6, "Knight"); break;
                     case KNIGHT: selectedPiece = PAWN;   displayWriteChars(1, 10, 6, "Pawn  "); break;
                     case PAWN:   selectedPiece = KING;   displayWriteChars(1, 10, 6, "King  "); break;
                     default: break;
                  }
                  LED_SetGridState( brd.pieces[selectedPiece] & brd.colors[selectedColor] );
                  break;
               }

            case POS_LEFT:
            case POS_RIGHT:
               {
                  if(selectedColor == WHITE)
                  {
                     selectedColor = BLACK;
                     displayWriteChars(1, 4, 5,"Black");
                  }
                  else
                  {
                     selectedColor = WHITE;
                     displayWriteChars(1, 4, 5,"White");
                  }
                  LED_SetGridState( brd.pieces[selectedPiece] & brd.colors[selectedColor] );
               }
               break;
            default:
               break;
         }

         case ST_DIAG_JOYSTICK:
         {
            // Clear all the positional indicators below...
            displayWriteChars(0, 8,  3,"   ");
            displayWriteChars(1, 8,  1," "  );
            displayWriteChars(1, 10, 1," "  );
            displayWriteChars(2, 8,  3,"   ");

            switch( bPos )
            {
               case    POS_RIGHT:       displayWriteChars(1,10, 1,"\x7e"); break;
               case    POS_LEFT:        displayWriteChars(1,8,  1,"\x7f"); break;
               case    POS_DOWN:        displayWriteChars(2,9,  1,"|");    break;
               case    POS_UP:          displayWriteChars(0,9,  1,"|");    break;
               case    POS_UP_RIGHT:    displayWriteChars(0,10, 1,"/");    break;
               case    POS_UP_LEFT:     displayWriteChars(0,8,  1,"\x0");  break;
               case    POS_DOWN_RIGHT:  displayWriteChars(2,10, 1,"\x0");  break;
               case    POS_DOWN_LEFT:   displayWriteChars(2,8,  1,"/");    break;
               default: break;
            }

         }
         break;

         case ST_HUMAN_MV:

         // DEBUG  - Use position change in human move state to simulate user trying to make a move...
         ev.ev = EV_PIECE_LIFT;
         ev.param = B1;
         putEvent(EVQ_EVENT_MANAGER, &ev);

         ev.ev = EV_PIECE_DROP;
         ev.param = C3;
         putEvent(EVQ_EVENT_MANAGER, &ev);
         break;

         case ST_TOP_MENUS:
         // We should never be here since currentMenu should always be non-NULL in this state
         DPRINT("Ignoring button position event in buttonPosEventHandler\n");

         break;

         default:
         break;

      }
   }
}

// Handle events triggered by menu operations
static void menuEventHandler(menuEvent_t ev)
{
   if(state == ST_TOP_MENUS)
   {
      switch(ev)
      {
         // User selected "Play Chess"
         case M_EV_START_GAME:

            DPRINT("Starting a new game...\n");

            game.startPos = NULL;
            game.brd = &brd;

            // Start with a board set to an initial state.
            setBoard(&brd, game.startPos);

            // Is the initial game position already present?
            if(occupiedSquares ^ (brd.colors[WHITE] | brd.colors[BLACK]))
            {
               // no...we aren't there yet...

               // Give the user a visual to prompt change..
               LED_SetGridState(occupiedSquares ^ (brd.colors[WHITE] | brd.colors[BLACK]));

               // Put message on display
               transState(ST_INITIAL_SETUP);
            }
            else
            {
               // Looks good... let's go!
               // TBD Do we need to add an acknowledge here to start the game?
               startGame();
            }
            break;

         case M_EV_GAME_SETTINGS:
            DPRINT("Game Settings...\n");

            settingsMenu = createMenu("---Game Settings----", TRUE);

            //          menu          offset       text              press                 right                  picker
            menuAddItem(settingsMenu, ADD_TO_END, "Players...",      M_EV_PLAYERS,         M_EV_IGNORED,          NULL);

            if(options.game.black == PLAYER_COMPUTER || options.game.white == PLAYER_COMPUTER)
            {
               menuAddItem(settingsMenu, ADD_TO_END, "Opening Book", M_EV_IGNORED, M_EV_IGNORED, openingBookPicker);
            }

            menuAddItem(settingsMenu, ADD_TO_END, "Time Settings",     M_EV_TIME_SETTINGS, M_EV_IGNORED, NULL);
            menuAddItem(settingsMenu, ADD_TO_END, "Back to Main Menu", M_EV_BACK, M_EV_IGNORED, NULL);

            currentMenu = settingsMenu;
            drawMenu(settingsMenu);
            break;

         case M_EV_PLAYERS:
            if(playersMenu == NULL)
            {
               playersMenu = createMenu("---Choose Players---", FALSE);
               menuAddItem(playersMenu, ADD_TO_END, "Player vs. Computer", M_EV_HUMAN_WHITE, M_EV_IGNORED, NULL);
               menuAddItem(playersMenu, ADD_TO_END, "Computer vs. Player", M_EV_HUMAN_BLACK, M_EV_IGNORED, NULL);
               menuAddItem(playersMenu, ADD_TO_END, "Player vs. Player",   M_EV_HUMAN_BOTH, M_EV_IGNORED, NULL);
            }
            currentMenu = playersMenu;
            drawMenu(playersMenu);
            break;


         case M_EV_HUMAN_WHITE:
            {
               options.game.white = PLAYER_HUMAN;
               options.game.black = PLAYER_COMPUTER;
               currentMenu = settingsMenu;
               drawMenu(currentMenu);
            }
            break;

         case M_EV_HUMAN_BLACK:
            {
               options.game.white = PLAYER_COMPUTER;
               options.game.black = PLAYER_HUMAN;
               currentMenu = settingsMenu;
               drawMenu(currentMenu);
            }
            break;

         case M_EV_HUMAN_BOTH:
            {
               options.game.white = PLAYER_HUMAN;
               options.game.black = PLAYER_HUMAN;
               currentMenu = settingsMenu;
               drawMenu(currentMenu);
            }
            break;

         case M_EV_SETUP_POS:
            DPRINT("Setting up a position...\n");
            transState(ST_SETUP);
            break;

         case M_EV_DIAGNOSTICS:
            DPRINT("Running diagnostics...\n");

            if(diagMenu == NULL)
            {
               diagMenu = createMenu("-----Diagnostics-----", TRUE);
               menuAddItem(diagMenu, ADD_TO_END, "Switches", M_EV_DIAG_SWITCHES, M_EV_IGNORED, NULL);
               menuAddItem(diagMenu, ADD_TO_END, "Joystick", M_EV_DIAG_JOYSTICK, M_EV_IGNORED, NULL);
               menuAddItem(diagMenu, ADD_TO_END, "Display",  M_EV_DIAG_DISPLAY,  M_EV_IGNORED, NULL);
               menuAddItem(diagMenu, ADD_TO_END, "LEDs",     M_EV_DIAG_LEDS,     M_EV_IGNORED, NULL);
            }

            currentMenu = diagMenu;
            drawMenu(currentMenu);

            break;

         case M_EV_BACK:
            if(currentMenu == settingsMenu)
            {
               destroyMenu(settingsMenu);
               settingsMenu = NULL;
               currentMenu = mainMenu;
               drawMenu(currentMenu);
            }
            else if(currentMenu == diagMenu)
            {
               currentMenu = mainMenu;
               drawMenu(currentMenu);
            }
            else if(currentMenu == onMoveMenu)
            {
               currentMenu = NULL;
               displayClear();
               drawGameScreen();
            }
            else if(currentMenu == timeSettingMenu)
            {
               currentMenu = settingsMenu;
               drawMenu(currentMenu);
            }

            break;

         case M_EV_TIME_SETTINGS:
            DPRINT("Time Settings\n");

            if(timeSettingMenu == NULL)
            {
               timeSettingMenu = createMenu("---Time Settings---_", FALSE);
               menuAddItem(timeSettingMenu, ADD_TO_END, "Cancel",      M_EV_BACK,    M_EV_IGNORED, NULL);
               menuAddItem(timeSettingMenu, ADD_TO_END, "Standard..",  M_EV_IGNORED, M_EV_IGNORED, NULL);
               menuAddItem(timeSettingMenu, ADD_TO_END, "Time Odds..", M_EV_IGNORED, M_EV_IGNORED, NULL);
               menuAddItem(timeSettingMenu, ADD_TO_END, "None",        M_EV_IGNORED, M_EV_IGNORED, NULL);
            }
            currentMenu = timeSettingMenu;
            drawMenu(currentMenu);

            break;

         case M_EV_DIAG_SWITCHES:
            DPRINT("Debug Switches & LEDs\n");
            break;

         case M_EV_DIAG_JOYSTICK:
            DPRINT("Debug Joystick\n");
            transState(ST_DIAG_JOYSTICK);
            break;

         case M_EV_DIAG_DISPLAY:
            DPRINT("Debug Display\n");
            break;

         case M_EV_DIAG_LEDS:
            DPRINT("Debug LEDs\n");
            break;

         default:
            DPRINT("Unexpected menu event %d in top menus\n", ev);
            break;
      }
   }
}

// Handle events from the chess engine
static void engineEventHandler( int param )
{
   switch(param)
   {
      case ENG_MOVE:

         // TODO First check to see if board state needs fixing....

         // transition to state where human makes move for comptuter

         // Clocks won't move during this grace period...
         game.graceTime = options.game.graceTimeForComputerMove;


         transState(ST_HUMAN_MV_FOR_COMP);
         break;
      default:
         break;
   }
}

// Handle timer expirations..
static void timerEventHandler( int tmr )
{

   switch(tmr)
   {
      case TMR_UI_TIMEOUT:
         {
            switch(state)
            {
               case ST_INIT:
                  transState(ST_TOP_MENUS);
                  break;

               default:
                  break;
            }
         }
         break;

      case TMR_GAME_CLOCK_TIC:
         // Subtract 0.1 second from clock of player on move

         // TBD - any cases where timers may be frozen?

         if(game.graceTime != 0) game.graceTime--;

         else if(brd.toMove == WHITE)
         {
            if(game.wtime != 0)
            {
               game.wtime--;

               // If less than one minute OR tenths position moves from 0 to 9
               if(game.wtime < 600 || (game.wtime % 10) == 9)
               {
                  udpateGameDisplayClocks();
               }
            }
         }
         else
         {
            if(game.btime != 0)
            {
               game.btime--;
               if(game.btime < 600 || (game.btime % 10) == 9)
               {
                  udpateGameDisplayClocks();
               }
            }
         }
         break;

      case TMR_DIAG_TIMEOUT:
         if(state == ST_DIAG_JOYSTICK)
         {
            transState(ST_TOP_MENUS);
         }
         break;

      default:
         break;
   }
}

// Make state transitions
static void transState( event_t to)
{

   DPRINT("Moving from state %d [%s] to %d [%s]\n", state, stateNames[state], to, stateNames[to]);

   if(transTable.exit[state] != NULL_TRAN_EXIT_FUNC) transTable.exit[state](to);
   prevState = state;
   state = to;
   if(transTable.enter[to] !=NULL_TRAN_ENTER_FUNC)   transTable.enter[to](prevState);

}

///////////////////////
// Transition functions
///////////////////////

static void enterTopMenus( state_t from)
{

   // If we haven't already set this up...
   if(mainMenu == NULL)
   {
      mainMenu = createMenu("---- Main Menu -----", FALSE);

      //          menu      offset      text               press               right                  picker
      menuAddItem(mainMenu, ADD_TO_END, "Play Chess",       M_EV_START_GAME,    M_EV_IGNORED,          NULL);
      menuAddItem(mainMenu, ADD_TO_END, "Game Settings...", M_EV_GAME_SETTINGS, M_EV_GAME_SETTINGS,    NULL);
      menuAddItem(mainMenu, ADD_TO_END, "Setup Position",   M_EV_SETUP_POS,     M_EV_IGNORED,          NULL);
      menuAddItem(mainMenu, ADD_TO_END, "Load Game",        M_EV_LOAD_GAME,     M_EV_IGNORED,          NULL);
      menuAddItem(mainMenu, ADD_TO_END, "Diagnostics",      M_EV_DIAGNOSTICS,   M_EV_IGNORED,          NULL);
      menuAddItem(mainMenu, ADD_TO_END, "About",            M_EV_ABOUT,         M_EV_IGNORED,          NULL);
   }
   else
   {
      DPRINT("Resetting main menu\n");
      // Return to top item selected, displayed at top line
      menuReset(mainMenu);
   }

   // Put the menu on the screen
   drawMenu(mainMenu);

   // menu receives all button presses
   currentMenu = mainMenu;

}

static void enterInitialSetup( state_t from )
{
   displayWriteLine( 1, "Please set board", TRUE );
   displayWriteLine( 2, "to initial position", TRUE );
}

static void enterSetup( state_t from)
{
   selectedColor = WHITE;
   selectedPiece = KING;

   displayWriteLine(0, "Select piece to add:", TRUE);
   displayWriteLine(1, "    White King", FALSE);
   displayWriteLine(2, "Move pieces any time", TRUE);
   displayWriteLine(3, "Press when done", TRUE);
}

static void enterHumanMove( state_t from )
{
   // Figure out how the board will be affected by each move primitive...
   calculateMoveEffects(legalMoves, &brd, moveEffects, totalLegalMoves);
   drawGameScreen();

   // Zero out any remaining time...
   game.graceTime = 0;
}

static void enterHumanMoveForComp( state_t from)
{

   displayWriteLine( 0, "Comp. has chosen move", TRUE );
   displayWriteLine( 1, "Please move pieces as", TRUE );
   displayWriteLine( 2, "indicated by LEDs", TRUE );

   // TODO turn on LEDs to note computer's move...
}

static void enterComputerMove( state_t from )
{
   // TODO Call engine with current board state...
   drawGameScreen();
}

static void enterGameOver( state_t from)
{
   timerKill(TMR_GAME_CLOCK_TIC);
}

static void enterTestJoystick( state_t from )
{
   // We need this to indicate DownRight or UpLeft positions...
   defineCharacter(0, charBackslash);

   // Displayed already cleared from leaving main menus.  Only output lines that will change...
   displayWriteLine(1, "         +          ", FALSE);
   displayWriteLine(3, "Press 5 sec. to exit", FALSE);
}

static void exitInit( state_t to)
{
   // In case user pressed button to get out, kill the timer...
   timerKill(TMR_UI_TIMEOUT);
   displayClear();
}

static void exitTopMenus( state_t to)
{

   // Menu no longer has focus...
   currentMenu = NULL;

   // Remove menu from screen
   displayClear();
}

///////////////////////
// Menuu picker functions
///////////////////////

char* enableText = "On";
char* disableText = "Off";

char* openingBookPicker( int dir )   // 0 = return current, 1 = set/return next, -1 = set/return prev.
{

   if(dir == 0)
   {
      if(options.engine.openingBook == FALSE)
      {
         return disableText;
      }
      else
      {
         return enableText;
      }
   }
   else
   {
      if(options.engine.openingBook == FALSE)
      {
         options.engine.openingBook = TRUE;
         return enableText;
      }
      else
      {
         options.engine.openingBook = FALSE;
         return disableText;
      }
   }
}

///////////////////////
// Helper functions
///////////////////////

// Given the set of legal moves and the current position, calculate, for each move, the following:
//   (1) the final occupied positions on the board
//   (2) all squares which should see activity during the move
//
//   This information will be consulted as the human is making a move as a way to validate that he/she
//   appears to be making a legal move.
static void calculateMoveEffects(const move_t *moves, const board_t *brd, moveEffects_t *effects, int num)
{

   if(num <= 0)
   {
      DPRINT("Error: Trying to calculate move effects with zero (or negative) num\n");
      return;
   }

   // ASSUMPTION:  all supplied moves are valid for the given position.  We can assume, for instance,
   //  that if a king moved 2 squares sideways from the initial position, that it was a legal castle move.

   int indx = 0;

   while(num--)
   {
      // Start with no dirty squares
      effects[indx].dirtySquares =  0;

      // Start with final state = pre-move state
      effects[indx].occupiedSquares = brd->colors[BLACK] | brd->colors[WHITE];

      // Copy in the move
      effects[indx].move = moves[indx];

      // Update source and destination as "dirty"
      effects[indx].dirtySquares |= squareMask[moves[indx].from];
      effects[indx].dirtySquares |= squareMask[moves[indx].to];

      // Mark source as empty, destination as occupied
      effects[indx].occupiedSquares &= ~squareMask[moves[indx].from];
      effects[indx].occupiedSquares |=  squareMask[moves[indx].to];

      // Special condition #1: enpassant capture
      //  We need to account for fact that captured pawn will be removed,
      //  but it does not reside at the target square of the piece's move
      //
      // if moved piece was a pawn AND
      //   'from' col is different than 'to' col AND
      //   we moved to an empty square
      if(  (brd->pieces[PAWN] & squareMask[moves[indx].from])  &&
            ( (moves[indx].to % 8) != (moves[indx].from % 8) )  &&
            (  ( (brd->colors[WHITE] | brd->colors[BLACK]) & squareMask[moves[indx].to] ) == 0 ) )
      {
         // Update dirty square and occupied squares for captured pawn
         if(brd->toMove == WHITE)
         {
            effects[indx].dirtySquares    |=  squareMask[moves[indx].to + 8];
            effects[indx].occupiedSquares &= ~squareMask[moves[indx].to + 8];
         }
         else
         {
            effects[indx].dirtySquares    |=  squareMask[moves[indx].to - 8];
            effects[indx].occupiedSquares &= ~squareMask[moves[indx].to - 8];
         }
      }

      // TODO CHESS960
      // Special condition #2: castling
      //   Need to account for moving of the rook...
      //
      // Did white king just move 2 spaces left/right?
      if( moves[indx].from ==  E1 )
      {
         if(brd->pieces[KING] & e1)
         {
            if(moves[indx].to ==  C1)
            {
               effects[indx].dirtySquares    |= (a1 | d1);
               effects[indx].occupiedSquares |=  d1;
               effects[indx].occupiedSquares &= ~a1;
            }
            else if (moves[indx].to == G1)
            {
               effects[indx].dirtySquares    |= (f1 | h1);
               effects[indx].occupiedSquares |=  f1;
               effects[indx].occupiedSquares &= ~h1;
            }
         }
      }

      // Did black king just move 2 spaces left/right?
      else if (moves[indx].from == E8)
      {
         if(brd->pieces[KING] & e8)
         {
            if(moves[indx].to ==  C8)
            {
               effects[indx].dirtySquares    |= (a8 | d8);
               effects[indx].occupiedSquares |=  d8;
               effects[indx].occupiedSquares &= ~a8;
            }
            else if (moves[indx].to == G8)
            {
               effects[indx].dirtySquares    |= (f8 | h8);
               effects[indx].occupiedSquares |=  f8;
               effects[indx].occupiedSquares &= ~h8;
            }
         }
      }
      indx++;
   }
}

// Scan the list of "move effects" to see if the current board state (dirty squares and occupied squares) matches a
//   given move effect of a legal move OR indicates that one of these moves is in progress.
static moveVal_t checkValidMoveProgress(moveEffects_t *moveEffects, int numMoves, BB dirtySquares, BB occupiedSquares, move_t **ret)
{

   bool_t foundPrecursor = FALSE;
   int indx = 0;
   *ret = NULL;

   while(numMoves--)
   {
      // if exact match of dirtySquare and occupiedSquares are found, the move is complete
      if( (moveEffects[indx].occupiedSquares == occupiedSquares) && (moveEffects[indx].dirtySquares == dirtySquares) )
      {
         DPRINT("Found legal move %s\n", moveToSAN(moveEffects[indx].move, &brd));
         *ret = &moveEffects[indx].move;
         return MV_LEGAL;
      }

      // else if dirtySquares fall within a subset (or match) of any dirty square pattern, keep going
      else if ( (~moveEffects[indx].dirtySquares & dirtySquares) == 0 )
      {
         DPRINT("Found possible precursor to legal move %s\n", moveToSAN(moveEffects[indx].move, &brd));
         foundPrecursor = TRUE;
      }
      indx++;
   }

   // We didn't find an exact match.  Did we at least find a precursor?
   if(foundPrecursor)
   {
      return MV_PRECURSOR;
   }
   else
   {
      DPRINT("No legal moves (or precursors) found\n");
      return MV_ILLEGAL;
   }
}

static void startGame( void )
{
   lastMoveText[0] = '\0';

   game.startPos = NULL;
   game.brd = &brd;

   game.wtime = options.game.whiteTime;
   game.btime = options.game.blackTime;
   game.graceTime = 0;

   memset(&game.moves, 0x00, sizeof(game.moves));
   memset(&game.posHash, 0x00, sizeof(game.posHash));

   game.chess960 = options.game.chess960;

   if(game.wtime !=0 || game.btime != 0)
   {
      timerStart(TMR_GAME_CLOCK_TIC, 100, 100);
   }

   // If either or both player is the computer, set up pipe to stockfish Engine
   if( (options.game.white == PLAYER_COMPUTER) || (options.game.black == PLAYER_COMPUTER) )
   {
      SF_initEngine();
   }

   // Find all the legal moves from here
   totalLegalMoves = findMoves(&brd, legalMoves);

   // If none, position is a terminal state...
   if(totalLegalMoves <= 0)
   {
      transState(ST_GAME_OVER);
   }

   // Set our baseline...
   dirtySquares = 0;

   // Depending upon who is playing white, transition to appropriate state...
   if(options.game.white == PLAYER_HUMAN)
   {
      transState(ST_HUMAN_MV);
   }
   else
   {
      transState(ST_COMPUTER_MV);
   }

}

// Called after the computer or human player has selected their move
static void applyMove( move_t mv )
{

   // Put last move text into string that will be displayed...
   strcpy(lastMoveText, moveToSAN(mv, &brd));

   // Make the move...
   move(&brd, mv);

   // Find all the legal moves from here
   totalLegalMoves = findMoves(&brd, legalMoves);

   if(totalLegalMoves <= 0)
   {
      transState(ST_GAME_OVER);
   }
   else
   {

      // Set our baseline...
      dirtySquares = 0;

      if(brd.toMove == WHITE)
      {
         if(options.game.white == PLAYER_HUMAN)
         {
            transState(ST_HUMAN_MV);
         }
         else
         {
            transState(ST_COMPUTER_MV);
         }
      }
      else
      {
         if(options.game.black == PLAYER_HUMAN)
         {
            transState(ST_HUMAN_MV);
         }
         else
         {
            transState(ST_COMPUTER_MV);
         }
      }
   }
}

static void displaySplashScreen( void )
{
   // Splash screen....
   displayClear();
   displayWriteLine( 0, "piChess version 1.0", TRUE );
   displayWriteLine( 1, "by Todd Witters", TRUE );
   displayWriteLine( 2, "powered by", TRUE );

   // FIX - retrieve version info from engine itself?
   displayWriteLine( 3, "Stockfish6", TRUE );
}

// Draw the entire screen presented during the game
static void drawGameScreen( void )
{
   displayClear();

   // LINE 1
   updateGameDisplayBanner();

   // LINE 2
   updateGameDisplayLastMove();
   updateGameDisplayMaterial();

   // LINE 3
   updateGameDisplayClockLegend();

   // LINE 4
   udpateGameDisplayClocks();

}

// Takes a time value (1 bit = 1/10th second) at converts to a string...
static char *convertTimeToString (uint32_t tenths )
{
   // Time  <  1 minute  shown as      SS.S
   // Times  <  1 hour   shown as   MM:SS
   // Times  >= 1 hour   shows at H:MM:SS

   static char str[9];

   if( tenths < 600 )
   {
      sprintf(str,"0:%04.1f",
            (tenths / 10.0) );
   }
   else if (tenths < 36000)
   {
      sprintf(str, "%d:%02d",      tenths / 600,  (tenths % 600) / 10);
   }
   else
   {
      sprintf(str, "%d:%02d:%02d", tenths / 36000, (tenths % 36000) / 600, (tenths % 600) / 10);
   }

   return str;
}

// Update the clock times and "to move" marker
static void udpateGameDisplayClocks( void )
{
   const char *timeString;
   char fullString[10];

   memset(fullString, ' ', 10);
   timeString = convertTimeToString(game.wtime);
   memcpy(fullString, timeString, strlen(timeString));
   if(game.brd->toMove == WHITE)
   {
      defineCharacter(0, charLeftFilledArrow);
      fullString[strlen(timeString) + 1] = 0;
   }
   displayWriteChars(3, 0, 10, fullString);

   memset(fullString, ' ', 10);
   timeString = convertTimeToString(game.btime);
   memcpy(&fullString[10-strlen(timeString)], timeString, strlen(timeString));
   if(game.brd->toMove == BLACK)
   {
      defineCharacter(0, charRightFilledArrow);
      fullString[10 - strlen(timeString) - 2] = 0;
   }
   displayWriteChars(3, 10, 10, fullString);
}

static void updateGameDisplayBanner( void )
{
   if( game.brd->toMove == WHITE)
   {
      if( options.game.white == PLAYER_COMPUTER)
      {
         displayWriteLine(0, "Computer is thinking", TRUE);
      }
      else
      {
         displayWriteLine(0, "    White's move    ", TRUE);
      }
   }
   else
   {
      if( options.game.black == PLAYER_COMPUTER)
      {
         displayWriteLine(0, "Computer is thinking", TRUE);
      }
      else
      {
         displayWriteLine(0, "    Black's move    ", TRUE);
      }
   }

}

static void updateGameDisplayLastMove( void )
{
   char lastMoveString[13];

   memset(lastMoveString, ' ', 13);

   sprintf(&lastMoveString[0], "Last: ");
   if(lastMoveText != NULL && strlen(lastMoveText) <= 7)
   {
      sprintf(&lastMoveString[6], lastMoveText);
      lastMoveString[6+strlen(lastMoveText)] = ' ';
   }
   else
   {
      lastMoveString[6] = ' ';
   }
   displayWriteChars(1, 0, 13, lastMoveString);
}

static void updateGameDisplayMaterial( void )
{
   char materialString[6];

   sprintf(materialString,"%02d/%02d", game.brd->materialCount[WHITE], game.brd->materialCount[BLACK]);
   displayWriteChars(1, 15, 5, materialString);
}

static void updateGameDisplayClockLegend( void )
{
   if(options.game.white != options.game.black)
   {
      if(options.game.white == PLAYER_HUMAN)
      {
         displayWriteChars(2, 0, 20, "Human       Computer");
      }
      else
      {
         displayWriteChars(2, 0, 20, "Computer       Human");
      }
   }
   else
   {
      displayWriteChars(2, 0, 20, "White          Black");
   }
}
