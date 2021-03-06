// #include "event.h"
#include "hsm.h"
#include "diag.h"
#include "string.h"
#include "event.h"

#include <pthread.h>
#include <semaphore.h>


extern char *eventName[];

typedef struct eventQueue_s
{
    int             evPushIndex;
    int             evPopIndex;
    event_t         evQueue[EVENT_QUEUE_SIZE];
    sem_t           sem;
    pthread_mutex_t mutex;

}eventQueue_t;

eventQueue_t eventQueue[EVQ_TOTAL];

void initEvent( void )
{
   int i;

   DPRINT("Initializing event handler\n");

   for(i=0;i<EVQ_TOTAL;i++)
   {
      eventQueue[i].evPushIndex = eventQueue[i].evPopIndex = 0;
      pthread_mutex_init(&eventQueue[i].mutex, NULL);
      sem_init(&eventQueue[i].sem, 0, 0);
   }
}


sem_t *getQueueSem(evQueueIndex_t indx)
{
   return &eventQueue[indx].sem;
}

void putEvent(evQueueIndex_t indx, event_t *evData)
{

   if(indx >= EVQ_TOTAL)
   {
       DPRINT("Invalid queue index %d passed to putEvent\n", (int)indx);
       return;
   }

   pthread_mutex_lock(&eventQueue[indx].mutex);

   if( ( (eventQueue[indx].evPushIndex + 1) == eventQueue[indx].evPopIndex ) ||
       (eventQueue[indx].evPushIndex == EVENT_QUEUE_SIZE -1  && eventQueue[indx].evPopIndex == 0) )
   {
      DPRINT("Event Queue Overflow.  Discarding Event %d %d\n", evData->ev, evData->data);
      pthread_mutex_unlock(&eventQueue[indx].mutex);
      return;
   }
   memcpy(&eventQueue[indx].evQueue[eventQueue[indx].evPushIndex], evData, sizeof(event_t));

   if(++eventQueue[indx].evPushIndex >= EVENT_QUEUE_SIZE) eventQueue[indx].evPushIndex = 0;

   pthread_mutex_unlock(&eventQueue[indx].mutex);

   sem_post(&eventQueue[indx].sem);

}

event_t *getEvent( evQueueIndex_t indx )
{
   static event_t retValue;

   if(indx >= EVQ_TOTAL)
   {
      DPRINT("Invalid queue index %d passed to getEvent\n", (int)indx);
      return NULL;
   }

   pthread_mutex_lock(&eventQueue[indx].mutex);

   if(eventQueue[indx].evPushIndex == eventQueue[indx].evPopIndex)
   {
      pthread_mutex_unlock(&eventQueue[indx].mutex);
      return NULL;
   }

   memcpy(&retValue, &eventQueue[indx].evQueue[eventQueue[indx].evPopIndex], sizeof(event_t));

   if(++eventQueue[indx].evPopIndex >= EVENT_QUEUE_SIZE) eventQueue[indx].evPopIndex = 0;

   pthread_mutex_unlock(&eventQueue[indx].mutex);

   return &retValue;
}

