// CONSTANTS
#include "constants.h"

// The baseline for a piece count (in centipawns) for PAWN, KNIGHT, BISHOP, ROOK, QUEEN
const int pieceValue[5] = {1, 3, 3, 5, 9};

// Bitmasks for each square number 0-63
const BB squareMask[64] =
{
   a8,b8,c8,d8,e8,f8,g8,h8,
   a7,b7,c7,d7,e7,f7,g7,h7,
   a6,b6,c6,d6,e6,f6,g6,h6,
   a5,b5,c5,d5,e5,f5,g5,h5,
   a4,b4,c4,d4,e4,f4,g4,h4,
   a3,b3,c3,d3,e3,f3,g3,h3,
   a2,b2,c2,d2,e2,f2,g2,h2,
   a1,b1,c1,d1,e1,f1,g1,h1
};

// Bitmask for the eight files
const BB fileMask[8] =
{
	a1|a2|a3|a4|a5|a6|a7|a8 ,
	b1|b2|b3|b4|b5|b6|b7|b8 ,
	c1|c2|c3|c4|c5|c6|c7|c8 ,
	d1|d2|d3|d4|d5|d6|d7|d8 ,
	e1|e2|e3|e4|e5|e6|e7|e8 ,
	f1|f2|f3|f4|f5|f6|f7|f8 ,
	g1|g2|g3|g4|g5|g6|g7|g8 ,
	h1|h2|h3|h4|h5|h6|h7|h8
};

// Bitmask for the eight rows
const BB rowMask[8] =
{
	a8|b8|c8|d8|e8|f8|g8|h8 ,
	a7|b7|c7|d7|e7|f7|g7|h7 ,
	a6|b6|c6|d6|e6|f6|g6|h6 ,
	a5|b5|c5|d5|e5|f5|g5|h5 ,
	a4|b4|c4|d4|e4|f4|g4|h4 ,
	a3|b3|c3|d3|e3|f3|g3|h3 ,
	a2|b2|c2|d2|e2|f2|g2|h2 ,
	a1|b1|c1|d1|e1|f1|g1|h1 ,
};

const BB lightSquares =

	a8|c8|e8|g8|
	b7|d7|f7|h7|
	a6|c6|e6|g6|
	b5|d5|f5|h5|
	a4|c4|e4|g4|
	b3|d3|f3|h3|
	a2|c2|e2|g2|
	b1|d1|f1|h1;

const BB darkSquares =

	b8|d8|f8|h8|
	a7|c7|e7|g7|
	b6|d6|f6|h6|
	a5|c5|e5|g5|
	b4|d4|f4|h4|
	a3|c3|e3|g3|
	b2|d2|f2|h2|
   a1|c1|e1|g1;

