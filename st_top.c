#include "hsm.h"
#include "hsmDefs.h"
#include "st_top.h"

#include "bcm2835.h"
#include "diag.h"
#include "options.h"
#include "timer.h"
#include "i2c.h"
#include "gpio.h"
#include "display.h"
#include "led.h"
#include "switch.h"
#include "event.h"
#include "st_inGame.h"


void topEntry( event_t ev )
{
   static bool_t initDone = FALSE;

   if(!initDone)
   {
      // Init the hardware
      bcm2835_init();

      DPRINT("Program start\n");

      // set all options
      loadOptions(&options);

      // Set up the timer tic...
      timerInit();

      // Peripherals
      i2cInit();
      gpioInit();
      displayInit();
      LED_Init();
      switchInit();

      // Start polling switches
      StartSwitchPoll();

      // Init the event handler
      initEvent();

      initDone = TRUE;
   }

}

uint16_t topPickSubstate(event_t ev)
{
   (void)ev;

   return ST_SPLASH_SCREEN;
}
