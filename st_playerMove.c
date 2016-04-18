#include "hsm.h"
#include "hsmDefs.h"
#include "st_playerMove.h"

#include "types.h"
#include "constants.h"

#include "moves.h"
#include "st_inGame.h"
#include "switch.h"
#include "diag.h"
#include "display.h"
#include "event.h"
#include "led.h"
#include "st_playingGame.h"
#include "st_inGame.h"

#include <stddef.h>

typedef struct moveEffect_s
{
   move_t move;          // A legal move

   // The following can be used to determine when the player has made a valid move.
   BB dirtySquares;      // Bitboard of squares that should see lifts/drops for the given move
   BB occupiedSquares;   // Bitboard of the occupied squares at the conclusion of the move
}moveEffects_t;

typedef enum moveVal_t
{
   MV_ILLEGAL,    // Move primitive is not part of any legal move
   MV_PRECURSOR,  // Move primitive is a precursor to one or more legal moves.
   MV_LEGAL       // Move primitive has completed a legal move
}moveVal_t;

// List of legal moves (and their effects) for this position (computed on entry to player moving state)
static move_t        legalMoves[200];
static moveEffects_t moveEffects[200];

// size of previous lists
static int totalLegalMoves = 0;

static moveVal_t checkValidMoveProgress(moveEffects_t *moveEffects, int numMoves, BB dirtySquares, BB occupiedSquares, move_t **ret);
static void calculateMoveEffects(const move_t *moves, const board_t *brd, moveEffects_t *effects, int num);

static uint64_t occupiedSquares, dirtySquares;

void playerMoveEntry( event_t ev )
{

   displayClear();
   if(game.brd.toMove == WHITE)
   {
      displayWriteLine(0, "White's Move", TRUE);
   }
   else
   {
      displayWriteLine(0, "Black's Move", TRUE);
   }

   inGame_udpateClocks();


   // Find all the legal moves from here
   totalLegalMoves = findMoves(&game.brd, legalMoves);

   // Figure out the move primitives for all legal moves...
   calculateMoveEffects(legalMoves, &game.brd, moveEffects, totalLegalMoves);

   // Even though we aren't responding to piece event, we still need
   //  to set the LEDs accordingly....
   occupiedSquares = GetSwitchStates();
   dirtySquares    = occupiedSquares ^ (game.brd.colors[WHITE] | game.brd.colors[BLACK]);

}

void playerMoveExit( event_t ev )
{
   // Leave LEDs on in case we are going to the in-game menu state
}

void playerMoves_boardChange( event_t ev)
{
   move_t     *moveMade;
   moveVal_t  moveProgress;

   occupiedSquares = GetSwitchStates();

   dirtySquares |= squareMask[ev.data];

   dirtySquares &= (game.brd.colors[WHITE] | game.brd.colors[BLACK] | occupiedSquares);

   if(occupiedSquares == (game.brd.colors[WHITE] | game.brd.colors[BLACK])) dirtySquares = 0;

   moveProgress = checkValidMoveProgress(moveEffects, totalLegalMoves, dirtySquares, occupiedSquares, &moveMade);

   LED_AllOff();

   switch(moveProgress)
   {
      case MV_LEGAL:
         playingGame_processSelectedMove(*moveMade);
         break;

      case MV_PRECURSOR:
         LED_SetGridState(dirtySquares);
         break;

      case MV_ILLEGAL:
         LED_FlashGridState(dirtySquares);
         break;
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
         DPRINT("Found legal move %s\n", moveToSAN(moveEffects[indx].move, &game.brd));
         *ret = &moveEffects[indx].move;
         return MV_LEGAL;
      }

      // else if dirtySquares fall within a subset (or match) of any dirty square pattern, keep going
      else if ( (~moveEffects[indx].dirtySquares & dirtySquares) == 0 )
      {
         DPRINT("Found possible precursor to legal move %s\n", moveToSAN(moveEffects[indx].move, &game.brd));
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
