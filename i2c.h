void i2cInit( void );
void i2cSendCommand( uint8_t slaveAddress, uint8_t *command, uint8_t len );
void i2cSendReceive( uint8_t slaveAddress, uint8_t *command, uint8_t cmdLen, uint8_t *rsp, uint8_t rspLen);
