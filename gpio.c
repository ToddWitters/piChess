#include "types.h"
#include "gpio.h"
#include "i2c.h"
#include "diag.h"
#include "bcm2835.h"

void gpioInit( void )
{
   uint8_t command[3];

   DPRINT("Initializing GPIO Expanders\n");


   bcm2835_gpio_set_pud(ROW_8_SWITCH_INT_PIN, BCM2835_GPIO_PUD_UP );
   bcm2835_gpio_set_pud(ROW_7_SWITCH_INT_PIN, BCM2835_GPIO_PUD_UP );
   bcm2835_gpio_set_pud(ROW_6_SWITCH_INT_PIN, BCM2835_GPIO_PUD_UP );
   bcm2835_gpio_set_pud(ROW_5_SWITCH_INT_PIN, BCM2835_GPIO_PUD_UP );
   bcm2835_gpio_set_pud(ROW_4_SWITCH_INT_PIN, BCM2835_GPIO_PUD_UP );
   bcm2835_gpio_set_pud(ROW_3_SWITCH_INT_PIN, BCM2835_GPIO_PUD_UP );
   bcm2835_gpio_set_pud(ROW_2_SWITCH_INT_PIN, BCM2835_GPIO_PUD_UP );
   bcm2835_gpio_set_pud(ROW_1_SWITCH_INT_PIN, BCM2835_GPIO_PUD_UP );
   bcm2835_gpio_set_pud(BUTTON_SWITCH_INT_PIN, BCM2835_GPIO_PUD_UP );

// REED SWITCHES

   // Define a pull-up for all inputs pins
   command[0] = GPPUA_ADDR;
   command[1] = 0xFF; // data for GPPUA
   command[2] = 0xFF; // data for GPPUB
   i2cSendCommand(GPIO_EXPANDER_87_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_65_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_43_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_21_ADDR, command, 3);

   // Set default state for pin values interrupt comparisons
   command[0] = DEFVALA_ADDR;
   i2cSendCommand(GPIO_EXPANDER_87_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_65_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_43_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_21_ADDR, command, 3);

   // Set interrupt to trigger on change from default value
   command[0] = INTCONA_ADDR;
   i2cSendCommand(GPIO_EXPANDER_87_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_65_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_43_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_21_ADDR, command, 3);

   // Enable interrupt output
   command[0] = GPINTENA_ADDR;
   i2cSendCommand(GPIO_EXPANDER_87_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_65_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_43_ADDR, command, 3);
   i2cSendCommand(GPIO_EXPANDER_21_ADDR, command, 3);

// U/I Board (Display and 5-way switch)

   // Set up GPIO Expander pin directions
   command[0] = IODIRA_ADDR;
   command[1] = 0x00; // data for IODIRA ( dispaly data lines are all outputs )
   command[2] = 0x1F; // data for IODIRB ( upper 3 pins are dispaly control outputs, lower 5 pins are button inputs)
   i2cSendCommand(GPIO_EXPANDER_UI_ADDR, command, 3);

   // Define a pull-up for all inputs pins
   command[0] = GPPUB_ADDR;
   command[1] = 0x1F;
   i2cSendCommand(GPIO_EXPANDER_UI_ADDR, command, 2);

   // Set default state for pin values interrupt comparisons
   command[0] = DEFVALB_ADDR;
   i2cSendCommand(GPIO_EXPANDER_UI_ADDR, command, 2);

   // Set interrupt to trigger on change from default value
   command[0] = INTCONB_ADDR;
   i2cSendCommand(GPIO_EXPANDER_UI_ADDR, command, 2);

   // Enable interrupt output
   command[0] = GPINTENB_ADDR;
   i2cSendCommand(GPIO_EXPANDER_UI_ADDR, command, 2);

}
