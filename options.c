#include "options.h"
#include "diag.h"
#include "timer.h"

static void setDefaultOptions( options_t *options );
static void validateOptions( options_t *options );

options_t options;

void loadOptions( options_t *options )
{

   DPRINT("Loading Options\n");

   setDefaultOptions( options );

   // FIX this is temporary to prevent compiler warnings
   validateOptions( options );
}

void saveOptions( const options_t *options )
{

}

static void setDefaultOptions( options_t *options )
{

   DPRINT("Setting all options to their default values\n");

   options->game.white        = PLAYER_HUMAN;
   options->game.black        = PLAYER_COMPUTER;
   options->game.whiteTime    = 3000; // in 0.1 second steps...
   options->game.blackTime    = 3000; // in 0.1 second steps...
   options->game.whiteTimeInc = 20;
   options->game.blackTimeInc = 20;
   options->game.chess960     = FALSE;
   options->game.graceTimeForComputerMove = 30; // allow 3 seconds to make move for computer

   options->board.pieceDropDebounce = (600 / MS_PER_TIC);
   options->board.pieceLiftDebounce = (150 / MS_PER_TIC);
   options->board.LED_Brightness    = 15;

   options->engine.strength         = 20;
   options->engine.ponder           = FALSE;
   options->engine.egtb             = FALSE;
   options->engine.openingBook      = FALSE;
}

static void validateOptions( options_t *options )
{
   if( (options->game.white != PLAYER_HUMAN) && (options->game.white != PLAYER_COMPUTER) )
   {
      options->game.white = PLAYER_HUMAN;
   }

   if( (options->game.black != PLAYER_HUMAN) && (options->game.black != PLAYER_COMPUTER) )
   {
      options->game.black = PLAYER_HUMAN;
   }

   if( (options->game.chess960 != TRUE) && (options->game.chess960 != FALSE) )
   {
      options->game.chess960 = FALSE;
   }

   if( (options->game.graceTimeForComputerMove > 100) )
   {
      options->game.graceTimeForComputerMove = 100;
   }

   if(options->board.pieceDropDebounce > 20)
   {
      options->board.pieceDropDebounce = 20;
   }

   if(options->board.pieceLiftDebounce > 5)
   {
      options->board.pieceLiftDebounce = 5;
   }

   if(options->board.LED_Brightness > 15)
   {
      options->board.LED_Brightness = 15;
   }

   if(options->engine.strength > 21)
   {
      options->engine.strength = 21;
   }

   if( (options->engine.ponder != TRUE) && (options->engine.ponder != FALSE) )
   {
      options->engine.ponder = FALSE;
   }

   if( (options->engine.egtb != TRUE) && (options->engine.egtb != FALSE) )
   {
      options->engine.egtb = FALSE;
   }

   if( (options->engine.openingBook != TRUE) && (options->engine.openingBook != FALSE) )
   {
      options->engine.openingBook = FALSE;
   }
}
