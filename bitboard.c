#include "bitboard.h"
#include "types.h"
#include "debug.h"

// Count number of bits in a bitboard...
int bitCount (BB x)
{

	const U64 k1 = 0x5555555555555555; /*  -1/3   */
	const U64 k2 = 0x3333333333333333; /*  -1/5   */
	const U64 k4 = 0x0f0f0f0f0f0f0f0f; /*  -1/17  */
	const U64 kf = 0x0101010101010101; /*  -1/255 */

    x =  x       - ((x >> 1)  & k1); /* put count of each 2 bits into those 2 bits */
    x = (x & k2) + ((x >> 2)  & k2); /* put count of each 4 bits into those 4 bits */
    x = (x       +  (x >> 4)) & k4 ; /* put count of each 8 bits into those 8 bits */
    x = (x * kf) >> 56; /* returns 8 most significant bits of x + (x<<8) + (x<<16) + (x<<24) + ...  */
    return (int) x;
}


// Magic number used for getLSBindex
static const U64 debruijn64 = 0x03f79d71b4cb0a89;

// Magic array used for getLSBindex
static const int index64[64] = {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};

/**
 * bitScanForward (renamed getLSBindex)
 * @author Martin Läuter (1997)
 *         Charles E. Leiserson
 *         Harald Prokop
 *         Keith H. Randall
 * "Using de Bruijn Sequences to Index a 1 in a Computer Word"
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
int getLSBindex(U64 bb) {
   ASSERT(bb != 0);
   return index64[((bb & -bb) * debruijn64) >> 58];
}

///////////

const int reverseIndex64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

/**
 * bitScanReverse (renamed getMSBindex)
 * @authors Kim Walisch, Mark Dickinson
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of most significant one bit
 */
int getMSBindex(U64 bb) {
   assert (bb != 0);
   bb |= bb >> 1;
   bb |= bb >> 2;
   bb |= bb >> 4;
   bb |= bb >> 8;
   bb |= bb >> 16;
   bb |= bb >> 32;
   return reverseIndex64[(bb * debruijn64) >> 58];
}

///////////

// Fills bit pattern towards North/South.  Usefull for pawn evaluation.
BB Nfill(BB gen) {
   gen |= (gen <<  8);
   gen |= (gen << 16);
   gen |= (gen << 32);
   return gen;
}

BB Sfill(BB gen) {
   gen |= (gen >>  8);
   gen |= (gen >> 16);
   gen |= (gen >> 32);
   return gen;
}


BB Sattacks(BB osliders, BB empty) {
   BB flood = osliders;
   flood |= osliders = (osliders >> 8) & empty;
   flood |= osliders = (osliders >> 8) & empty;
   flood |= osliders = (osliders >> 8) & empty;
   flood |= osliders = (osliders >> 8) & empty;
   flood |= osliders = (osliders >> 8) & empty;
   flood |=            (osliders >> 8) & empty;
   return            flood >> 8;
}

BB Nattacks(BB osliders, BB empty) {
   BB flood = osliders;
   flood |= osliders = (osliders << 8) & empty;
   flood |= osliders = (osliders << 8) & empty;
   flood |= osliders = (osliders << 8) & empty;
   flood |= osliders = (osliders << 8) & empty;
   flood |= osliders = (osliders << 8) & empty;
   flood |=         (osliders << 8) & empty;
   return            flood << 8;
}

BB Eattacks(BB osliders, BB empty) {

   BB flood = osliders;

   empty &= notAfile;

   flood |= osliders = (osliders >> 1) & empty;
   flood |= osliders = (osliders >> 1) & empty;
   flood |= osliders = (osliders >> 1) & empty;
   flood |= osliders = (osliders >> 1) & empty;
   flood |= osliders = (osliders >> 1) & empty;
   flood |=         (osliders >> 1) & empty;

   return           (flood >> 1) & notAfile ;
}

BB Wattacks(BB osliders, BB empty) {
   BB flood = osliders;
   empty &= notHfile;
   flood |= osliders = (osliders << 1) & empty;
   flood |= osliders = (osliders << 1) & empty;
   flood |= osliders = (osliders << 1) & empty;
   flood |= osliders = (osliders << 1) & empty;
   flood |= osliders = (osliders << 1) & empty;
   flood |=         (osliders << 1) & empty;
   return           (flood << 1) & notHfile ;
}

BB NWattacks(BB vsliders, BB empty) {
   BB flood = vsliders;
   empty &= notHfile;
   flood |= vsliders = (vsliders << 9) & empty;
   flood |= vsliders = (vsliders << 9) & empty;
   flood |= vsliders = (vsliders << 9) & empty;
   flood |= vsliders = (vsliders << 9) & empty;
   flood |= vsliders = (vsliders << 9) & empty;
   flood |=            (vsliders << 9) & empty;
   return               (flood << 9) & notHfile ;
}

BB SWattacks(BB vsliders, BB empty) {
   BB flood = vsliders;
   empty &= notHfile;
   flood |= vsliders = (vsliders >> 7) & empty;
   flood |= vsliders = (vsliders >> 7) & empty;
   flood |= vsliders = (vsliders >> 7) & empty;
   flood |= vsliders = (vsliders >> 7) & empty;
   flood |= vsliders = (vsliders >> 7) & empty;
   flood |=           (vsliders >> 7) & empty;
   return               (flood >> 7) & notHfile ;
}

BB SEattacks(BB vsliders, BB empty) {
   BB flood = vsliders;
   empty &= notAfile;
   flood |= vsliders = (vsliders >> 9) & empty;
   flood |= vsliders = (vsliders >> 9) & empty;
   flood |= vsliders = (vsliders >> 9) & empty;
   flood |= vsliders = (vsliders >> 9) & empty;
   flood |= vsliders = (vsliders >> 9) & empty;
   flood |=           (vsliders >> 9) & empty;
   return               (flood >> 9) & notAfile ;
}

BB NEattacks(BB vsliders, BB empty) {
   BB flood = vsliders;
   empty &= notAfile;
   flood |= vsliders = (vsliders << 7) & empty;
   flood |= vsliders = (vsliders << 7) & empty;
   flood |= vsliders = (vsliders << 7) & empty;
   flood |= vsliders = (vsliders << 7) & empty;
   flood |= vsliders = (vsliders << 7) & empty;
   flood |=            (vsliders << 7) & empty;
   return               (flood << 7) & notAfile ;
}