// King coverage square masks for all 64 squares
const BB kingCoverage[64] =
{
	(          a7 | b7 | b8 ) , // a8
	(a7 | a8 | b7 | c7 | c8 ) , // b8
	(b7 | b8 | c7 | d7 | d8 ) , // c8
	(c7 | c8 | d7 | e7 | e8 ) , // d8
	(d7 | d8 | e7 | f7 | f8 ) , // e8
	(e7 | e8 | f7 | g7 | g8 ) , // f8
	(f7 | f8 | g7 | h7 | h8 ) , // g8
	(g7 | g8 | h7           ) , // h8


	(               a8 | a6 | b8 | b7 | b6), // a7
	(a6 | a7 | a8 | b8 | b6 | c8 | c7 | c6), // b7
	(b6 | b7 | b8 | c8 | c6 | d8 | d7 | d6), // c7
	(c6 | c7 | c8 | d8 | d6 | e8 | e7 | e6), // d7
	(d6 | d7 | d8 | e8 | e6 | f8 | f7 | f6), // e7
	(e6 | e7 | e8 | f8 | f6 | g8 | g7 | g6), // f7
	(f6 | f7 | f8 | g8 | g6 | h8 | h7 | h6), // g7
	(g6 | g7 | g8 | h8 | h6               ), // h7

	(               a7 | a5 | b7 | b6 | b5), // a6
	(a5 | a6 | a7 | b7 | b5 | c7 | c6 | c5), // b6
	(b5 | b6 | b7 | c7 | c5 | d7 | d6 | d5), // c6
	(c5 | c6 | c7 | d7 | d5 | e7 | e6 | e5), // d6
	(d5 | d6 | d7 | e7 | e5 | f7 | f6 | f5), // e6
	(e5 | e6 | e7 | f7 | f5 | g7 | g6 | g5), // f6
	(f5 | f6 | f7 | g7 | g5 | h7 | h6 | h5), // g6
	(g5 | g6 | g7 | h7 | h5               ), // h6

	(               a6 | a4 | b6 | b5 | b4), // a5
	(a4 | a5 | a6 | b6 | b4 | c6 | c5 | c4), // b5
	(b4 | b5 | b6 | c6 | c4 | d6 | d5 | d4), // c5
	(c4 | c5 | c6 | d6 | d4 | e6 | e5 | e4), // d5
	(d4 | d5 | d6 | e6 | e4 | f6 | f5 | f4), // e5
	(e4 | e5 | e6 | f6 | f4 | g6 | g5 | g4), // f5
	(f4 | f5 | f6 | g6 | g4 | h6 | h5 | h4), // g5
	(g4 | g5 | g6 | h6 | h4               ), // h5

	(               a5 | a3 | b5 | b4 | b3), // a4
	(a3 | a4 | a5 | b5 | b3 | c5 | c4 | c3), // b4
	(b3 | b4 | b5 | c5 | c3 | d5 | d4 | d3), // c4
	(c3 | c4 | c5 | d5 | d3 | e5 | e4 | e3), // d4
	(d3 | d4 | d5 | e5 | e3 | f5 | f4 | f3), // e4
	(e3 | e4 | e5 | f5 | f3 | g5 | g4 | g3), // f4
	(f3 | f4 | f5 | g5 | g3 | h5 | h4 | h3), // g4
	(g3 | g4 | g5 | h5 | h3               ), // h4

	(               a4 | a2 | b4 | b3 | b2), // a3
	(a2 | a3 | a4 | b4 | b2 | c4 | c3 | c2), // b3
	(b2 | b3 | b4 | c4 | c2 | d4 | d3 | d2), // c3
	(c2 | c3 | c4 | d4 | d2 | e4 | e3 | e2), // d3
	(d2 | d3 | d4 | e4 | e2 | f4 | f3 | f2), // e3
	(e2 | e3 | e4 | f4 | f2 | g4 | g3 | g2), // f3
	(f2 | f3 | f4 | g4 | g2 | h4 | h3 | h2), // g3
	(g2 | g3 | g4 | h4 | h2               ), // h3

	(               a3 | a1 | b3 | b2 | b1), // a2
	(a1 | a2 | a3 | b3 | b1 | c3 | c2 | c1), // b2
	(b1 | b2 | b3 | c3 | c1 | d3 | d2 | d1), // c2
	(c1 | c2 | c3 | d3 | d1 | e3 | e2 | e1), // d2
	(d1 | d2 | d3 | e3 | e1 | f3 | f2 | f1), // e2
	(e1 | e2 | e3 | f3 | f1 | g3 | g2 | g1), // f2
	(f1 | f2 | f3 | g3 | g1 | h3 | h2 | h1), // g2
	(g1 | g2 | g3 | h3 | h1               ), // h2

	(          a2 | b2 | b1), // a1
	(a1 | a2 | b2 | c2 | c1), // b1
	(b1 | b2 | c2 | d2 | d1), // c1
	(c1 | c2 | d2 | e2 | e1), // d1
	(d1 | d2 | e2 | f2 | f1), // e1
	(e1 | e2 | f2 | g2 | g1), // f1
	(f1 | f2 | g2 | h2 | h1), // g1
	(g1 | g2 | h2          )  // h1
};

