#include "diag.h"
#include "bcm2835.h"

#include <pthread.h>

static pthread_mutex_t I2C_Mutex;


void i2cInit( void )
{

   pthread_mutex_init(&I2C_Mutex, NULL);


   DPRINT("Initializing I2C \n");

   bcm2835_i2c_begin();
   bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);
}


void i2cSendCommand( uint8_t slaveAddress, uint8_t *command, uint8_t len )
{
    pthread_mutex_lock(&I2C_Mutex);

    bcm2835_i2c_setSlaveAddress( slaveAddress );
    bcm2835_i2c_write((char *)command, len);

    pthread_mutex_unlock(&I2C_Mutex);
}

void i2cSendReceive( uint8_t slaveAddress, uint8_t *command, uint8_t cmdLen, uint8_t *rsp, uint8_t rspLen)
{
    pthread_mutex_lock(&I2C_Mutex);

    bcm2835_i2c_setSlaveAddress( slaveAddress );
    bcm2835_i2c_write_read_rs( (char *)command, cmdLen, (char *)rsp, rspLen);

    pthread_mutex_unlock(&I2C_Mutex);

}
