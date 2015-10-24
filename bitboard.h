#include "types.h"

int bitCount (BB x);
int getLSBindex(U64 bb);
int getMSBindex(U64 bb);

BB Nfill(BB gen);
BB Sfill(BB gen);

BB Nattacks(BB osliders, BB empty);
BB Sattacks(BB osliders, BB empty);
BB Eattacks(BB osliders, BB empty);
BB Wattacks(BB osliders, BB empty);
BB NEattacks(BB osliders, BB empty);
BB NWattacks(BB osliders, BB empty);
BB SEattacks(BB osliders, BB empty);
BB SWattacks(BB osliders, BB empty);



// Usefull for shifting operations...
#define notAfile 0x7F7F7F7F7F7F7F7F
#define notHfile 0xFEFEFEFEFEFEFEFE

#define shiftS(X) ((X) >> 8)
#define shiftN(X) ((X) << 8)
#define shiftE(X) (((X) >> 1) & notAfile)
#define shiftW(X) (((X) << 1) & notHfile)

#define shiftNE(X) (((X) << 7) & notAfile)
#define shiftNW(X) (((X) << 9) & notHfile)
#define shiftSE(X) (((X) >> 9) & notAfile)
#define shiftSW(X) (((X) >> 7) & notHfile)

#define lsb(X) ((X) & -(X))
#define clearlsb(X) (X = (X & (X-1)))
