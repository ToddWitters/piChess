#include "book.h"
#include "types.h"
#include "board.h"
#include "moves.h"

#include "string.h"
#include "constants.h"
#include "diag.h"

#include <time.h>
#include <stdlib.h>
#include "bcm2835.h"

static FILE *bk = NULL;
U32 numEntries = 0;

static U32 findFirstKeyMatch( U64 val);
static U32 compareKey(U32 lower, U32 upper);
static void readPositionRecord(U32 offset, candidate_t *c);
static U64 reverseBytes( U64 input);
static void correctCastling(board_t *b, move_t *mv);

bookErr_t openBook( char *file )
{
   U32 numUnique = 0;

   U8 id[8];
   U8 record[16];

   FILE *temp = NULL;

   bookErr_t retVal = BOOK_NO_ERROR;

   // Seed random number once only when book is opened.
   srand(bcm2835_st_read());

   char filename[100];

   // If a book is already opened, keep track of it just in case new can't be opened
   if(bk != NULL) temp = bk;

   // Create name in books folder
   sprintf(filename, "/home/pi/chess/books/%s", file);

   DPRINT("Attempting to open %s\n", filename);

   // Point to this newly opened book
   bk = fopen(filename, "rb");

   // If we failed to open...
   if(bk == NULL)
   {
      DPRINT("Could not open file\n");
      // restore pointer
      bk = temp;
      return BOOK_FILE_NOT_FOUND;
   }

   // If we had kept a temporary
   if(temp != NULL)
   {
      fclose(temp);
      retVal = BOOK_REPLACED;
   }

   // Get the number of entries by examining the size of the file
   fseek(bk, 0, SEEK_END);
   numEntries = ftell(bk) / 16;
   rewind(bk);

   // GATHER STATS...

   // Set the id to something that won't match the first entry...
   memset(id, 0xFF, 8);

   // Scan for differences between adjacent entries...
   while( fread(record, 1, 16, bk) == 16)
   {
      if(memcmp(id, record, 8)) numUnique++;
      memcpy(id, record, 8);
   }

   DPRINT("%d records, %d unique positions\n", numEntries, numUnique);

   return retVal;
}

bool_t isBookOpen( void )
{
    if(bk == NULL) return FALSE;
    else return TRUE;
}

bookErr_t closeBook( void )
{

    if(bk == NULL) return BOOK_ALREADY_CLOSED;

    fclose(bk);

    bk = NULL;

    return BOOK_NO_ERROR;
}

bookErr_t listBookMoves( board_t *b )
{
   U32 firstMatch;
   U32 original;

   U32 totalWeight = 0;

   candidate_t c;

   if(!isBookOpen()) return BOOK_NOT_OPEN;

   original = firstMatch = findFirstKeyMatch( b->hash );

   if(firstMatch == 0xFFFFFFFF) return BOOK_POSITION_NOT_FOUND;

   // Get total of all the weights for all available moves
   while(1)
   {
      readPositionRecord(firstMatch++, &c);

      // If we've stepped outside the range of matches for this id, we're done..
      if(c.hash != b->hash) break;

      // Accumulate.
      totalWeight += c.weight;
   }

   // now scan and print data for each entry...
   while(1)
   {
      // Extract the data
      readPositionRecord(original++, &c);

      // make sure we're still in the block that matches this id
      if(c.hash != b->hash) break;

      // Need to correct polygot format of king "capturing" rook on castling moves
      correctCastling(b, &c.mv);

      // Show this move and its weight relative to the total.
      DPRINT("%4.1f%% %s\n", (100.0 * (float)c.weight)/(float)totalWeight, moveToSAN(c.mv, b));
   }

   return BOOK_NO_ERROR;

}

bookErr_t getBestMove  ( board_t *b, move_t *mv )
{
   U32 firstMatch;
   U32 bestPos;

   U16 best = 0;

   candidate_t c;


   if(!isBookOpen()) return BOOK_NOT_OPEN;

   firstMatch = findFirstKeyMatch( b->hash );

   if(firstMatch == 0xFFFFFFFF) return BOOK_POSITION_NOT_FOUND;

   while(1)
   {
      // Read the new record
      readPositionRecord(firstMatch++, &c);

      // If we have stepped outside of the range of matches, get out
      if(c.hash != b->hash) break;

      // If we have a new best, update best and remember our location
      if(c.weight > best)
      {
         best = c.weight;
         bestPos = firstMatch-1;
      }
   }

   // Grab the data back out for the best
   readPositionRecord(bestPos, &c);

   // Need to correct polygot format of king "capturing" rook on castling moves
   correctCastling(b, &c.mv);

   // prepare the return value...
   memcpy(mv, &c.mv, sizeof(move_t));

   return BOOK_NO_ERROR;

}

