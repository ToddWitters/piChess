extern U64 zobristTable[781];

#define SVALUE(S) (8 * (7 - (S)/8)) + (S % 8)

#define Z_PIECESQUARE_KEY(P,C,S)   (zobristTable[((P)*128) + ((C)*64) + (SVALUE(S))])
#define Z_WHITE_SHORT_KEY          (zobristTable[768])
#define Z_WHITE_LONG_KEY           (zobristTable[769])
#define Z_BLACK_SHORT_KEY          (zobristTable[770])
#define Z_BLACK_LONG_KEY           (zobristTable[771])
#define Z_ENPASSANT_COL_KEY(C)     (zobristTable[772+(C)])
#define Z_WHITE_TURN_KEY           (zobristTable[780])