// Provides a bitmask of knight coverage for each of the 64 squares....
const BB knightCoverage[64] =
{
		(c7 | b6          ), // a8
	   (d7 | c6 | a6     ), // b8
		(e7 | d6 | b6 | a7), // c8
		(f7 | e6 | c6 | b7), // d8
		(g7 | f6 | d6 | c7), // e8
		(h7 | g6 | e6 | d7), // f8
		(     h6 | f6 | e7), // g8
		(          g6 | f7), // h8

		(c6 | c8 | b5               ), // a7
	   (d6 | d8 | c5 | a5          ), // b7
		(e6 | e8 | d5 | b5 | a6 | a8), // c7
		(f6 | f8 | e5 | c5 | b6 | b8), // d7
		(g6 | g8 | f5 | d5 | c6 | c8), // e7
		(h6 | h8 | g5 | e5 | d6 | d8), // f7
		(	        h5 | f5 | e6 | e8), // g7
		(               g5 | f6 | f8), // h7

		(c5 | c7 | b4 | b8                    ), // a6
	   (d5 | d7 | c4 | c8 | a4 | a8          ), // b6
		(e5 | e7 | d4 | d8 | b4 | b8 | a5 | a7), // c6
		(f5 | f7 | e4 | e8 | c4 | c8 | b5 | b7), // d6
		(g5 | g7 | f4 | f8 | d4 | d8 | c5 | c7), // e6
		(h5 | h7 | g4 | g8 | e4 | e8 | d5 | d7), // f6
		(          h4 | h8 | f4 | f8 | e5 | e7), // g6
		(                    g4 | g8 | f5 | f7), // h6

		(c4 | c6 | b3 | b7                    ), // a5
	   (d4 | d6 | c3 | c7 | a3 | a7          ), // b5
		(e4 | e6 | d3 | d7 | b3 | b7 | a4 | a6), // c5
		(f4 | f6 | e3 | e7 | c3 | c7 | b4 | b6), // d5
		(g4 | g6 | f3 | f7 | d3 | d7 | c4 | c6), // e5
		(h4 | h6 | g3 | g7 | e3 | e7 | d4 | d6), // f5
		(          h3 | h7 | f3 | f7 | e4 | e6), // g5
		(                    g3 | g7 | f4 | f6), // h5

		(c3 | c5 | b2 | b6                    ), // a4
	   (d3 | d5 | c2 | c6 | a2 | a6          ), // b4
		(e3 | e5 | d2 | d6 | b2 | b6 | a3 | a5), // c4
		(f3 | f5 | e2 | e6 | c2 | c6 | b3 | b5), // d4
		(g3 | g5 | f2 | f6 | d2 | d6 | c3 | c5), // e4
		(h3 | h5 | g2 | g6 | e2 | e6 | d3 | d5), // f4
		(          h2 | h6 | f2 | f6 | e3 | e5), // g4
		(                    g2 | g6 | f3 | f5), // h4

		(c2 | c4 | b1 | b5                    ), // a3
	   (d2 | d4 | c1 | c5 | a1 | a5          ), // b3
		(e2 | e4 | d1 | d5 | b1 | b5 | a2 | a4), // c3
		(f2 | f4 | e1 | e5 | c1 | c5 | b2 | b4), // d3
		(g2 | g4 | f1 | f5 | d1 | d5 | c2 | c4), // e3
		(h2 | h4 | g1 | g5 | e1 | e5 | d2 | d4), // f3
		(          h1 | h5 | f1 | f5 | e2 | e4), // g3
		(                    g1 | g5 | f2 | f4), // h3

		(c1 | c3 | b4               ), // a2
	   (d1 | d3 | c4 | a4          ), // b2
		(e1 | e3 | d4 | b4 | a1 | a3), // c2
		(f1 | f3 | e4 | c4 | b1 | b3), // d2
		(g1 | g3 | f4 | d4 | c1 | c3), // e2
		(h1 | h3 | g4 | e4 | d1 | d3), // f2
		(          h4 | f4 | e1 | e3), // g2
		(               g4 | f1 | f3), // h2

		(c2 | b3          ), // a1
	   (d2 | c3 | a3     ), // b1
		(e2 | d3 | b3 | a2), // c1
		(f2 | e3 | c3 | b2), // d1
		(g2 | f3 | d3 | c2), // e1
		(h2 | g3 | e3 | d2), // f1
		(     h3 | f3 | e2), // g1
		(          g3 | f2), // h1
};


