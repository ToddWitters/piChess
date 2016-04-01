#include "hsm.c"
#include "hsmDefs.c"

int main ( void )
{
   HSM_Handle_t *sm;
   event_t event;

   printf("\n Calling create \n");
   createHSM(myStateDef, myTransDef, ST_COUNT, sizeof(myTransDef)/sizeof(myTransDef[0]), &sm );

   printf("\n Calling int \n");
   initHSM(sm);

   event.ev = EV_BUTTON_STATE;

   printf("\n Calling processEvent \n");
   processEvent(sm, event);

   return 0;
}
