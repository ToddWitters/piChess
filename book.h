#include "types.h"
#include "stdio.h"


#define MAX_CANDIDATES 256
#define RECORD_SIZE     16
#define KEY_OFFSET       0
#define MOVE_OFFSET      8
#define WEIGHT_OFFSET   10
#define LEARN_OFFSET    12

typedef struct
{
    move_t mv;
    U16    weight;
    U64    hash;
}candidate_t;

typedef enum
{
  BOOK_NO_ERROR,
  BOOK_NOT_OPEN,
  BOOK_FILE_NOT_FOUND,
  BOOK_REPLACED,
  BOOK_ALREADY_CLOSED,
  BOOK_MOVE_ALREADY_EXISTS,
  BOOK_POSITION_NOT_FOUND
}bookErr_t;

bool_t    isBookOpen( void );

bookErr_t openBook  ( char *filename );
bookErr_t closeBook ( void );

bookErr_t listBookMoves( board_t *b);
bookErr_t getBestMove  ( board_t *b, move_t *mv );
bookErr_t getRandMove  ( board_t *b, move_t *mv );

bookErr_t addBookMove ( U64 key, move_t mv, U16 weight, U32 learn);
bookErr_t delMove     ( U64 key, move_t mv);
bookErr_t setWeight   ( U64 key, move_t mv, U16 weight);
bookErr_t setLearn    ( U64 key, move_t mv, U32 learn);

    