// Precalculated ray coverage in eight directions....
const BB ray[8][64] =
{

{
	// NORTH

	(0), // a8
	(0), // b8
	(0), // c8
	(0), // d8
	(0), // e8
	(0), // f8
	(0), // g8
	(0), // h8

	(a8), // a7
	(b8), // b7
	(c8), // c7
	(d8), // d7
	(e8), // e7
	(f8), // f7
	(g8), // g7
	(h8), // h7

	(a8 | a7), // a6
	(b8 | b7), // b6
	(c8 | c7), // c6
	(d8 | d7), // d6
	(e8 | e7), // e6
	(f8 | f7), // f6
	(g8 | g7), // g6
	(h8 | h7), // h6

	(a8 | a7 | a6), // a5
	(b8 | b7 | b6), // b5
	(c8 | c7 | c6), // c5
	(d8 | d7 | d6), // d5
	(e8 | e7 | e6), // e5
	(f8 | f7 | f6), // f5
	(g8 | g7 | g6), // g5
	(h8 | h7 | h6), // h5

	(a8 | a7 | a6 | a5), // a4
	(b8 | b7 | b6 | b5), // b4
	(c8 | c7 | c6 | c5), // c4
	(d8 | d7 | d6 | d5), // d4
	(e8 | e7 | e6 | e5), // e4
	(f8 | f7 | f6 | f5), // f4
	(g8 | g7 | g6 | g5), // g4
	(h8 | h7 | h6 | h5), // h4

	(a8 | a7 | a6 | a5 | a4), // a3
	(b8 | b7 | b6 | b5 | b4), // b3
	(c8 | c7 | c6 | c5 | c4), // c3
	(d8 | d7 | d6 | d5 | d4), // d3
	(e8 | e7 | e6 | e5 | e4), // e3
	(f8 | f7 | f6 | f5 | f4), // f3
	(g8 | g7 | g6 | g5 | g4), // g3
	(h8 | h7 | h6 | h5 | h4), // h3

	(a8 | a7 | a6 | a5 | a4 | a3), // a2
	(b8 | b7 | b6 | b5 | b4 | b3), // b2
	(c8 | c7 | c6 | c5 | c4 | c3), // c2
	(d8 | d7 | d6 | d5 | d4 | d3), // d2
	(e8 | e7 | e6 | e5 | e4 | e3), // e2
	(f8 | f7 | f6 | f5 | f4 | f3), // f2
	(g8 | g7 | g6 | g5 | g4 | g3), // g2
	(h8 | h7 | h6 | h5 | h4 | h3), // h2

	(a8 | a7 | a6 | a5 | a4 | a3 | a2), // a1
	(b8 | b7 | b6 | b5 | b4 | b3 | b2), // b1
	(c8 | c7 | c6 | c5 | c4 | c3 | c2), // c1
	(d8 | d7 | d6 | d5 | d4 | d3 | d2), // d1
	(e8 | e7 | e6 | e5 | e4 | e3 | e2), // e1
	(f8 | f7 | f6 | f5 | f4 | f3 | f2), // f1
	(g8 | g7 | g6 | g5 | g4 | g3 | g2), // g1
	(h8 | h7 | h6 | h5 | h4 | h3 | h2)  // h1

},

{

	// SOUTH

	(a7 | a6 | a5 | a4 | a3 | a2 | a1), // a8
	(b7 | b6 | b5 | b4 | b3 | b2 | b1), // b8
	(c7 | c6 | c5 | c4 | c3 | c2 | c1), // c8
	(d7 | d6 | d5 | d4 | d3 | d2 | d1), // d8
	(e7 | e6 | e5 | e4 | e3 | e2 | e1), // e8
	(f7 | f6 | f5 | f4 | f3 | f2 | f1), // f8
	(g7 | g6 | g5 | g4 | g3 | g2 | g1), // g8
	(h7 | h6 | h5 | h4 | h3 | h2 | h1), // h8

	(a6 | a5 | a4 | a3 | a2 | a1), // a7
	(b6 | b5 | b4 | b3 | b2 | b1), // b7
	(c6 | c5 | c4 | c3 | c2 | c1), // c7
	(d6 | d5 | d4 | d3 | d2 | d1), // d7
	(e6 | e5 | e4 | e3 | e2 | e1), // e7
	(f6 | f5 | f4 | f3 | f2 | f1), // f7
	(g6 | g5 | g4 | g3 | g2 | g1), // g7
	(h6 | h5 | h4 | h3 | h2 | h1), // h7

	(a5 | a4 | a3 | a2 | a1), // a6
	(b5 | b4 | b3 | b2 | b1), // b6
	(c5 | c4 | c3 | c2 | c1), // c6
	(d5 | d4 | d3 | d2 | d1), // d6
	(e5 | e4 | e3 | e2 | e1), // e6
	(f5 | f4 | f3 | f2 | f1), // f6
	(g5 | g4 | g3 | g2 | g1), // g6
	(h5 | h4 | h3 | h2 | h1), // h6

	(a4 | a3 | a2 | a1), // a5
	(b4 | b3 | b2 | b1), // b5
	(c4 | c3 | c2 | c1), // c5
	(d4 | d3 | d2 | d1), // d5
	(e4 | e3 | e2 | e1), // e5
	(f4 | f3 | f2 | f1), // f5
	(g4 | g3 | g2 | g1), // g5
	(h4 | h3 | h2 | h1), // h5

	(a3 | a2 | a1), // a4
	(b3 | b2 | b1), // b4
	(c3 | c2 | c1), // c4
	(d3 | d2 | d1), // d4
	(e3 | e2 | e1), // e4
	(f3 | f2 | f1), // f4
	(g3 | g2 | g1), // g4
	(h3 | h2 | h1), // h4

	(a2 | a1), // a3
	(b2 | b1), // b3
	(c2 | c1), // c3
	(d2 | d1), // d3
	(e2 | e1), // e3
	(f2 | f1), // f3
	(g2 | g1), // g3
	(h2 | h1), // h3

	(a1), // a2
	(b1), // b2
	(c1), // c2
	(d1), // d2
	(e1), // e2
	(f1), // f2
	(g1), // g2
	(h1), // h2

	(0), // a1
	(0), // b1
	(0), // c1
	(0), // d1
	(0), // e1
	(0), // f1
	(0), // g1
	(0)  // h1
},

{

	// EAST

	(b8 | c8 | d8 | e8 | f8 | g8 | h8), // a8
	(c8 | d8 | e8 | f8 | g8 | h8),      // b8
	(d8 | e8 | f8 | g8 | h8),           // c8
	(e8 | f8 | g8 | h8),                // d8
	(f8 | g8 | h8),                     // e8
	(g8 | h8),                          // f8
	(h8),                               // g8
	(0),                                // h8

	(b7 | c7 | d7 | e7 | f7 | g7 | h7), // a7
	(c7 | d7 | e7 | f7 | g7 | h7),      // b7
	(d7 | e7 | f7 | g7 | h7),           // c7
	(e7 | f7 | g7 | h7),                // d7
	(f7 | g7 | h7),                     // e7
	(g7 | h7),                          // f7
	(h7),                               // g7
	(0),                                // h7

	(b6 | c6 | d6 | e6 | f6 | g6 | h6), // a6
	(c6 | d6 | e6 | f6 | g6 | h6),      // b6
	(d6 | e6 | f6 | g6 | h6),           // c6
	(e6 | f6 | g6 | h6),                // d6
	(f6 | g6 | h6),                     // e6
	(g6 | h6),                          // f6
	(h6),                               // g6
	(0),                                // h6

	(b5 | c5 | d5 | e5 | f5 | g5 | h5), // a5
	(c5 | d5 | e5 | f5 | g5 | h5),      // b5
	(d5 | e5 | f5 | g5 | h5),           // c5
	(e5 | f5 | g5 | h5),                // d5
	(f5 | g5 | h5),                     // e5
	(g5 | h5),                          // f5
	(h5),                               // g5
	(0),                                // h5

	(b4 | c4 | d4 | e4 | f4 | g4 | h4), // a4
	(c4 | d4 | e4 | f4 | g4 | h4),      // b4
	(d4 | e4 | f4 | g4 | h4),           // c4
	(e4 | f4 | g4 | h4),                // d4
	(f4 | g4 | h4),                     // e4
	(g4 | h4),                          // f4
	(h4),                               // g4
	(0),                                // h4

	(b3 | c3 | d3 | e3 | f3 | g3 | h3), // a3
	(c3 | d3 | e3 | f3 | g3 | h3),      // b3
	(d3 | e3 | f3 | g3 | h3),           // c3
	(e3 | f3 | g3 | h3),                // d3
	(f3 | g3 | h3),                     // e3
	(g3 | h3),                          // f3
	(h3),                               // g3
	(0),                                // h3

	(b2 | c2 | d2 | e2 | f2 | g2 | h2), // a2
	(c2 | d2 | e2 | f2 | g2 | h2),      // b2
	(d2 | e2 | f2 | g2 | h2),           // c2
	(e2 | f2 | g2 | h2),                // d2
	(f2 | g2 | h2),                     // e2
	(g2 | h2),                          // f2
	(h2),                               // g2
	(0),                                // h2

	(b1 | c1 | d1 | e1 | f1 | g1 | h1), // a1
	(c1 | d1 | e1 | f1 | g1 | h1),      // b1
	(d1 | e1 | f1 | g1 | h1),           // c1
	(e1 | f1 | g1 | h1),                // d1
	(f1 | g1 | h1),                     // e1
	(g1 | h1),                          // f1
	(h1),                               // g1
	(0)                                 // h1
},

{

	// WEST

	(0),                                // a8
	(a8),                               // b8
	(a8 | b8),                          // c8
	(a8 | b8 | c8),                     // d8
	(a8 | b8 | c8 | d8),                // e8
	(a8 | b8 | c8 | d8 | e8),           // f8
	(a8 | b8 | c8 | d8 | e8 | f8),      // g8
	(a8 | b8 | c8 | d8 | e8 | f8 | g8), // h8

	(0),                                // a7
	(a7),                               // b7
	(a7 | b7),                          // c7
	(a7 | b7 | c7),                     // d7
	(a7 | b7 | c7 | d7),                // e7
	(a7 | b7 | c7 | d7 | e7),           // f7
	(a7 | b7 | c7 | d7 | e7 | f7),      // g7
	(a7 | b7 | c7 | d7 | e7 | f7 | g7), // h7

	(0),                                // a6
	(a6),                               // b6
	(a6 | b6),                          // c6
	(a6 | b6 | c6),                     // d6
	(a6 | b6 | c6 | d6),                // e6
	(a6 | b6 | c6 | d6 | e6),           // f6
	(a6 | b6 | c6 | d6 | e6 | f6),      // g6
	(a6 | b6 | c6 | d6 | e6 | f6 | g6), // h6

	(0),                                // a5
	(a5),                               // b5
	(a5 | b5),                          // c5
	(a5 | b5 | c5),                     // d5
	(a5 | b5 | c5 | d5),                // e5
	(a5 | b5 | c5 | d5 | e5),           // f5
	(a5 | b5 | c5 | d5 | e5 | f5),      // g5
	(a5 | b5 | c5 | d5 | e5 | f5 | g5), // h5

	(0),                                // a4
	(a4),                               // b4
	(a4 | b4),                          // c4
	(a4 | b4 | c4),                     // d4
	(a4 | b4 | c4 | d4),                // e4
	(a4 | b4 | c4 | d4 | e4),           // f4
	(a4 | b4 | c4 | d4 | e4 | f4),      // g4
	(a4 | b4 | c4 | d4 | e4 | f4 | g4), // h4

	(0),                                // a3
	(a3),                               // b3
	(a3 | b3),                          // c3
	(a3 | b3 | c3),                     // d3
	(a3 | b3 | c3 | d3),                // e3
	(a3 | b3 | c3 | d3 | e3),           // f3
	(a3 | b3 | c3 | d3 | e3 | f3),      // g3
	(a3 | b3 | c3 | d3 | e3 | f3 | g3), // h3

	(0),                                // a2
	(a2),                               // b2
	(a2 | b2),                          // c2
	(a2 | b2 | c2),                     // d2
	(a2 | b2 | c2 | d2),                // e2
	(a2 | b2 | c2 | d2 | e2),           // f2
	(a2 | b2 | c2 | d2 | e2 | f2),      // g2
	(a2 | b2 | c2 | d2 | e2 | f2 | g2), // h2

	(0),                                // a1
	(a1),                               // b1
	(a1 | b1),                          // c1
	(a1 | b1 | c1),                     // d1
	(a1 | b1 | c1 | d1),                // e1
	(a1 | b1 | c1 | d1 | e1),           // f1
	(a1 | b1 | c1 | d1 | e1 | f1),      // g1
	(a1 | b1 | c1 | d1 | e1 | f1 | g1)  // h1
},

{
	// NORTHEAST

	(0), // a8
	(0), // b8
	(0), // c8
	(0), // d8
	(0), // e8
	(0), // f8
	(0), // g8
	(0), // h8

	(b8), // a7
	(c8), // b7
	(d8), // c7
	(e8), // d7
	(f8), // e7
	(g8), // f7
	(h8), // g7
	(0),  // h7

	(b7 | c8), // a6
	(c7 | d8), // b6
	(d7 | e8), // c6
	(e7 | f8), // d6
	(f7 | g8), // e6
	(g7 | h8), // f6
	(h7),      // g6
	(0),       // h6

	(b6 | c7 | d8), // a5
	(c6 | d7 | e8), // b5
	(d6 | e7 | f8), // c5
	(e6 | f7 | g8), // d5
	(f6 | g7 | h8), // e5
	(g6 | h7),      // f5
	(h6),           // g5
	(0),            // h5

	(b5 | c6 | d7 | e8), // a4
	(c5 | d6 | e7 | f8), // b4
	(d5 | e6 | f7 | g8), // c4
	(e5 | f6 | g7 | h8), // d4
	(f5 | g6 | h7),      // e4
	(g5 | h6),           // f4
	(h5),                // g4
	(0),                 // h4

	(b4 | c5 | d6 | e7 | f8), // a3
	(c4 | d5 | e6 | f7 | g8), // b3
	(d4 | e5 | f6 | g7 | h8), // c3
	(e4 | f5 | g6 | h7),      // d3
	(f4 | g5 | h6),           // e3
	(g4 | h5),                // f3
	(h4),                     // g3
	(0),                      // h3

	(b3 | c4 | d5 | e6 | f7 | g8), // a2
	(c3 | d4 | e5 | f6 | g7 | h8), // b2
	(d3 | e4 | f5 | g6 | h7),      // c2
	(e3 | f4 | g5 | h6),           // d2
	(f3 | g4 | h5),                // e2
	(g3 | h4),                     // f2
	(h3),                          // g2
	(0),                           // h2

	(b2 | c3 | d4 | e5 | f6 | g7 | h8), // a1
	(c2 | d3 | e4 | f5 | g6 | h7),      // b1
	(d2 | e3 | f4 | g5 | h6),           // c1
	(e2 | f3 | g4 | h5),                // d1
	(f2 | g3 | h4),                     // e1
	(g2 | h3),                          // f1
	(h2),                               // g1
	(0)                                 // h1
},

{

	// NORTHWEST

	(0), // a8
	(0), // b8
	(0), // c8
	(0), // d8
	(0), // e8
	(0), // f8
	(0), // g8
	(0), // h8

	(0),  // a7
	(a8), // b7
	(b8), // c7
	(c8), // d7
	(d8), // e7
	(e8), // f7
	(f8), // g7
	(g8), // h7

	(0),       // a6
	(a7),      // b6
	(b7 | a8), // c6
	(c7 | b8), // d6
	(d7 | c8), // e6
	(e7 | d8), // f6
	(f7 | e8), // g6
	(g7 | f8), // h6

	(0),            // a5
	(a6),           // b5
	(b6 | a7),      // c5
	(c6 | b7 | a8), // d5
	(d6 | c7 | b8), // e5
	(e6 | d7 | c8), // f5
	(f6 | e7 | d8), // g5
	(g6 | f7 | e8), // h5

	(0),                 // a4
	(a5),                // b4
	(b5 | a6),           // c4
	(c5 | b6 | a7),      // d4
	(d5 | c6 | b7 | a8), // e4
	(e5 | d6 | c7 | b8), // f4
	(f5 | e6 | d7 | c8), // g4
	(g5 | f6 | e7 | d8), // h4

	(0),                      // a3
	(a4),                     // b3
	(b4 | a5),                // c3
	(c4 | b5 | a6),           // d3
	(d4 | c5 | b6 | a7),      // e3
	(e4 | d5 | c6 | b7 | a8), // f3
	(f4 | e5 | d6 | c7 | b8), // g3
	(g4 | f5 | e6 | d7 | c8), // h3

	(0),                           // a2
	(a3),                          // b2
	(b3 | a4),                     // c2
	(c3 | b4 | a5),                // d2
	(d3 | c4 | b5 | a6),           // e2
	(e3 | d4 | c5 | b6 | a7),      // f2
	(f3 | e4 | d5 | c6 | b7 | a8), // g2
	(g3 | f4 | e5 | d6 | c7 | b8), // h2

	(0),                                // a1
	(a2),                               // b1
	(b2 | a3),                          // c1
	(c2 | b3 | a4),                     // d1
	(d2 | c3 | b4 | a5),                // e1
	(e2 | d3 | c4 | b5 | a6),           // f1
	(f2 | e3 | d4 | c5 | b6 | a7),      // g1
	(g2 | f3 | e4 | d5 | c6 | b7 | a8)  // h1

},

{

	// SOUTHEAST

	(b7 | c6 | d5 | e4 | f3 | g2 | h1), // a8
	(c7 | d6 | e5 | f4 | g3 | h2),      // b8
	(d7 | e6 | f5 | g4 | h3),           // c8
	(e7 | f6 | g5 | h4),                // d8
	(f7 | g6 | h5),                     // e8
	(g7 | h6),                          // f8
	(h7),                               // g8
	(0),                                // h8

	(b6 | c5 | d4 | e3 | f2 | g1), // a7
	(c6 | d5 | e4 | f3 | g2 | h1), // b7
	(d6 | e5 | f4 | g3 | h2),      // c7
	(e6 | f5 | g4 | h3),           // d7
	(f6 | g5 | h4),                // e7
	(g6 | h5),                     // f7
	(h6),                          // g7
	(0),                           // h7

	(b5 | c4 | d3 | e2 | f1), // a6
	(c5 | d4 | e3 | f2 | g1), // b6
	(d5 | e4 | f3 | g2 | h1), // c6
	(e5 | f4 | g3 | h2),      // d6
	(f5 | g4 | h3),           // e6
	(g5 | h4),                // f6
	(h5),                     // g6
	(0),                      // h6

	(b4 | c3 | d2 | e1), // a5
	(c4 | d3 | e2 | f1), // b5
	(d4 | e3 | f2 | g1), // c5
	(e4 | f3 | g2 | h1), // d5
	(f4 | g3 | h2),      // e5
	(g4 | h3),           // f5
	(h4),                // g5
	(0),                 // h5

	(b3 | c2 | d1), // a4
	(c3 | d2 | e1), // b4
	(d3 | e2 | f1), // c4
	(e3 | f2 | g1), // d4
	(f3 | g2 | h1), // e4
	(g3 | h2),      // f4
	(h3),           // g4
	(0),            // h4

	(b2 | c1), // a3
	(c2 | d1), // b3
	(d2 | e1), // c3
	(e2 | f1), // d3
	(f2 | g1), // e3
	(g2 | h1), // f3
	(h2),      // g3
	(0),       // h3

	(b1), // a2
	(c1), // b2
	(d1), // c2
	(e1), // d2
	(f1), // e2
	(g1), // f2
	(h1), // g2
	(0),  // h2

	(0), // a1
	(0), // b1
	(0), // c1
	(0), // d1
	(0), // e1
	(0), // f1
	(0), // g1
	(0)  // h1

},

{

	// SOUTHWEST

	(0),                                // a8
	(a7),                               // b8
	(b7 | a6),                          // c8
	(c7 | b6 | a5),                     // d8
	(d7 | c6 | b5 | a4),                // e8
	(e7 | d6 | c5 | b4 | a3),           // f8
	(f7 | e6 | d5 | c4 | b3 | a2),      // g8
	(g7 | f6 | e5 | d4 | c3 | b2 | a1), // h8

	(0),                           // a7
	(a6),                          // b7
	(b6 | a5),                     // c7
	(c6 | b5 | a4),                // d7
	(d6 | c5 | b4 | a3),           // e7
	(e6 | d5 | c4 | b3 | a2),      // f7
	(f6 | e5 | d4 | c3 | b2 | a1), // g7
	(g6 | f5 | e4 | d3 | c2 | b1), // h7

	(0),                      // a6
	(a5),                     // b6
	(b5 | a4),                // c6
	(c5 | b4 | a3),           // d6
	(d5 | c4 | b3 | a2),      // e6
	(e5 | d4 | c3 | b2 | a1), // f6
	(f5 | e4 | d3 | c2 | b1), // g6
	(g5 | f4 | e3 | d2 | c1), // h6

	(0),                 // a5
	(a4),                // b5
	(b4 | a3),           // c5
	(c4 | b3 | a2),      // d5
	(d4 | c3 | b2 | a1), // e5
	(e4 | d3 | c2 | b1), // f5
	(f4 | e3 | d2 | c1), // g5
	(g4 | f3 | e2 | d1), // h5

	(0),            // a4
	(a3),           // b4
	(b3 | a2),      // c4
	(c3 | b2 | a1), // d4
	(d3 | c2 | b1), // e4
	(e3 | d2 | c1), // f4
	(f3 | e2 | d1), // g4
	(g3 | f2 | e1), // h4

	(0),       // a3
	(a2),      // b3
	(b2 | a1), // c3
	(c2 | b1), // d3
	(d2 | c1), // e3
	(e2 | d1), // f3
	(f2 | e1), // g3
	(g2 | f1), // h3

	(0),  // a2
	(a1), // b2
	(b1), // c2
	(c1), // d2
	(d1), // e2
	(e1), // f2
	(f1), // g2
	(g1), // h2

	(0), // a1
	(0), // b1
	(0), // c1
	(0), // d1
	(0), // e1
	(0), // f1
	(0), // g1
	(0)  // h1

}

};

const char* pieceNames[] =
{
   "Pawn",
   "Knight",
   "Bishop",
   "Rook",
   "Queen",
   "King"
};

const char* colorNames[] =
{
   "Black",
   "White"
};
