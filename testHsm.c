#include "hsm.c"
#include "hsmDefs.c"

int main ( void )
{
   HSM_Handle_t *sm;

#if 0
   bcm2835_init();

   DPRINT("Program start\n");

   // set all options
   loadOptions(&options);

   // Set up the timer tic...
   timerInit();

   // Set up hardware interfaces
   i2cInit();
   gpioInit();
   displayInit();
   LED_Init();
   switchInit();

   // Start polling switches
   StartSwitchPoll();

   // Init the event handler
   initEvent();
#endif

   HSM_createHSM(myStateDef, myTransDef, ST_COUNT, sizeof(myTransDef)/sizeof(myTransDef[0]), &sm );
   HSM_init(sm);

#if 0
   while(1)
   {

      eventData_t *evPtr;
      eventData_t eventData;

      // Wait for an event...
      sem_wait(getQueueSem(EVQ_EVENT_MANAGER));

      // Retrieve it
      evPtr = getEvent(EVQ_EVENT_MANAGER);

      // skip processing on error...
      if(evPtr == NULL) continue;

      // Move new event into local structure
      memcpy(&eventData, evPtr, sizeof(eventData_t));

      // process the event...
      HSM_processEvent(sm, eventData);

   }

   return 0;

#endif
}
