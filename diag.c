#include <stdio.h>
#include <stdarg.h>

#include "types.h"
#include "diag.h"
#include "time.h"
#include "bcm2835.h"

void DIAG_print(char *msg, ...)
{
   va_list argp;
   static bool_t   timeInit  = FALSE;
   static uint64_t startTime = 0;

   uint64_t t;

   if(timeInit == FALSE)
   {
      startTime = bcm2835_st_read();
      t = 0;
      timeInit = TRUE;
   }
   else
   {
      t = bcm2835_st_read() - startTime;
   }

   printf("[%10.6f] ", t/1000000.0);
   va_start(argp, msg);

   vprintf(msg, argp);

   va_end(argp);
}
