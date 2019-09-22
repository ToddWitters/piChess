#include "options.h"
#include "diag.h"
#include "timer.h"
#include "hashtable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

static void setDefaultOptions( void );

options_t options;
hashtable_t *optionsHashTable = NULL;

bool disableSaveOnChange = false;

void loadOptions( void )
{
   if(optionsHashTable == NULL)
      optionsHashTable = ht_create(50);

   // Start with defaults
   setDefaultOptions();

   // Load in options file if it exists...will overwrite defaults
   ht_load(optionsHashTable, "options");

   // Save result back....
   ht_save(optionsHashTable, "options");

}

static void setDefaultOptions( void )
{

   disableSaveOnChange = true;
   setOptionVal("whitePlayer", PLAYER_HUMAN);
   setOptionVal("blackPlayer", PLAYER_COMPUTER);
   setOptionVal("timeControl", TIME_NONE);
   setOptionStr("computerStrategy", "fixedDepth");
   setOptionVal("searchDepth", 12);
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

   options.game.white                                 = PLAYER_HUMAN;
   options.game.black                                 = PLAYER_COMPUTER;

   options.game.timeControl.type = TIME_NONE;
   options.game.timeControl.compStrategySetting.type  = STRAT_FIXED_DEPTH;
   options.game.timeControl.compStrategySetting.depth = 12;

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

int getOptionVal(char *opt)
{
   return atoi(ht_getKey(optionsHashTable, opt));
}

void setOptionVal(char *opt, int val)
{
   char str[20];

   sprintf(str, "%d", val);

   ht_setKey(optionsHashTable, opt, str);

   if(!disableSaveOnChange)
      ht_save(optionsHashTable, "options");
}

bool isOptionVal(char *opt, int val)
{
   return (val == atoi(ht_getKey(optionsHashTable, opt)));
}


