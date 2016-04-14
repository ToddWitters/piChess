#include "hsm.h"
#include "hsmDefs.h"
#include "event.h"
#include <string.h>

#include <semaphore.h>
#include <stddef.h>



int main ( void )
{

   HSM_Handle_t *sm = NULL;

   HSM_createHSM(myStateDef, myTransDef, ST_COUNT, transDefCount, &sm );
   
   HSM_init(sm);

   while(1)
   {

      event_t *evPtr;
      event_t eventData;

      // Wait for an event...
      sem_wait(getQueueSem(EVQ_EVENT_MANAGER));

      // Retrieve it
      if( (evPtr = getEvent(EVQ_EVENT_MANAGER)) == NULL) continue;

      // Move new event into local structure
      memcpy(&eventData, evPtr, sizeof(event_t));

      // process the event...
      HSM_processEvent(sm, eventData);

   }


   return 0;
}
