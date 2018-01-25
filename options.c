#include "options.h"
#include "diag.h"
#include "timer.h"

static void setDefaultOptions( options_t *options );
static void validateOptions( options_t *options );

options_t options;

void loadOptions( options_t *options )
{

   DPRINT("Loading Options\n");

   // TODO:  Load options from file as textual key/value pairs
   setDefaultOptions( options );

   // FIX this is temporary to prevent compiler warnings
   validateOptions( options );
}

void saveOptions( const options_t *options )
{
   // Dump hash table to file?
}

static void setDefaultOptions( options_t *options )
{

   DPRINT("Setting all options to their default values\n");

   options->game.white                                 = PLAYER_HUMAN;
   options->game.black                                 = PLAYER_COMPUTER;

   options->game.timeControl.type = TIME_NONE;
   options->game.timeControl.compStrategySetting.type  = STRAT_FIXED_DEPTH;
   options->game.timeControl.compStrategySetting.depth = 12;

   options->game.timeControl.timeSettings[0].totalTime = 3*60;
   options->game.timeControl.timeSettings[0].increment = 0;
   options->game.timeControl.timeSettings[0].moves     = 0;

   options->game.timeControl.timeSettings[1].totalTime = 3*60;
   options->game.timeControl.timeSettings[1].increment = 0;
   options->game.timeControl.timeSettings[1].moves     = 0;

   options->game.timeControl.timeSettings[2].totalTime = 3*60;
   options->game.timeControl.timeSettings[2].increment = 0;
   options->game.timeControl.timeSettings[2].moves     = 0;

   options->game.chess960                              = FALSE;
   options->game.graceTimeForComputerMove              = 40; // allow 4 seconds to make move for computer
   options->game.useOpeningBook                        = FALSE;

   options->board.pieceDropDebounce                    = (600 / MS_PER_TIC);
   options->board.pieceLiftDebounce                    = (100 / MS_PER_TIC);
   options->board.LED_Brightness                       = 15;

   options->engine.strength                            = 20;
   options->engine.ponder                              = FALSE;
   options->engine.egtb                                = FALSE;
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

   if(options->engine.strength > 20)
   {
      options->engine.strength = 20;
   }

   if( (options->engine.ponder != TRUE) && (options->engine.ponder != FALSE) )
   {
      options->engine.ponder = FALSE;
   }

   if( (options->engine.egtb != TRUE) && (options->engine.egtb != FALSE) )
   {
      options->engine.egtb = FALSE;
   }

   if( (options->game.useOpeningBook != TRUE) && (options->game.useOpeningBook != FALSE) )
   {
      options->game.useOpeningBook = FALSE;
   }
}
