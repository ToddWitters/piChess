#ifndef EVENT_H
#define EVENT_H

#include <semaphore.h>

#include "types.h"

#define EVENT_QUEUE_SIZE 50

typedef enum eQueIndex_e
{
    EVQ_EVENT_MANAGER,

    EVQ_TOTAL
}evQueueIndex_t;

typedef struct eventData_t
{
   event_t ev;     // The event
   int     param;  // param depends upon event...
}eventData_t;

void     initEvent( void );
void     putEvent(evQueueIndex_t indx, event_t *evData);
event_t *getEvent( evQueueIndex_t indx );
sem_t   *getQueueSem(evQueueIndex_t indx);

#endif
