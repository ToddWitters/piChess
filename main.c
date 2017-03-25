#include "hsm.h"
#include "hsmDefs.h"
#include "event.h"
#include "diag.h"

#include <string.h>

#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

extern char *eventName[];

uint16_t smInit( event_t ev )
{
   return ST_SPLASH_SCREEN;
}

void smExit( event_t ev )
{

}

int main ( void )
{
   HSM_Error_t err;
   HSM_Handle_t sm;


   //DPRINT("Creating state machine...\n");
   if ( (err = HSM_createHSM(myStateDef, myTransDef, ST_COUNT, transDefCount, smInit, smExit,  &sm ) ) != HSM_NO_ERROR)
   {
      printf("HSM_createHSM() failed with return value of %d\n", err);
      exit(-1);
   }


   if ( (err = HSM_init(&sm) ) != HSM_NO_ERROR)
   {
      printf("HSM_init() failed with return value of %d\n", err);
      exit(-1);
   }


   while(1)
   {

      event_t *evPtr;
      event_t eventData;

      // Wait for an event...
      sem_wait(getQueueSem(EVQ_EVENT_MANAGER));

      // Retrieve it.  Ignore (continue) on error
      if( (evPtr = getEvent(EVQ_EVENT_MANAGER)) == NULL) continue;

      // Move new event into local structure
      memcpy(&eventData, evPtr, sizeof(event_t));

      // process the event...
      err = HSM_processEvent(&sm, eventData);

      // report any errors found...
//      if(err == HSM_EV_NOT_IN_TABLE)
//         DPRINT("Warning: HSM_ProcessEvent() could not find event %d in transition table\n", eventData.ev);

//      else if(err == HSM_NO_EV_HANDLER_FOUND)
//         DPRINT("Warning: HSM_ProcessEvent() could not find transition for event %d in state %d\n", eventData.ev, sm.currentState);

      if(err != HSM_NO_ERROR && err != HSM_NO_EV_HANDLER_FOUND)
         DPRINT("Error: HSM_ProcessEvent() returned error %d while processing event %d in state %d\n", err, eventData.ev, sm.currentState);

   }

   return 0;
}
