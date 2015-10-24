#include <stdlib.h>

#include "types.h"

typedef enum player_e
{
   PLAYER_HUMAN,
   PLAYER_COMPUTER
}player_t;

typedef struct gameOptions_s
{
   player_t white;
   player_t black;

   uint32_t whiteTime;
   uint16_t whiteTimeInc;

   uint32_t blackTime;
   uint16_t blackTimeInc;

   bool_t   chess960;

   uint16_t graceTimeForComputerMove;
}gameOptions_t;

typedef struct engineOptions_s
{
   uint8_t strength;
   bool_t  ponder;
   bool_t  egtb;
   bool_t  openingBook;
}engineOptions_t;

typedef struct boardOptions_s
{
   uint16_t pieceDropDebounce;
   uint16_t pieceLiftDebounce;
   uint8_t  LED_Brightness;
}boardOptions_t;

typedef struct options_s
{
   gameOptions_t     game;
   boardOptions_t   board;
   engineOptions_t engine;
}options_t;

extern options_t options;


void loadOptions( options_t *options );
void saveOptions( const options_t *options );
