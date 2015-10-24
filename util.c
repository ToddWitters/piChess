#include "util.h"
#include "diag.h"
#include "types.h"

char *convertSqNumToCoord(int sq)
{
   static char coord[3];

   if(sq>63)
   {
      DPRINT("ERROR: Invalid square number in convertSqNumToCoord\n");
      coord[0] = coord[1] = '?';
   }
   else
   {
      coord[0] = 'a' + sq % 8;
      coord[1] = '8' - sq / 8;
   }

   coord[2] = '\0';
   return coord;
}

move_t convertCoordMove( char *coord )
{

   static move_t mv;

   mv.from    = 0;
   mv.to      = 0;
   mv.promote = (unsigned short)PIECE_NONE;

   if( coord[0] < 'a' || coord[0] > 'h' ||
       coord[1] < '1' || coord[1] > '8' ||
       coord[2] < 'a' || coord[2] > 'h' ||
       coord[3] < '1' || coord[3] > '8' )
   {
      DPRINT("Parse Error 1:  Selected computer move is has unexpected format\n");
      return mv;
   }

   mv.from = 8 * ('8' - coord[1]) + (coord[0] - 'a');
   mv.to   = 8 * ('8' - coord[3]) + (coord[2] - 'a');

   switch(coord[4])
   {
      case 'n': mv.promote = KNIGHT; break;
      case 'b': mv.promote = BISHOP; break;
      case 'r': mv.promote = ROOK;   break;
      case 'q': mv.promote = QUEEN;  break;
   }

   return mv;
}
