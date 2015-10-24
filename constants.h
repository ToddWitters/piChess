#include "types.h"

#define A8 0
#define B8 1
#define C8 2
#define D8 3
#define E8 4
#define F8 5
#define G8 6
#define H8 7

#define A7 8
#define B7 9
#define C7 10
#define D7 11
#define E7 12
#define F7 13
#define G7 14
#define H7 15

#define A6 16
#define B6 17
#define C6 18
#define D6 19
#define E6 20
#define F6 21
#define G6 22
#define H6 23

#define A5 24
#define B5 25
#define C5 26
#define D5 27
#define E5 28
#define F5 29
#define G5 30
#define H5 31

#define A4 32
#define B4 33
#define C4 34
#define D4 35
#define E4 36
#define F4 37
#define G4 38
#define H4 39

#define A3 40
#define B3 41
#define C3 42
#define D3 43
#define E3 44
#define F3 45
#define G3 46
#define H3 47

#define A2 48
#define B2 49
#define C2 50
#define D2 51
#define E2 52
#define F2 53
#define G2 54
#define H2 55

#define A1 56
#define B1 57
#define C1 58
#define D1 59
#define E1 60
#define F1 61
#define G1 62
#define H1 63

#define a1  ((BB)0x0000000000000080)
#define a2  ((BB)0x0000000000008000)
#define a3  ((BB)0x0000000000800000)
#define a4  ((BB)0x0000000080000000)
#define a5  ((BB)0x0000008000000000)
#define a6  ((BB)0x0000800000000000)
#define a7  ((BB)0x0080000000000000)
#define a8  ((BB)0x8000000000000000)

#define b1  ((BB)0x0000000000000040)
#define b2  ((BB)0x0000000000004000)
#define b3  ((BB)0x0000000000400000)
#define b4  ((BB)0x0000000040000000)
#define b5  ((BB)0x0000004000000000)
#define b6  ((BB)0x0000400000000000)
#define b7  ((BB)0x0040000000000000)
#define b8  ((BB)0x4000000000000000)

#define c1  ((BB)0x0000000000000020)
#define c2  ((BB)0x0000000000002000)
#define c3  ((BB)0x0000000000200000)
#define c4  ((BB)0x0000000020000000)
#define c5  ((BB)0x0000002000000000)
#define c6  ((BB)0x0000200000000000)
#define c7  ((BB)0x0020000000000000)
#define c8  ((BB)0x2000000000000000)

#define d1  ((BB)0x0000000000000010)
#define d2  ((BB)0x0000000000001000)
#define d3  ((BB)0x0000000000100000)
#define d4  ((BB)0x0000000010000000)
#define d5  ((BB)0x0000001000000000)
#define d6  ((BB)0x0000100000000000)
#define d7  ((BB)0x0010000000000000)
#define d8  ((BB)0x1000000000000000)

#define e1  ((BB)0x0000000000000008)
#define e2  ((BB)0x0000000000000800)
#define e3  ((BB)0x0000000000080000)
#define e4  ((BB)0x0000000008000000)
#define e5  ((BB)0x0000000800000000)
#define e6  ((BB)0x0000080000000000)
#define e7  ((BB)0x0008000000000000)
#define e8  ((BB)0x0800000000000000)

#define f1  ((BB)0x0000000000000004)
#define f2  ((BB)0x0000000000000400)
#define f3  ((BB)0x0000000000040000)
#define f4  ((BB)0x0000000004000000)
#define f5  ((BB)0x0000000400000000)
#define f6  ((BB)0x0000040000000000)
#define f7  ((BB)0x0004000000000000)
#define f8  ((BB)0x0400000000000000)

#define g1  ((BB)0x0000000000000002)
#define g2  ((BB)0x0000000000000200)
#define g3  ((BB)0x0000000000020000)
#define g4  ((BB)0x0000000002000000)
#define g5  ((BB)0x0000000200000000)
#define g6  ((BB)0x0000020000000000)
#define g7  ((BB)0x0002000000000000)
#define g8  ((BB)0x0200000000000000)

#define h1  ((BB)0x0000000000000001)
#define h2  ((BB)0x0000000000000100)
#define h3  ((BB)0x0000000000010000)
#define h4  ((BB)0x0000000001000000)
#define h5  ((BB)0x0000000100000000)
#define h6  ((BB)0x0000010000000000)
#define h7  ((BB)0x0001000000000000)
#define h8  ((BB)0x0100000000000000)



extern const BB squareMask[64];
extern const BB knightCoverage[64];
extern const BB kingCoverage[64];

extern const BB lightSquares;
extern const BB darkSquares;

extern const int pieceValue[5];

extern const BB rowMask[8];
extern const BB fileMask[8];

extern const BB ray[8][64];
