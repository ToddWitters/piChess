#include "options.h"
#include "diag.h"
#include "timer.h"
#include "hashtable.h"
#include "engine.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

static void checkOptions( void );

options_t options;
hashtable_t *optionsHashTable = NULL;

bool disableSaveOnChange = false;

void loadOptions( void )
{
   if(optionsHashTable == NULL)
      optionsHashTable = ht_create(50);

   ht_load(optionsHashTable, "options");

   // Ensure every option exists and has legal values
   checkOptions();

   // Save results
   ht_save(optionsHashTable, "options");

}

static void checkOptions( void )
{

   disableSaveOnChange = true;
   long int depth;


   if(
       getOptionStr("whitePlayer") == NULL ||
       (
         !isOptionStr("whitePlayer","human") &&
         !isOptionStr("whitePlayer","computer")
       )
     )
   {
      setOptionStr("whitePlayer", "human");
   }

   if(
       getOptionStr("blackPlayer") == NULL ||
       (
         !isOptionStr("blackPlayer","human") &&
         !isOptionStr("blackPlayer","computer")
       )
     )
   {
      setOptionStr("blackPlayer", "computer");
   }

   if(
       getOptionStr("timeControl") == NULL ||
       (
         !isOptionStr("timeControl", "untimed") &&
         !isOptionStr("timeControl", "equal")   &&
         !isOptionStr("timeControl", "odds")
       )
     )
   {
      setOptionStr("timeControl", "untimed");
   }


   if(
       getOptionStr("computerStrategy") == NULL ||
       (
         !isOptionStr("computerStrategy", "fixedDepth")  &&
         !isOptionStr("computerStrategy", "fixedTime")   &&
         !isOptionStr("computerStrategy", "tillButton")
       )
     )
   {
      setOptionStr("computerStrategy", "fixedDepth");
   }


   depth = getOptionVal("searchDepth");
   if(
       getOptionStr("searchDepth") == NULL ||
       (
         depth > MAX_PLY_DEPTH ||
         depth < MIN_PLY_DEPTH
       )
     )
   {
      setOptionVal("searchDepth", DEFAULT_PLY_DEPTH );
   }

   setOptionVal("searchTimeInMs", 3000);
   setOptionVal("timePeriod1.timeInSec", 180);
   setOptionVal("timePeriod1.increment", 0);
   setOptionVal("timePeriod1.moves", 0);
   setOptionVal("timePeriod2.timeInSec", 180);
   setOptionVal("timePeriod2.increment", 0);
   setOptionVal("timePeriod2.moves", 0);
   setOptionVal("timePeriod3.timeInSec", 180);
   setOptionVal("timePeriod3.increment", 0);
   setOptionVal("timePeriod3.moves", 0);
   setOptionStr("chess960", "false");
   setOptionVal("graceTimeForComputerMoveInSec", 4);
   setOptionStr("openingBook", "true");
   setOptionStr("coaching", "true");
   setOptionStr("takeBack", "true");
   setOptionVal("dropDebounceInTicks", (600 / MS_PER_TIC));
   setOptionVal("liftDebounceInTicks", (100 / MS_PER_TIC));
   setOptionVal("ledBrightness", 15);
   setOptionVal("engineStrength", 20);
   setOptionStr("ponder", "false");

   disableSaveOnChange = false;


//////////

   DPRINT("Setting all options to their default values\n");

//   options.game.white                                 = PLAYER_HUMAN;
//   options.game.black                                 = PLAYER_COMPUTER;

//   options.game.timeControl.type = TIME_NONE;
//   options.game.timeControl.compStrategySetting.type  = STRAT_FIXED_DEPTH;
//   options.game.timeControl.compStrategySetting.depth = 12;

   options.game.timeControl.timeSettings[0].totalTime = 3*60;
   options.game.timeControl.timeSettings[0].increment = 0;
   options.game.timeControl.timeSettings[0].moves     = 0;

   options.game.timeControl.timeSettings[1].totalTime = 3*60;
   options.game.timeControl.timeSettings[1].increment = 0;
   options.game.timeControl.timeSettings[1].moves     = 0;

   options.game.timeControl.timeSettings[2].totalTime = 3*60;
   options.game.timeControl.timeSettings[2].increment = 0;
   options.game.timeControl.timeSettings[2].moves     = 0;

   options.game.chess960                              = FALSE;
   options.game.graceTimeForComputerMove              = 40; // allow 4 seconds to make move for computer
   options.game.useOpeningBook                        = FALSE;

   options.board.pieceDropDebounce                    = (600 / MS_PER_TIC);
   options.board.pieceLiftDebounce                    = (100 / MS_PER_TIC);
   options.board.LED_Brightness                       = 15;

//   options.engine.strength                            = 20;
   options.engine.ponder                              = FALSE;
   options.engine.egtb                                = FALSE;

}

char *getOptionStr(char *opt)
{
   return ht_getKey(optionsHashTable, opt);
}

void setOptionStr(char *opt, char *val)
{
   ht_setKey(optionsHashTable, opt, val);

   if(!disableSaveOnChange)
      ht_save(optionsHashTable, "options");
}

bool isOptionStr(char *opt, char *val)
{
   return(!strcmp(val, ht_getKey(optionsHashTable, opt)));
}

long int getOptionVal(char *opt)
{
   char *resultPtr;

   resultPtr = ht_getKey(optionsHashTable, opt);

   if(resultPtr == NULL)
      return 0;
   else
      return strtol(resultPtr, NULL, 10);
}

void setOptionVal(char *opt, long int val)
{
   char str[20];

   sprintf(str, "%ld", val);

   ht_setKey(optionsHashTable, opt, str);

   if(!disableSaveOnChange)
      ht_save(optionsHashTable, "options");
}

bool isOptionVal(char *opt, int val)
{
   char *resultPtr;

   resultPtr = ht_getKey(optionsHashTable, opt);

   if(resultPtr == NULL)
      return false;
   else
      return (val == strtol(resultPtr, NULL, 10));
}


