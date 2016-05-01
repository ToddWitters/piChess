#include <stdlib.h>

#include "types.h"

typedef enum player_e
{
   PLAYER_HUMAN,
   PLAYER_COMPUTER
}player_t;

typedef struct gameOptions_s
{
   player_t      white;
   player_t      black;
   timeControl_t timeControl;
   bool_t        chess960;
   bool_t        useOpeningBook;            // ignored if chess960 == true
   uint16_t      graceTimeForComputerMove;  // ignored if no computer player
}gameOptions_t;

typedef struct engineOptions_s
{
   uint8_t strength;
   bool_t  ponder;
   bool_t  egtb;
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
