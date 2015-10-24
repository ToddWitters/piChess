/*
 *  timer usage examples borrowed from http://courses.cs.vt.edu/~cs5565/spring2006-final/projects/project1A/posix-timers.c
 */

#include "timer.h"
#include "event.h"
#include "diag.h"
#include "switch.h"

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

static timerEntry_t    timerData[TMR_TOTAL_TIMERS];
static pthread_t       timerThread;
static pthread_mutex_t timerDataMutex;
static sem_t           timerSem;

static void timerTic( void );
static void *timerTask ( void *arg );
static void timersignalhandler();

timerErr_t timerInit( void )
{
   static bool_t init = FALSE;
   int i;

   // structure that will be passed to setitimer
   struct itimerval tval;

   tval.it_interval.tv_sec  = 0;
   tval.it_interval.tv_usec = MS_PER_TIC * 1000;

   tval.it_value.tv_sec  = 0;
   tval.it_value.tv_usec = MS_PER_TIC * 1000;

   DPRINT("Initializing timers\n");

   if(init)
   {
      DPRINT("timerInit called after timers already initialized\n");
      return TMR_ERR_ALREADY_INIT;
   }

   for(i=0; i< TMR_TOTAL_TIMERS; i++)
   {
      timerData[i].val = 0;
      timerData[i].rel = 0;
   }

   // Create main thread for this module
   pthread_create(&timerThread, NULL, timerTask , NULL);

   // Create a mutex to block data access from multiple threads.
   pthread_mutex_init(&timerDataMutex, NULL);

   // Initialize the semaphore that will signal a timer tic
   sem_init(&timerSem, 0, 0);

   // Map the signal the timer will generate with a handler
   signal(SIGALRM, timersignalhandler);

   // Start the timer...
   setitimer(ITIMER_REAL, &tval, (struct itimerval*)0);

   // never run init again...
   init = TRUE;

   return TMR_ERR_NONE;
}

static void timerTic( void )
{
   int i;

   pthread_mutex_lock(&timerDataMutex);

   for(i=0; i<TMR_TOTAL_TIMERS; i++)
   {
      if(timerData[i].val)
      {
         if(--timerData[i].val == 0)
         {
            eventData_t eventData;

            eventData.ev     = EV_TIMER;
            eventData.param  = i;

            putEvent(EVQ_EVENT_MANAGER, &eventData);

            if(timerData[i].rel != 0)
            {
               timerData[i].val = timerData[i].rel;
            }
         }
      }
   }
   pthread_mutex_unlock(&timerDataMutex);
}


static void *timerTask ( void *arg )
{
   while(1)
   {
      sem_wait(&timerSem);
      if(errno != EINTR)
      {
         switchPoll();
         timerTic();
      }
   }

   return NULL;
}

// Converts SIGALRM into a semaphore posting...
static void timersignalhandler()
{
	sem_post(&timerSem);	// the only async-signal-safe function pthreads defines
}

// Pass in time (in ms)
timerErr_t timerStart( timerRef_t id, uint32_t val, uint32_t rel)
{

   if(id > TMR_TOTAL_TIMERS)
   {
      DPRINT("Invalid timer id %d passed to timerStart\n", (int)id);
      return TMR_ERR_INVALID_ID;
   }

   pthread_mutex_lock(&timerDataMutex);

   // Ensure anything less than the tic interval gets at least 1 tic
   timerData[id].val = ((val / MS_PER_TIC) + 1);
   timerData[id].rel = ((rel / MS_PER_TIC));

   pthread_mutex_unlock(&timerDataMutex);

   return TMR_ERR_NONE;
}

timerErr_t timerKill( timerRef_t id )
{
   if(id > TMR_TOTAL_TIMERS)
   {
      DPRINT("Invalid timer id %d passed to timerKill\n", (int)id);
      return TMR_ERR_INVALID_ID;
   }

   pthread_mutex_lock(&timerDataMutex);

   timerData[id].val = 0;

   pthread_mutex_unlock(&timerDataMutex);

   return TMR_ERR_NONE;
}

timerErr_t timerGetVal( timerRef_t id, uint32_t *val)
{

   if(id > TMR_TOTAL_TIMERS)
   {
      DPRINT("Invalid timer id %d passed to timerGetVal\n", (int)id);
      return TMR_ERR_INVALID_ID;
   }

   pthread_mutex_lock(&timerDataMutex);

   *val = MS_PER_TIC * (timerData[id].val);

   pthread_mutex_unlock(&timerDataMutex);

   return TMR_ERR_NONE;
}
