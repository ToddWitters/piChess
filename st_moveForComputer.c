#include <string.h>

#include "hsm.h"
#include "hsmDefs.h"
#include "st_moveForComputer.h"

#include "st_inGame.h"
#include "st_playingGame.h"
#include "display.h"
#include "led.h"
#include "switch.h"
#include "event.h"
#include "constants.h"
#include "options.h"
#include "moves.h"
#include "bitboard.h"
#include "st_fixBoard.h"
#include "board.h"
#include "diag.h"

uint64_t occupiedSquares;
extern bool_t computerMovePending;
uint64_t mustMove;
extern game_t game;

static uint8_t boardChangeCount;

static void evaluateNextAction( void );

board_t prevBoard;

void moveForComputerEntry( event_t ev )
{

   // Figure out the position prior to the selected move, and if we didn't enter from there, go to
   //   the fix board state...
   memcpy(&prevBoard, &game.brd, sizeof(board_t));
   unmove(&prevBoard, game.posHistory[game.playedMoves-1].revMove);

   if( game.posHistory[game.playedMoves - 1].revMove.captured != PIECE_NONE)
      mustMove = squareMask[game.posHistory[game.playedMoves - 1].move.to]; 
   else
      mustMove = 0;   
 
   // If we arrived here with the move already made (from FIX_BOARD), see where to go next...
   if(GetSwitchStates() == (game.brd.colors[WHITE] | game.brd.colors[BLACK]))
   {
      evaluateNextAction();
   }
   else
   {
      if(GetSwitchStates() != (prevBoard.colors[WHITE] | prevBoard.colors[BLACK]))
      {
         event_t ev;

         fixBoard_setDirty(mustMove);
         ev.ev = EV_FIX_BOARD;
         putEvent(EVQ_EVENT_MANAGER, &ev);
   
      }
      else
      {
         displayWriteLine(0, "Make indicated move", TRUE);
         LED_SetGridState( (GetSwitchStates() ^ (game.brd.colors[WHITE] | game.brd.colors[BLACK])) | mustMove);
         game.graceTime = options.game.graceTimeForComputerMove;
         boardChangeCount = 0;

      }
   }  
}

void moveForComputerExit( event_t ev )
{
   // displayClear();
   LED_AllOff();
}

void moveForComputer_boardChange( event_t ev)
{
   uint64_t current = GetSwitchStates();

   boardChangeCount++;



   // If a square is marked "mustMove", udate that once a piece is lifted...
   if( (ev.ev = EV_PIECE_LIFT) && (squareMask[ev.data] == mustMove) )
      mustMove = 0;
 

   // If everything is where it belongs...
   if( mustMove == 0 && (current == (game.brd.colors[WHITE] | game.brd.colors[BLACK])))
      evaluateNextAction();

   // If we have moved things around too much OR there are more pieces on the board than there should be...
   else if(boardChangeCount > 5 || 
           bitCount(current) > bitCount(game.brd.colors[WHITE] | game.brd.colors[BLACK]) )
   {
      event_t ev;

      fixBoard_setDirty(mustMove);
      ev.ev = EV_FIX_BOARD;
      putEvent(EVQ_EVENT_MANAGER, &ev);
   }

   else
   {
      uint64_t dirty, expectedDirty;

      dirty = current ^ (game.brd.colors[WHITE] | game.brd.colors[BLACK]);

      // Compare this board with previous position to see which squares we expect to be different.
      expectedDirty = (game.brd.colors[WHITE] | game.brd.colors[BLACK]) ^ (prevBoard.colors[WHITE] | prevBoard.colors[BLACK]);
      
      // If we captured, 
      if( game.posHistory[game.playedMoves - 1].revMove.captured != PIECE_NONE)
         expectedDirty |= squareMask[game.posHistory[game.playedMoves - 1].move.to]; 


      DPRINT("Expected Dirty: %016llX\n", expectedDirty );
      DPRINT("Dirty: %016llX\n", dirty );

      if(dirty & ~expectedDirty)
      {
         event_t ev;

         fixBoard_setDirty(mustMove);
         ev.ev = EV_FIX_BOARD;
         putEvent(EVQ_EVENT_MANAGER, &ev);

      }
      else
      {
         // Highlight the squares that still need attention
         LED_SetGridState( (current ^ (game.brd.colors[WHITE] | game.brd.colors[BLACK])) | mustMove);  
      }
   }
}

static void evaluateNextAction( void )
{
     event_t event;

      int totalMovesFound = findMoves(&game.brd , NULL);

      // If there are no legal moves left...
      if( totalMovesFound <= 0 )
      {
         event.ev = EV_GAME_DONE;

         if(totalMovesFound == CHECKMATE)
         {
            game.disposition = GAME_AT_CHECKMATE;
            event.data = GAME_END_CHECKMATE;
         }
         else
         {
            game.disposition = GAME_AT_STALEMATE;
            event.data = GAME_END_STALEMATE;
         }
      }
      else
      {
         game.graceTime = 0;
         event.ev = EV_PLAYER_MOVED_FOR_COMP;
      }

      putEvent(EVQ_EVENT_MANAGER, &event);
      computerMovePending = FALSE;
}
