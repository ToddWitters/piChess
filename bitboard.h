#include "types.h"

/// Count number of bits in a bitboard
int bitCount (BB x);

/// find index of least significant bit set
int getLSBindex(U64 bb);

/// find index of most significant bit set
int getMSBindex(U64 bb);

/// Fills bit pattern towards North.  Usefull for pawn evaluation.
BB Nfill(BB gen);

/// Fills bit pattern towards North.  Usefull for pawn evaluation.
BB Sfill(BB gen);

/// Create bitboard of all North attack squares from given bitboard positions
BB Nattacks(BB osliders, BB empty);

/// Create bitboard of all South attack squares from given bitboard positions
BB Sattacks(BB osliders, BB empty);

/// Create bitboard of all East attack squares from given bitboard positions
BB Eattacks(BB osliders, BB empty);

/// Create bitboard of all West attack squares from given bitboard positions
BB Wattacks(BB osliders, BB empty);

/// Create bitboard of all NorthEast attack squares from given bitboard positions
BB NEattacks(BB osliders, BB empty);

/// Create bitboard of all Northwest attack squares from given bitboard positions
BB NWattacks(BB osliders, BB empty);

/// Create bitboard of all SouthEast attack squares from given bitboard positions
BB SEattacks(BB osliders, BB empty);

/// Create bitboard of all SouthWest attack squares from given bitboard positions
BB SWattacks(BB osliders, BB empty);


/// Mask to remove A-file positions from a bitboard
#define notAfile 0x7F7F7F7F7F7F7F7F

/// Mask to remove H-file positions from a bitboard
#define notHfile 0xFEFEFEFEFEFEFEFE

/// Move all bits in bitboard 1 position South
#define shiftS(X) ((X) >> 8)
/// Move all bits in bitboard 1 position North
#define shiftN(X) ((X) << 8)
/// Move all bits in bitboard 1 position East
#define shiftE(X) (((X) >> 1) & notAfile)
/// Move all bits in bitboard 1 position West
#define shiftW(X) (((X) << 1) & notHfile)

/// Move all bits in bitboard 1 position NorthEast
#define shiftNE(X) (((X) << 7) & notAfile)
/// Move all bits in bitboard 1 position NorthWest
#define shiftNW(X) (((X) << 9) & notHfile)
/// Move all bits in bitboard 1 position SouthEast
#define shiftSE(X) (((X) >> 9) & notAfile)
/// Move all bits in bitboard 1 position SouthWest
#define shiftSW(X) (((X) >> 7) & notHfile)

/// Find least significant bit of X (X must be non-zero)
#define lsb(X) ((X) & -(X))
// Clears the least signficant bit of X (X must be non-zero)
#define clearlsb(X) (X = (X & (X-1)))