bookErr_t getRandMove  ( board_t *b, move_t *mv )
{
   U32 firstMatch;
   U32 original;
   U32 totalWeight = 0;

   candidate_t c;

   // Generate a random number
   int r = rand();

   // Check error conditions first...
   if(!isBookOpen()) return BOOK_NOT_OPEN;

   original = firstMatch = findFirstKeyMatch( b->hash );

   if(firstMatch == 0xFFFFFFFF) return BOOK_POSITION_NOT_FOUND;

   while(1)
   {
      // Get the data
      readPositionRecord(firstMatch++, &c);

      // Make sure we are still in range...
      if(c.hash != b->hash) break;

      // Accumulate total weight.
      totalWeight += c.weight;
   }

   // Create a random number from 1 to total weight
   r %= totalWeight;
   r++;

   // Scan until our acculuated weight lands in a "bin"
   while(1)
   {
      readPositionRecord(original++, &c);

      if(c.weight >= r) break;

      r -= c.weight;

   }

   // Need to correct polygot format of king "capturing" rook on castling moves
   correctCastling(b, &c.mv);

   memcpy(mv, &c.mv, sizeof(move_t));

   return BOOK_NO_ERROR;
}



static U32 mid;
static U64 key;
static U64 thisKey;

// Entry into binary sort
static U32 findFirstKeyMatch( U64 val)
{
   key = val;

   return compareKey(0, numEntries - 1);
}

// Resursive sort function
static U32 compareKey(U32 lower, U32 upper)
{
   mid = (lower + upper) / 2;

   // Pull out the first 8 bytes (the hash value);
   fseek(bk, mid * RECORD_SIZE + KEY_OFFSET , SEEK_SET);
   fread(&thisKey, 1, 8 , bk);

   // Stored MSB/LSB, so reverse the bytes...
   thisKey = reverseBytes(thisKey);

   // Do we have a match?
   if(thisKey == key)
   {
      // YES!  Work backwards to find the first
      while(mid--)
      {
         fseek(bk, mid * RECORD_SIZE + KEY_OFFSET, SEEK_SET);
         fread(&thisKey, 1, 8 , bk);

         // reverse the byte ordering
         thisKey = reverseBytes(thisKey);

         // if we backed up too far, return the previous location
         if(thisKey != key) return mid + 1;
      }

      return 0;
   }

   // If no hit, either search upper or lower half accordingly
   else if (thisKey > key)
   {
      // can't narrow any further - no match available.
      if(lower == mid) return 0xFFFFFFFF;

      // Recurse, moving in closer
      else return compareKey( lower, mid-1 );
   }
   else
   {
      // can't narrow any further - no match available.
      if(upper == mid) return 0xFFFFFFFF;

      // Recurse, moving in closer
      else return compareKey( mid+1, upper );
   }
}


// Reverse byte order of a U64
static U64 reverseBytes( U64 input)
{
    int i;
    U64 retValue = 0;

    for(i=0;i<8;i++)
    {
        retValue <<= 8;
        retValue |= *((U8 *)(&input)+i);
    }

    return retValue;
}


// Extract data from record in binary
static void readPositionRecord(U32 offset, candidate_t *c)
{

   U8 toFile;
   U8 toRow;
   U8 fromFile;
   U8 fromRow;
   U8 promotion;
   U8 bytes[8];

   // move to desired location
   fseek(bk, offset * RECORD_SIZE , SEEK_SET);

   // Get the hash
   fread(bytes, 1, 8 , bk);
   c->hash = reverseBytes( *((U64 *)(&bytes)));

   // The next two bytes are the move information...
   fread(bytes, 1, 2 , bk);

   // Pick apart the bits....
   toFile   =   bytes[1] & 0x07;
   toRow    =  (bytes[1] & 0x38) >> 3;
   fromFile = ((bytes[1] & 0xC0) >> 6) | ( (bytes[0] & 0x01) << 2);
   fromRow  =  (bytes[0] & 0x0E) >> 1;
   promotion = (bytes[0] & 0x70) >> 4;

   c->mv.to   = toFile   + (7 - toRow)   * 8;
   c->mv.from = fromFile + (7 - fromRow) * 8;

   if(promotion == 0)
   {
      c->mv.promote = PIECE_NONE;
   }
   else
   {
      c->mv.promote = (piece_t)promotion + KNIGHT;
   }

   // Get the weight of this move
   fread(bytes, 1, 2, bk);
   c->weight = bytes[0] * 256 + bytes[1];

   // IGNORE THE LEARN DATA...
}

// polyglot format uses an unusal notation for castling.  Indicates a king to move to rook's square.
//   we use king moving left or right two spaces, so make the adjustment if necessary...
static void correctCastling(board_t *b, move_t *mv)
{

   // If from square is white king's square...
   if( mv->from == E1)
   {
      // ... and white king is still there (AND white is on move)...
      if( b->pieces[KING] & b->colors[b->toMove] & squareMask[E1] )
      {
         if ( mv->to == H1)
            mv->to = G1;
         else if (mv->to == A1)
            mv->to = C1;
      }
   }

   else if( mv->from == E8)
   {
      if( b->pieces[KING] & b->colors[b->toMove] & squareMask[E8] )
      {
         if( mv->to == H8)
            mv->to = G8;
         else if(mv->to == A8)
            mv->to = C8;
      }
   }
}
