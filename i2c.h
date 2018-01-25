void i2cInit( void );
uint8_t i2cSendCommand( uint8_t slaveAddress, uint8_t *command, uint8_t len );
uint8_t i2cSendReceive( uint8_t slaveAddress, uint8_t *command, uint8_t cmdLen, uint8_t *rsp, uint8_t rspLen);
