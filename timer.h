#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// NOTE:  The following should be a divisor of 100ms in order
//    for the chess clock to have the proper resolution...
#define MS_PER_TIC 50

struct timerEntry_s;

typedef struct timerEntry_s
{
   uint32_t            val;
   uint32_t            rel;
}timerEntry_t;

typedef enum timerRef_e
{
   TMR_UI_TIMEOUT,
   TMR_GAME_CLOCK_TIC,
   TMR_DIAG_TIMEOUT,

   TMR_TOTAL_TIMERS
}timerRef_t;

typedef enum timerErr_e
{
   TMR_ERR_NONE,
   TMR_ERR_ALREADY_INIT,
   TMR_ERR_INVALID_ID,
}timerErr_t;


timerErr_t timerInit( void );
timerErr_t timerStart( timerRef_t id, uint32_t val, uint32_t rel);
timerErr_t timerKill( timerRef_t id );
timerErr_t timerGetVal( timerRef_t id, uint32_t *val);

#endif
